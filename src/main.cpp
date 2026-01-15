#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <imgui.h>
#include <memory>

#include "widget/model_viewer/viewer.h"

#if 0
#include <torch/torch.h>
#endif

int main() {
  SDL_Log("start mynn program");
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("unable to init SDL3");
    SDL_Quit();
  }

  constexpr int width = 1920;
  constexpr int height = 1280;
  SDL_Window *window =
      SDL_CreateWindow("MyNN", width, height, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Log("unable to create SDL3 window");
    SDL_Quit();
    return -1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    SDL_Log("unable to create SDL3 renderer");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // make the fonts more sharp to look a higher resolution.
  io.Fonts->Clear();
  ImFontConfig font_cfg;
  font_cfg.OversampleH = 3; // better quality
  font_cfg.OversampleV = 3;
  font_cfg.PixelSnapH = false;
  io.Fonts->AddFontFromFileTTF("./font/DroidSans.ttf", 20.0f, &font_cfg);

  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();

  style.FontSizeBase = 15.0f;
  style.FontScaleMain = 1.0f;
  style.FontScaleDpi = 1.0f;

  if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) ||
      !ImGui_ImplSDLRenderer3_Init(renderer)) {
    SDL_Log("unable to init ImGui for SDL renderer");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  bool running = true;
  SDL_Event e;

  auto model_viewer = std::make_shared<ModelViewer>();
  bool show_demo_window = true;
  bool show_model_viewer = true;

  while (running) {
    while (SDL_PollEvent(&e)) {
      ImGui_ImplSDL3_ProcessEvent(&e);
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          e.window.windowID == SDL_GetWindowID(window)) {
        running = false;
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
      ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
      ImGuiID dock_id_left_sidebar = 0;
      ImGuiID dock_id_main = dockspace_id;
      ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f,
                                  &dock_id_left_sidebar, &dock_id_main);
      ImGuiID dock_id_left_sidebar_top = 0;
      ImGuiID dock_id_left_sidebar_bottom = 0;
      ImGui::DockBuilderSplitNode(dock_id_left_sidebar, ImGuiDir_Up, 0.50f,
                                  &dock_id_left_sidebar_top,
                                  &dock_id_left_sidebar_bottom);
      ImGui::DockBuilderDockWindow("Model Viewer", dock_id_main);
      ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_left_sidebar_top);
      ImGui::DockBuilderDockWindow("Helper Window",
                                   dock_id_left_sidebar_bottom);
      ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("Windows")) {
        ImGui::MenuItem("Demo Window", nullptr, &show_demo_window);
        ImGui::MenuItem("Model Viewer", nullptr, &show_model_viewer);
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (show_demo_window) {
      ImGui::ShowDemoWindow(&show_demo_window);
    }

    if (show_model_viewer) {
      if (ImGui::Begin("Model Viewer", &show_model_viewer,
                       ImGuiWindowFlags_None)) {
        {
          ImVec2 content_area_size = ImGui::GetWindowContentRegionMax();
          // model_viewer->set_size(content_area_size);
          model_viewer->set_size(ImVec2{0, 0});
        }
        model_viewer->draw();
        ImGui::End();
      }
    }

    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
  }

  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
