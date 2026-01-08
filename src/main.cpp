#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <cmath>
#include <imgui.h>
#include <memory>

#include "widget/model_viewer/node.h"
#include "widget/model_viewer/viewer.h"

#include "model/inspector.h"
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
  // ImGuiIO &io = ImGui::GetIO();
  // io.Fonts->Clear();

  ImGui::StyleColorsDark();

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

    // show widget
    static bool show_preview_window = true;
    if (show_preview_window) {
      if (ImGui::Begin("Preview Window", &show_preview_window,
                       ImGuiWindowFlags_MenuBar)) {
        ImGui::Text("Hello World!");
#if 0
      if (ImGui::Button("create an tensor")) {
        torch::Tensor tensor = torch::rand({2, 3});
        std::cout << tensor << std::endl;
      }
#endif
        float samples[120];
        for (int i = 0; i < 120; ++i) {
          samples[i] = sinf(i * 0.2f + ImGui::GetTime() * 1.5f);
        }
        ImGui::PlotLines("Samples", samples, IM_ARRAYSIZE(samples));
        ImGui::PlotHistogram("Samples", samples, IM_ARRAYSIZE(samples));
        ImGui::End();
      }
    }

    static bool show_model_viewer = true;
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
