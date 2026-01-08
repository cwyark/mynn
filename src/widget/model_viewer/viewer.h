#pragma once

#include <memory>
#include <unordered_map>

#include "ImNodeFlow.h"
#include "imgui.h"

#include "../../model/inspector.h"
#include "node.h"

class ModelViewer {
public:
  ModelViewer();
  void set_size(ImVec2 d);
  void draw();

private:
  void build_graph();

  ImFlow::ImNodeFlow mINF;
  std::unique_ptr<ModelInspector> m_inspector;
  std::unordered_map<const sModelGraphNode *,
                     std::shared_ptr<ModelGraphNodeView>>
      m_node_views;
};
