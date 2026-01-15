#include "top.h"

#include "imgui.h"

namespace {

constexpr const char *kRecentModels[] = {"resnet50.onnx",
                                         "mobilenet_v3_small.onnx",
                                         "bert_base.onnx", "pruned-unet.onnx"};

inline void UpdateStatus(TopMenuState &state, const char *message) {
  state.status_message = message;
}

inline void OpenModel(TopMenuState &state, const char *path) {
  state.current_model = path;
  UpdateStatus(state, "Model loaded");
}

} // namespace

void ShowTopMenu(TopMenuState &state) {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Workspace", "Ctrl+Shift+N")) {
      state.current_model = "Untitled Model";
      state.simulation_running = false;
      state.recording_traces = false;
      state.device_connected = false;
      state.network_monitoring = true;
      UpdateStatus(state, "Workspace reset");
    }

    if (ImGui::MenuItem("Open Model...", "Ctrl+O")) {
      UpdateStatus(state, "Open model dialog triggered");
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
    if (ImGui::MenuItem("Save Graph", "Ctrl+S", false,
                        !state.current_model.empty())) {
      UpdateStatus(state, "Current graph saved");
    }
    if (ImGui::MenuItem("Export ONNX", nullptr, false,
                        !state.current_model.empty())) {
      UpdateStatus(state, "ONNX export started");
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Exit", "Alt+F4")) {
      UpdateStatus(state, "Exit requested");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Model")) {
    if (ImGui::MenuItem("Inspect Graph", "Ctrl+I")) {
      state.show_inspector_panel = true;
      UpdateStatus(state, "Inspector opened");
    }
    if (ImGui::MenuItem("Refresh Layout", "Ctrl+R")) {
      UpdateStatus(state, "Graph layout refreshed");
    }
    if (ImGui::MenuItem("Optimize", nullptr)) {
      UpdateStatus(state, "Optimization pass queued");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Network")) {
    const char *label =
        state.device_connected ? "Disconnect Device" : "Connect to Device";
    if (ImGui::MenuItem(label, "Ctrl+K")) {
      state.device_connected = !state.device_connected;
      UpdateStatus(state, state.device_connected ? "Device connected"
                                                 : "Device disconnected");
    }
    ImGui::MenuItem("Monitor Traffic", "Ctrl+M", &state.network_monitoring);
    if (ImGui::MenuItem("Send Test Packet", "Ctrl+P")) {
      UpdateStatus(state, "Test packet dispatched");
    }
    if (ImGui::MenuItem("Deploy Graph", "Ctrl+D", false,
                        state.device_connected)) {
      state.simulation_running = true;
      UpdateStatus(state, "Deployment started");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Tools")) {
    ImGui::MenuItem("Network Console", nullptr, &state.show_network_console);
    ImGui::MenuItem("Performance Profiler", nullptr,
                    &state.show_profiler_window);
    ImGui::MenuItem("Recording Traces", "Ctrl+T", &state.recording_traces);
    if (ImGui::MenuItem("Run Diagnostics", "Ctrl+D")) {
      UpdateStatus(state, "Diagnostics running");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Model Viewer", nullptr, &state.show_model_viewer);
    ImGui::MenuItem("Inspector", nullptr, &state.show_inspector_panel);
    ImGui::MenuItem("Helper Window", nullptr, &state.show_helper_window);
    ImGui::MenuItem("Demo Window", nullptr, &state.show_demo_window);
    ImGui::MenuItem("Network Console", nullptr, &state.show_network_console);
    ImGui::MenuItem("Profiler", nullptr, &state.show_profiler_window);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Help")) {
    if (ImGui::MenuItem("About Neuton Network Tool")) {
      UpdateStatus(state, "Neuton network tool info requested");
    }
    ImGui::EndMenu();
  }

  ImGui::SameLine();
  ImGui::Text("Model: %s", state.current_model.c_str());
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.9f, 1.0f), "%s",
                     state.status_message.c_str());
  ImGui::EndMainMenuBar();
}
