#pragma once

#include <string>

#include "types.h"

class ModelInspector {
private:
  std::string _model_path;
  sModelGraph _graph;

public:
  ModelInspector(const std::string &model_path);
  std::string getName();
  bool load_model(const std::string &model_path);
  const sModelGraph &graph() const { return _graph; }
  const std::vector<sModelGraphNode> &nodes() const { return _graph.nodes; }
};
