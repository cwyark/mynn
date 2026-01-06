#include "inspector.h"
#include <fstream>
#include <onnx/onnx_pb.h>
#include <onnxruntime_cxx_api.h>

ModelInspector::ModelInspector(const std::string &model_path)
    : _model_path(model_path) {
  load_model(_model_path);
}

bool ModelInspector::load_model(const std::string &model_path) {
  // use onnxruntime api to get the model info.
  Ort::Env env{ORT_LOGGING_LEVEL_INFO, "model-inspector"};
  Ort::SessionOptions session_options;
  Ort::Session session = Ort::Session(env, _model_path.data(), session_options);
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
    // Use std::string directly to construct fstream to avoid pointer/length
    // issues
    std::fstream input(model_path, std::ios::in | std::ios::binary);
    model_proto.ParseFromIstream(&input);
  }

  auto &graph = model_proto.graph();
  std::cout << "Number of nodes: " << graph.node_size() << std::endl;

  for (int i = 0; i < graph.node_size(); ++i) {
    auto &node = graph.node(i);
    std::cout << "Node " << i << " op_type=" << node.op_type()
              << ", name=" << node.name() << ", input=[";
    for (auto &input : node.input())
      std::cout << input << " ";
    std::cout << "], outputs=[";
    for (auto &output : node.output())
      std::cout << output << " ";
    std::cout << "]" << std::endl;
  }

  return true;
}
