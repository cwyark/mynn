#include "inspector.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <google/protobuf/repeated_field.h>

#include <onnx/onnx_pb.h>
#include <onnxruntime_cxx_api.h>

namespace {

eModelTensorDataType onnx_to_model_dtype(int onnx_type) {
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
  case onnx::TensorProto_DataType_BOOL:
    return MODEL_TENSOR_DATA_TYPE_BOOL;
  default:
    return MODEL_TENSOR_DATA_TYPE_UNDEFINED;
  }
}

std::vector<int64_t>
shape_from_tensor_type(const onnx::TypeProto::Tensor &tensor_type) {
  std::vector<int64_t> shape;
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

eModelTensorDataType
dtype_from_tensor_type(const onnx::TypeProto::Tensor &tensor_type) {
  if (!tensor_type.has_elem_type()) {
    return MODEL_TENSOR_DATA_TYPE_UNDEFINED;
  }
  return onnx_to_model_dtype(tensor_type.elem_type());
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

std::string ModelInspector::getName() { return _model_path; }

bool ModelInspector::load_model(const std::string &model_path) {
  _model_path = model_path;
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

  const auto &graph_proto = model_proto.graph();
  _graph = sModelGraph{};

  std::unordered_map<std::string, int> tensor_index_by_name;
  std::unordered_map<std::string, int> producer_map;

  auto ensure_tensor =
      [&](const std::string &name, const std::vector<int64_t> &shape,
          eModelTensorDataType dtype, bool is_initializer) -> int {
    if (name.empty()) {
      return -1;
    }
    auto it = tensor_index_by_name.find(name);
    if (it != tensor_index_by_name.end()) {
      auto &tensor = _graph.tensors[it->second];
      if (!shape.empty()) {
        tensor.shape = shape;
      }
      if (dtype != MODEL_TENSOR_DATA_TYPE_UNDEFINED) {
        tensor.tensorDataType = dtype;
      }
      tensor.is_initializer = tensor.is_initializer || is_initializer;
      return it->second;
    }

    sModelTensor tensor;
    tensor.name = name;
    tensor.shape = shape;
    tensor.tensorDataType = dtype;
    tensor.is_initializer = is_initializer;
    _graph.tensors.push_back(tensor);
    int index = static_cast<int>(_graph.tensors.size()) - 1;
    tensor_index_by_name[name] = index;
    return index;
  };

  auto add_edge = [&](const std::string &name, int tensor_index,
                      int source_node, int target_node) -> int {
    sModelGraphEdge edge;
    edge.name = name;
    edge.tensor_index = tensor_index;
    edge.source_node = source_node;
    edge.target_node = target_node;
    _graph.edges.push_back(edge);
    return static_cast<int>(_graph.edges.size()) - 1;
  };

  auto populate_value_info = [&](const onnx::ValueInfoProto &value_info) {
    if (value_info.name().empty()) {
      return;
    }
    auto value_type = value_info.type();
    if (!value_type.has_tensor_type()) {
      return;
    }
    const auto &tensor_type = value_type.tensor_type();
    ensure_tensor(value_info.name(), shape_from_tensor_type(tensor_type),
                  dtype_from_tensor_type(tensor_type), false);
  };

  for (const auto &initializer : graph_proto.initializer()) {
    if (initializer.name().empty()) {
      continue;
    }
    ensure_tensor(initializer.name(), shape_from_initializer(initializer),
                  onnx_to_model_dtype(initializer.data_type()), true);
  }

  for (const auto &input : graph_proto.input()) {
    auto value_type = input.type();
    std::vector<int64_t> shape;
    eModelTensorDataType dtype = MODEL_TENSOR_DATA_TYPE_UNDEFINED;
    if (value_type.has_tensor_type()) {
      const auto &tensor_type = value_type.tensor_type();
      shape = shape_from_tensor_type(tensor_type);
      dtype = dtype_from_tensor_type(tensor_type);
    }
    const int tensor_index =
        ensure_tensor(input.name(), std::move(shape), dtype, false);
    if (tensor_index >= 0) {
      _graph.input_tensors.push_back(tensor_index);
    }
    producer_map[input.name()] = -1;
  }

  for (const auto &value_info : graph_proto.value_info()) {
    populate_value_info(value_info);
  }

  for (const auto &output : graph_proto.output()) {
    populate_value_info(output);
    if (auto it = tensor_index_by_name.find(output.name());
        it != tensor_index_by_name.end()) {
      _graph.output_tensors.push_back(it->second);
    }
  }

  int node_counter = 0;
  for (const auto &node_proto : graph_proto.node()) {
    std::string node_name = node_proto.name();
    if (node_name.empty()) {
      node_name = node_proto.op_type() + "_" + std::to_string(node_counter);
    }
    _graph.nodes.emplace_back();
    auto &current_node = _graph.nodes.back();
    current_node.name = std::move(node_name);
    current_node.op_type = node_proto.op_type();

    for (const auto &attribute : node_proto.attribute()) {
      if (auto value = attribute_to_string(attribute); !value.empty()) {
        current_node.attributes.emplace(attribute.name(), std::move(value));
      }
    }

    const int node_index = static_cast<int>(_graph.nodes.size()) - 1;
    for (const auto &input_name : node_proto.input()) {
      if (input_name.empty()) {
        continue;
      }
      const int tensor_index = ensure_tensor(
          input_name, {}, MODEL_TENSOR_DATA_TYPE_UNDEFINED, false);
      if (tensor_index < 0) {
        continue;
      }
      int source_node = -1;
      if (auto it = producer_map.find(input_name); it != producer_map.end()) {
        source_node = it->second;
      }
      const int edge_index =
          add_edge(input_name, tensor_index, source_node, node_index);
      current_node.input_edges.push_back(edge_index);
      if (source_node >= 0 &&
          source_node < static_cast<int>(_graph.nodes.size())) {
        _graph.nodes[source_node].output_edges.push_back(edge_index);
      }
    }

    for (const auto &output_name : node_proto.output()) {
      if (output_name.empty()) {
        continue;
      }
      ensure_tensor(output_name, {}, MODEL_TENSOR_DATA_TYPE_UNDEFINED, false);
      producer_map[output_name] = node_index;
    }
    ++node_counter;
  }

  for (const auto &output : graph_proto.output()) {
    if (output.name().empty()) {
      continue;
    }
    const auto it = tensor_index_by_name.find(output.name());
    if (it == tensor_index_by_name.end()) {
      continue;
    }
    int source_node = -1;
    if (auto prod_it = producer_map.find(output.name());
        prod_it != producer_map.end()) {
      source_node = prod_it->second;
    }
    const int edge_index = add_edge(output.name(), it->second, source_node, -1);
    if (source_node >= 0 &&
        source_node < static_cast<int>(_graph.nodes.size())) {
      _graph.nodes[source_node].output_edges.push_back(edge_index);
    }
  }

  std::cout << "Number of nodes: " << _graph.nodes.size() << std::endl;
  return true;
}
