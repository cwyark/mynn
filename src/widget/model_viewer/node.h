#pragma once

#include "ImNodeFlow.h"
#include <imgui.h>
#include <string>
#include <unordered_map>

#include "../../model/types.h"

// Renders a model graph node with its op type, parameters, and tensor pins.
class ModelGraphNodeView : public ImFlow::BaseNode {
public:
  explicit ModelGraphNodeView(const sModelGraphNode *node,
                              const sModelGraph *graph)
      : m_node(node), m_graph(graph) {
    if (!m_node) {
      setTitle("invalid");
      return;
    }
    setTitle(!m_node->name.empty() ? m_node->name : "unnamed node");
    setStyle(ImFlow::NodeStyle::cyan());

    m_inputPins.reserve(m_node->input_edges.size());
    for (std::size_t i = 0; i < m_node->input_edges.size(); ++i) {
      const auto *edge = edge_at(m_node->input_edges[i]);
      auto label = make_pin_label(edge, "In", i);
      auto pin = ImFlow::BaseNode::addIN<float>(
          label, 0.f, ImFlow::ConnectionFilter::SameType());
      if (auto *tensor = tensor_for_edge(edge)) {
        m_inputPins[tensor] = pin.get();
      }
    }

    m_outputPins.reserve(m_node->output_edges.size());
    for (std::size_t i = 0; i < m_node->output_edges.size(); ++i) {
      const auto *edge = edge_at(m_node->output_edges[i]);
      auto label = make_pin_label(edge, "Out", i);
      auto pin = ImFlow::BaseNode::addOUT<float>(label);
      pin->behaviour([this]() { return 0.f; });
      if (auto *tensor = tensor_for_edge(edge)) {
        m_outputPins[tensor] = pin.get();
      }
    }
  }

  void draw() override {
    if (!m_node) {
      ImGui::TextUnformatted("<invalid node>");
      return;
    }
    if (!ImFlow::BaseNode::isSelected()) {
      return;
    }
    ImGui::Text("Op: %s",
                !m_node->op_type.empty() ? m_node->op_type.c_str() : "Unknown");

    show_edge_list("Inputs", m_node->input_edges, "In");
    show_edge_list("Outputs", m_node->output_edges, "Out");

    if (!m_node->attributes.empty()) {
      ImGui::Text("Attributes:");
      int attr_count = 0;
      for (const auto &[key, value] : m_node->attributes) {
        ImGui::Text("%s=%s", key.c_str(), value.c_str());
        if (++attr_count >= 3) {
          break;
        }
      }
    }
  }

  ImFlow::Pin *inputPin(const sModelTensor *tensor) const {
    auto it = m_inputPins.find(tensor);
    return it != m_inputPins.end() ? it->second : nullptr;
  }

  ImFlow::Pin *outputPin(const sModelTensor *tensor) const {
    auto it = m_outputPins.find(tensor);
    return it != m_outputPins.end() ? it->second : nullptr;
  }

private:
  static std::string make_pin_label(const sModelGraphEdge *edge,
                                    const char *fallback, std::size_t idx) {
    if (edge && !edge->name.empty()) {
      return edge->name;
    }
    return std::string(fallback) + std::to_string(idx);
  }

  const sModelGraphEdge *edge_at(int edge_index) const {
    if (!m_graph || edge_index < 0 ||
        edge_index >= static_cast<int>(m_graph->edges.size())) {
      return nullptr;
    }
    return &m_graph->edges[edge_index];
  }

  const sModelTensor *tensor_for_edge(const sModelGraphEdge *edge) const {
    if (!edge || !m_graph) {
      return nullptr;
    }
    const int tensor_index = edge->tensor_index;
    if (tensor_index < 0 ||
        tensor_index >= static_cast<int>(m_graph->tensors.size())) {
      return nullptr;
    }
    return &m_graph->tensors[tensor_index];
  }

  void show_edge_list(const char *label, const std::vector<int> &edges,
                      const char *fallback) const {
    if (!m_graph || edges.empty()) {
      return;
    }
    ImGui::Text("%s:", label);
    for (std::size_t idx = 0; idx < edges.size(); ++idx) {
      if (const auto *edge = edge_at(edges[idx])) {
        const auto text =
            make_pin_label(edge, fallback, static_cast<std::size_t>(idx));
        std::string decorated = text;
        if (const auto *tensor = tensor_for_edge(edge)) {
          decorated += " (" + tensor->name + ")";
        }
        ImGui::Text("%s", decorated.c_str());
      }
    }
  }

  const sModelGraphNode *m_node = nullptr;
  const sModelGraph *m_graph = nullptr;
  std::unordered_map<const sModelTensor *, ImFlow::Pin *> m_inputPins;
  std::unordered_map<const sModelTensor *, ImFlow::Pin *> m_outputPins;
};
