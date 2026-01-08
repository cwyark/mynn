cmake_minimum_required(VERSION 4.0.0)

project(mynn)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find Libraries: SDL3, ImGui, ImNodeFlow
include(FindSDL3Vendor)
include(FindImGuiVendor)
include(FindImNodeFlowVendor)
include(FindImGuiFileDialogVendor)
include(findImPlot2DVendor)
include(findImPlot3DVendor)
include(FindOnnxRuntimeVendor)
include(FindTorchVendor)
include(FindProtobufVendor)
include(FindOnnxVendor)

set(target "${CMAKE_PROJECT_NAME}")
set(source_files
  src/main.cpp
  src/widget/model_viewer/node.cpp
  src/widget/model_viewer/node.h
  src/widget/model_viewer/viewer.cpp
  src/widget/model_viewer/viewer.h
  src/model/inspector.cpp
  src/model/inspector.h
  src/model/types.h
)

add_executable(${target} ${source_files})
set(_target_link_libs
  SDL3::SDL3
  imgui::imgui
  imgui::ImNodeFlow
  imgui::ImGuiFileDialog
  imgui::ImPlot2D
  imgui::ImPlot3D
  onnxruntime
  onnx
  onnx_proto
  ${Protobuf_LIBRARIES}
)
target_link_libraries(${target} PRIVATE ${_target_link_libs})
