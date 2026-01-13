#include "viewer.h"

#include <limits>
#include <map>
#include <queue>
#include <unordered_map>
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

  const auto &graph = m_inspector->graph();
  const auto &nodes = graph.nodes;
  if (nodes.empty()) {
    return;
  }

  auto edge_at = [&](int edge_index) -> const sModelGraphEdge * {
    if (edge_index < 0 || edge_index >= static_cast<int>(graph.edges.size())) {
      return nullptr;
    }
    return &graph.edges[edge_index];
  };
  const int node_count = static_cast<int>(nodes.size());
  std::vector<int> depth(node_count, std::numeric_limits<int>::max());
  std::queue<int> bfs_queue;

  for (int i = 0; i < node_count; ++i) {
    bool is_root = true;
    for (int edge_index : nodes[i].input_edges) {
      if (const auto *edge = edge_at(edge_index)) {
        if (edge->source_node >= 0) {
          is_root = false;
          break;
        }
      }
    }
    if (is_root) {
      depth[i] = 0;
      bfs_queue.push(i);
    }
  }

  while (!bfs_queue.empty()) {
    const int current = bfs_queue.front();
    bfs_queue.pop();
    const int current_depth = depth[current];
    for (int edge_index : nodes[current].output_edges) {
      if (const auto *edge = edge_at(edge_index)) {
        const int target = edge->target_node;
        if (target < 0 || target >= node_count) {
          continue;
        }
        if (depth[target] > current_depth + 1) {
          depth[target] = current_depth + 1;
          bfs_queue.push(target);
        }
      }
    }
  }

  std::map<int, std::vector<const sModelGraphNode *>> layers;
  for (int i = 0; i < node_count; ++i) {
    const auto *node = &nodes[i];
    const int node_depth =
        depth[i] == std::numeric_limits<int>::max() ? 0 : depth[i];
    layers[node_depth].push_back(node);
  }

  m_node_views.clear();
  constexpr float layer_spacing_x = 260.f;
  constexpr float node_spacing_y = 200.f;
  const float base_x = 80.f;
  const float base_y = 360.f;

  for (const auto &[layer_depth, group_nodes] : layers) {
    const int per_layer = static_cast<int>(group_nodes.size());
    const float column_height =
        per_layer > 1 ? (per_layer - 1) * node_spacing_y : 0.f;
    const float start_y = base_y - column_height * 0.5f;
    const float x = base_x + layer_depth * layer_spacing_x;

    for (int idx = 0; idx < per_layer; ++idx) {
      float y = start_y + idx * node_spacing_y;
      ImVec2 pos = {x, y};
      auto view =
          mINF.addNode<ModelGraphNodeView>(pos, group_nodes[idx], &graph);
      m_node_views.emplace(group_nodes[idx], view);
    }
  }

  auto tensor_for_edge =
      [&](const sModelGraphEdge &edge) -> const sModelTensor * {
    if (edge.tensor_index < 0 ||
        edge.tensor_index >= static_cast<int>(graph.tensors.size())) {
      return nullptr;
    }
    return &graph.tensors[edge.tensor_index];
  };

  for (const auto &edge : graph.edges) {
    if (edge.source_node < 0 || edge.target_node < 0) {
      continue;
    }
    if (edge.source_node >= static_cast<int>(nodes.size()) ||
        edge.target_node >= static_cast<int>(nodes.size())) {
      continue;
    }
    const auto *source = &nodes[edge.source_node];
    const auto *target = &nodes[edge.target_node];
    auto src_it = m_node_views.find(source);
    auto dst_it = m_node_views.find(target);
    if (src_it == m_node_views.end() || dst_it == m_node_views.end()) {
      continue;
    }
    const auto *tensor = tensor_for_edge(edge);
    auto *out_pin = src_it->second->outputPin(tensor);
    auto *in_pin = dst_it->second->inputPin(tensor);
    if (out_pin && in_pin) {
      out_pin->createLink(in_pin);
    }
  }
}
