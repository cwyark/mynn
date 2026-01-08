#pragma once

#include <memory>
#include <string>
#include <vector>

#include "types.h"

class ModelInspector {
private:
  std::string _model_path;
  sModelGraph _graph;
  std::vector<std::unique_ptr<sModelGraphNode>> _nodes;
  std::vector<std::unique_ptr<sModelTensorDesc>> _tensor_descs;

public:
  ModelInspector(const std::string &model_path);
  std::string getName();
  bool load_model(const std::string &model_path);
  const sModelGraph &graph() const { return _graph; }
  const std::vector<std::unique_ptr<sModelGraphNode>> &nodes() const {
    return _nodes;
  }
};
