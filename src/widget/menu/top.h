#pragma once

struct TopMenuState {
  bool show_demo_window = true;
  bool show_graph_viewer = false;
  bool show_helper_window = true;
};

void ShowTopMenu(TopMenuState &state);
