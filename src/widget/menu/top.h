#pragma once

#include <string>

struct TopMenuState {
  bool show_demo_window = false;
  bool show_model_viewer = true;
  bool show_helper_window = true;
  bool show_inspector_panel = true;
  bool show_network_console = false;
  bool show_profiler_window = false;

  bool simulation_running = false;
  bool device_connected = false;
  bool network_monitoring = true;
  bool recording_traces = false;

  std::string current_model = "Untitled Model";
  std::string status_message = "Ready";
};

void ShowTopMenu(TopMenuState &state);
