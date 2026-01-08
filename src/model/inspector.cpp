#include "inspector.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

#include <google/protobuf/repeated_field.h>

#include <onnx/onnx_pb.h>
#include <onnxruntime_cxx_api.h>

namespace {

eModelTensorDataType onnx_to_model_tensor_type(int onnx_type) {
  switch (onnx_type) {
  case onnx::TensorProto_DataType_UINT8:
    return MODEL_TENSOR_DATA_TYPE_UINT8;
  case onnx::TensorProto_DataType_INT8:
    return MODEL_TENSOR_DATA_TYPE_INT8;
  case onnx::TensorProto_DataType_UINT16:
    return MODEL_TENSOR_DATA_TYPE_UINT16;
  case onnx::TensorProto_DataType_INT16:
    return MODEL_TENSOR_DATA_TYPE_INT16;
  case onnx::TensorProto_DataType_UINT32:
    return MODEL_TENSOR_DATA_TYPE_UINT32;
  case onnx::TensorProto_DataType_INT32:
    return MODEL_TENSOR_DATA_TYPE_INT32;
  case onnx::TensorProto_DataType_UINT64:
    return MODEL_TENSOR_DATA_TYPE_UINT64;
  case onnx::TensorProto_DataType_INT64:
    return MODEL_TENSOR_DATA_TYPE_INT64;
  case onnx::TensorProto_DataType_FLOAT16:
    return MODEL_TENSOR_DATA_TYPE_FLOAT16;
  case onnx::TensorProto_DataType_FLOAT:
    return MODEL_TENSOR_DATA_TYPE_FLOAT32;
  case onnx::TensorProto_DataType_DOUBLE:
    return MODEL_TENSOR_DATA_TYPE_DOUBLE;
  case onnx::TensorProto_DataType_BFLOAT16:
    return MODEL_TENSOR_DATA_TYPE_BFLOAT16;
  default:
    return MODEL_TENSOR_DATA_TYPE_UNDEFINED;
  }
}

std::vector<int64_t> shape_from_value_info(const onnx::ValueInfoProto &value) {
  std::vector<int64_t> shape;
  if (!value.has_type()) {
    return shape;
  }
  const auto &tensor_type = value.type().tensor_type();
  if (!tensor_type.has_shape()) {
    return shape;
  }
  for (const auto &dim : tensor_type.shape().dim()) {
    if (dim.has_dim_value()) {
      shape.push_back(dim.dim_value());
    } else {
      shape.push_back(-1);
    }
  }
  return shape;
}

std::vector<int64_t>
shape_from_initializer(const onnx::TensorProto &initializer) {
  return {initializer.dims().begin(), initializer.dims().end()};
}

template <typename Field>
std::string join_repeated_field(const Field &field, char delimiter) {
  std::string serialized;
  for (const auto &value : field) {
    if (!serialized.empty()) {
      serialized.push_back(delimiter);
    }
    serialized += std::to_string(value);
  }
  return serialized;
}

std::string
join_string_field(const google::protobuf::RepeatedPtrField<std::string> &field,
                  char delimiter) {
  std::string serialized;
  for (const auto &value : field) {
    if (!serialized.empty()) {
      serialized.push_back(delimiter);
    }
    serialized += value;
  }
  return serialized;
}

std::string attribute_to_string(const onnx::AttributeProto &attr) {
  switch (attr.type()) {
  case onnx::AttributeProto_AttributeType_FLOAT:
    return std::to_string(attr.f());
  case onnx::AttributeProto_AttributeType_INT:
    return std::to_string(attr.i());
  case onnx::AttributeProto_AttributeType_STRING:
    return attr.s();
  case onnx::AttributeProto_AttributeType_FLOATS:
    return join_repeated_field(attr.floats(), ',');
  case onnx::AttributeProto_AttributeType_INTS:
    return join_repeated_field(attr.ints(), ',');
  case onnx::AttributeProto_AttributeType_STRINGS:
    return join_string_field(attr.strings(), ',');
  case onnx::AttributeProto_AttributeType_TENSOR:
    return "[tensor]";
  case onnx::AttributeProto_AttributeType_TENSORS:
    return "[tensor-list]";
  default:
    return {};
  }
}

} // namespace

ModelInspector::ModelInspector(const std::string &model_path)
    : _model_path(model_path) {
  load_model(_model_path);
}

bool ModelInspector::load_model(const std::string &model_path) {
  _nodes.clear();
  _tensor_descs.clear();
  _graph.root_node = nullptr;

  Ort::Env env{ORT_LOGGING_LEVEL_INFO, "model-inspector"};
  Ort::SessionOptions session_options;
  Ort::Session session = Ort::Session(env, model_path.data(), session_options);
  Ort::AllocatorWithDefaultOptions allocator;

  std::cout << "Input Name/Shape" << std::endl;
  std::vector<std::string> input_names;
  for (std::size_t i = 0; i < session.GetInputCount(); ++i) {
    input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
    std::cout << input_names.at(i) << std::endl;
  }

  std::cout << "Output Name/Shape" << std::endl;
  std::vector<std::string> output_names;
  for (std::size_t i = 0; i < session.GetOutputCount(); ++i) {
    output_names.emplace_back(
        session.GetOutputNameAllocated(i, allocator).get());
    std::cout << output_names.at(i) << std::endl;
  }

  onnx::ModelProto model_proto;
  {
    std::fstream input(model_path, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
      std::cerr << "unable to open model file: " << model_path << std::endl;
      return false;
    }
    if (!model_proto.ParseFromIstream(&input)) {
      std::cerr << "failed to parse ONNX file: " << model_path << std::endl;
      return false;
    }
  }

  const auto &graph = model_proto.graph();
  std::unordered_map<std::string, sModelTensorDesc *> tensor_map;

  auto register_tensor_desc =
      [&](const std::string &_name, std::vector<int64_t> shape,
          eModelTensorDataType dtype) -> sModelTensorDesc * {
    auto it = tensor_map.find(_name);
    if (it != tensor_map.end()) {
      return it->second;
    }
    auto desc = std::make_unique<sModelTensorDesc>();
    desc->name = _name;
    desc->shape = std::move(shape);
    desc->tensorDataType = dtype;
    sModelTensorDesc *ptr = desc.get();
    _tensor_descs.push_back(std::move(desc));
    tensor_map.emplace(_name, ptr);
    return ptr;
  };

  auto ensure_tensor_desc = [&](const std::string &_name) {
    auto it = tensor_map.find(_name);
    if (it != tensor_map.end()) {
      return it->second;
    }
    return register_tensor_desc(_name, {}, MODEL_TENSOR_DATA_TYPE_UNDEFINED);
  };

  auto register_value_info = [&](const onnx::ValueInfoProto &value_info) {
    if (value_info.name().empty()) {
      return;
    }
    auto shape = shape_from_value_info(value_info);
    auto dtype = MODEL_TENSOR_DATA_TYPE_UNDEFINED;
    if (value_info.has_type() && value_info.type().has_tensor_type() &&
        value_info.type().tensor_type().has_elem_type()) {
      dtype = onnx_to_model_tensor_type(
          value_info.type().tensor_type().elem_type());
    }
    register_tensor_desc(value_info.name(), std::move(shape), dtype);
  };

  auto register_initializer = [&](const onnx::TensorProto &initializer) {
    if (initializer.name().empty()) {
      return;
    }
    register_tensor_desc(initializer.name(),
                         shape_from_initializer(initializer),
                         onnx_to_model_tensor_type(initializer.data_type()));
  };

  // gather tensor metadata from graph inputs/outputs/infos
  for (const auto &input : graph.input()) {
    register_value_info(input);
  }
  for (const auto &value_info : graph.value_info()) {
    register_value_info(value_info);
  }
  for (const auto &output : graph.output()) {
    register_value_info(output);
  }
  for (const auto &initializer : graph.initializer()) {
    register_initializer(initializer);
  }

  auto add_node = [&](std::string name, std::string op_type) {
    auto node = std::make_unique<sModelGraphNode>();
    node->name = std::move(name);
    node->op_type = std::move(op_type);
    node->param_count = 0;
    node->flops_estimate = 0.0;
    auto *ptr = node.get();
    _nodes.push_back(std::move(node));
    return ptr;
  };

  std::unordered_map<std::string, sModelGraphNode *> producer_map;

  auto create_placeholder =
      [&](const std::string &name,
          const std::string &op_type) -> sModelGraphNode * {
    if (name.empty()) {
      return nullptr;
    }
    return add_node(name, op_type);
  };

  for (const auto &input : graph.input()) {
    auto *placeholder =
        create_placeholder("__input_node__" + input.name(), "GraphInput");
    if (placeholder) {
      producer_map[input.name()] = placeholder;
    }
  }
  for (const auto &initializer : graph.initializer()) {
    auto *placeholder = create_placeholder(
        "__initializer_node__" + initializer.name(), "Initializer");
    if (placeholder) {
      producer_map[initializer.name()] = placeholder;
    }
  }

  int node_index = 0;
  for (const auto &node_proto : graph.node()) {
    std::string node_name = node_proto.name();
    if (node_name.empty()) {
      node_name = node_proto.op_type() + std::to_string(node_index);
    }
    auto *node = add_node(node_name, node_proto.op_type());
    if (!_graph.root_node) {
      _graph.root_node = node;
    }
    for (const auto &attribute : node_proto.attribute()) {
      const auto value = attribute_to_string(attribute);
      if (!value.empty()) {
        node->attributes.emplace(attribute.name(), value);
      }
    }

    for (const auto &input : node_proto.input()) {
      sModelGraphNode *source{};
      auto it = producer_map.find(input);
      if (it != producer_map.end()) {
        source = it->second;
      } else {
        source = add_node("__unknown_source_" + input, "Synthetic");
        producer_map.emplace(input, source);
      }
      auto *tensor_desc = ensure_tensor_desc(input);
      sModelGraphEdge input_edge{input, *tensor_desc, *source, *node};
      node->inputs.push_back(input_edge);
      source->outputs.push_back(input_edge);
    }

    for (const auto &output : node_proto.output()) {
      producer_map[output] = node;
      ensure_tensor_desc(output);
    }
    ++node_index;
  }

  std::cout << "Number of nodes: " << graph.node_size() << std::endl;
  return true;
}
