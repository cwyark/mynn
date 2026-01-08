#include "viewer.h"

#include <algorithm>
#include <functional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

ModelViewer::ModelViewer()
    : m_inspector(
          std::make_unique<ModelInspector>("models/MobileNet-v2.onnx")) {
  if (m_inspector && !m_inspector->nodes().empty()) {
    build_graph();
  }
  mINF.getGrid().config().scroll_button = ImGuiMouseButton_Right;
}

void ModelViewer::set_size(ImVec2 d) { mINF.setSize(d); }

void ModelViewer::draw() { mINF.update(); }

void ModelViewer::build_graph() {
  if (!m_inspector) {
    return;
  }

  const auto &nodes = m_inspector->nodes();
  if (nodes.empty()) {
    return;
  }

  std::unordered_map<const sModelGraphNode *, int> depth_cache;
  std::unordered_set<const sModelGraphNode *> visiting;
  std::function<int(const sModelGraphNode *)> compute_depth;

  compute_depth = [&](const sModelGraphNode *node) -> int {
    if (!node) {
      return 0;
    }
    auto it = depth_cache.find(node);
    if (it != depth_cache.end()) {
      return it->second;
    }
    if (!visiting.insert(node).second) {
      return 0;
    }
    int depth = 0;
    for (const auto &edge : node->inputs) {
      depth = std::max(depth, compute_depth(&edge.source_node) + 1);
    }
    visiting.erase(node);
    depth_cache[node] = depth;
    return depth;
  };

  for (const auto &node : nodes) {
    compute_depth(node.get());
  }

  std::map<int, std::vector<const sModelGraphNode *>> layers;
  for (const auto &node : nodes) {
    const auto *ptr = node.get();
    layers[depth_cache[ptr]].push_back(ptr);
  }

  constexpr float kHorizontalSpacing = 260.f;  // widen columns
  constexpr float kVerticalSpacing = 190.f;    // give rows more breathing room
  const float base_x = 40.f;
  const float base_y = 180.f;  // move graph slightly down for better start position

  constexpr int kMaxRowsPerColumn = 7;
  constexpr float kColumnSpacing = kHorizontalSpacing * 0.45f;

  m_node_views.clear();
  for (const auto &[layer, group_nodes] : layers) {
    const int node_count = static_cast<int>(group_nodes.size());
    const int columns =
        std::max(1, (node_count + kMaxRowsPerColumn - 1) / kMaxRowsPerColumn);

    for (int column = 0; column < columns; ++column) {
      const int start = column * kMaxRowsPerColumn;
      const int end = std::min(start + kMaxRowsPerColumn, node_count);
      const int column_nodes = end - start;
      const float column_span = (std::max(1, column_nodes) - 1) * kVerticalSpacing;

      float y_offset = base_y;
      if (column_span > 0.f) {
        y_offset = std::max(base_y, base_y - column_span * 0.5f);
      }

      float x = layer * kHorizontalSpacing + base_x + column * kColumnSpacing;
      for (int index = start; index < end; ++index) {
        ImVec2 pos = {x, y_offset + (index - start) * kVerticalSpacing};
        auto view = mINF.addNode<ModelGraphNodeView>(pos, group_nodes[index]);
        m_node_views.emplace(group_nodes[index], view);
      }
    }
  }

  for (const auto &node : nodes) {
    for (const auto &edge : node->outputs) {
      const auto *source = &edge.source_node;
      const auto *tail = &edge.tail_node;
      auto src_it = m_node_views.find(source);
      auto dst_it = m_node_views.find(tail);
      if (src_it == m_node_views.end() || dst_it == m_node_views.end()) {
        continue;
      }
      const auto *tensor = &edge.tensor_desc;
      auto *out_pin = src_it->second->outputPin(tensor);
      auto *in_pin = dst_it->second->inputPin(tensor);
      if (out_pin && in_pin) {
        out_pin->createLink(in_pin);
      }
    }
  }
}
