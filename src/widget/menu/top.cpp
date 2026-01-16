#include "top.h"

#include "imgui.h"

namespace {

constexpr const char *kRecentModels[] = {"resnet50.onnx",
                                         "mobilenet_v3_small.onnx",
                                         "bert_base.onnx", "pruned-unet.onnx"};

inline void UpdateStatus(TopMenuState &state, const char *message) {
}

inline void OpenModel(TopMenuState &state, const char *path) {
  UpdateStatus(state, "Model loaded");
}

} // namespace

void ShowTopMenu(TopMenuState &state) {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Workspace")) {
    }

    if (ImGui::MenuItem("Open Model...")) {
    }

    if (ImGui::BeginMenu("Recent Models")) {
      for (const char *model : kRecentModels) {
        if (ImGui::MenuItem(model)) {
          OpenModel(state, model);
        }
      }
      ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Exit")) {
      UpdateStatus(state, "Exit requested");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Tools")) {
    ImGui::MenuItem("Graph Viewer", nullptr, &state.show_graph_viewer);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Helper Window", nullptr, &state.show_helper_window);
    ImGui::MenuItem("Dear ImGui Demo", nullptr, &state.show_demo_window);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Help")) {
    if (ImGui::MenuItem("About MyNN")) {
    }
    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}
