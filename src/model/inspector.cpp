#include "inspector.h"
#include <iostream>
#include <onnxruntime_cxx_api.h>
#include <string>

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

  return true;
}
