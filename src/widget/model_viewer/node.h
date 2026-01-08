#pragma once

#include "ImNodeFlow.h"
#include <imgui.h>
#include <string>
#include <unordered_map>

#include "../../model/types.h"

// Represents an input of the neural network graph
class InputNode : public ImFlow::BaseNode {
public:
  InputNode() {
    setTitle("Input");
    setStyle(ImFlow::NodeStyle::green());

    // For now we use a single scalar value to keep behaviour simple
    ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() {
      return m_value;
    });
  }

  void draw() override {
    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("Value", &m_value);
  }

private:
  int m_value = 0;
};

// Represents a single neuron that sums its inputs and applies an
// activation-like function.
class NeuronNode : public ImFlow::BaseNode {
public:
  NeuronNode() {
    setTitle("Neuron");
    setStyle(ImFlow::NodeStyle::green());

    // Allow multiple inputs by naming them generically for now
    ImFlow::BaseNode::addIN<int>("In1", 0,
                                 ImFlow::ConnectionFilter::SameType());
    ImFlow::BaseNode::addIN<int>("In2", 0,
                                 ImFlow::ConnectionFilter::SameType());

    ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() {
      int sum = getInVal<int>("In1") + getInVal<int>("In2") + m_bias;
      // A very simple "activation" for now: clamp at zero (ReLU-like)
      return sum < 0 ? 0 : sum;
    });
  }

  void draw() override {
    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("Bias", &m_bias);
  }

private:
  int m_bias = 0;
};

// Represents an output of the neural network graph
class OutputNode : public ImFlow::BaseNode {
public:
  OutputNode() {
    setTitle("Output");
    setStyle(ImFlow::NodeStyle::brown());

    ImFlow::BaseNode::addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
  }

  void draw() override { ImGui::Text("Output: %d", getInVal<int>("In")); }
};

// Renders a model graph node with its op type, parameters, and tensor pins.
class ModelGraphNodeView : public ImFlow::BaseNode {
public:
  explicit ModelGraphNodeView(const sModelGraphNode *node) : m_node(node) {
    if (!m_node) {
      setTitle("invalid");
      return;
    }
    setTitle(!m_node->name.empty() ? m_node->name : "unnamed node");
    setStyle(ImFlow::NodeStyle::cyan());

    m_inputPins.reserve(m_node->inputs.size());
    for (std::size_t i = 0; i < m_node->inputs.size(); ++i) {
      const auto *edge = &m_node->inputs[i];
      auto label = make_pin_label(edge, "In", i);
      auto pin = ImFlow::BaseNode::addIN<float>(
          label, 0.f, ImFlow::ConnectionFilter::SameType());
      if (auto *tensor = tensor_desc(edge)) {
        m_inputPins[tensor] = pin.get();
      }
    }

    m_outputPins.reserve(m_node->outputs.size());
    for (std::size_t i = 0; i < m_node->outputs.size(); ++i) {
      const auto *edge = &m_node->outputs[i];
      auto label = make_pin_label(edge, "Out", i);
      auto pin = ImFlow::BaseNode::addOUT<float>(label);
      pin->behaviour([this]() { return 0.f; });
      if (auto *tensor = tensor_desc(edge)) {
        m_outputPins[tensor] = pin.get();
      }
    }
  }

  void draw() override {
    if (!m_node) {
      ImGui::TextUnformatted("<invalid node>");
      return;
    }
    ImGui::Text("Op: %s",
                !m_node->op_type.empty() ? m_node->op_type.c_str() : "Unknown");
    ImGui::Text("Params: %lld", static_cast<long long>(m_node->param_count));
    ImGui::Text("FLOPs: %.2f", m_node->flops_estimate);

    int attr_count = 0;
    for (const auto &[key, value] : m_node->attributes) {
      ImGui::Text("%s=%s", key.c_str(), value.c_str());
      if (++attr_count >= 3) {
        break;
      }
    }
  }

  ImFlow::Pin *inputPin(const sModelTensorDesc *tensor) const {
    auto it = m_inputPins.find(tensor);
    return it != m_inputPins.end() ? it->second : nullptr;
  }

  ImFlow::Pin *outputPin(const sModelTensorDesc *tensor) const {
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

  static const sModelTensorDesc *tensor_desc(const sModelGraphEdge *edge) {
    if (!edge) {
      return nullptr;
    }
    return &edge->tensor_desc;
  }

  const sModelGraphNode *m_node = nullptr;
  std::unordered_map<const sModelTensorDesc *, ImFlow::Pin *> m_inputPins;
  std::unordered_map<const sModelTensorDesc *, ImFlow::Pin *> m_outputPins;
};
