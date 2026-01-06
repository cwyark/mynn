cmake_minimum_required(VERSION 4.0.0)

project(mynn)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find Libraries: SDL3, ImGui, ImNodeFlow
include(FindSDL3Vendor)
include(FindImGuiVendor)
include(FindImNodeFlowVendor)
include(FindTorchVendor)
include(FindOnnxRuntimeVendor)

set(target "${CMAKE_PROJECT_NAME}")
set(source_files
  src/main.cpp
  src/node.cpp
  src/node.h
  src/model/inspector.cpp
  src/model/inspector.h
  src/model/types.h
)

add_executable(${target} ${source_files})
target_link_libraries(${target} PRIVATE
  SDL3::SDL3
  ImGui::ImGui
  ImNodeFlow::ImNodeFlow
  ${TORCH_LIBRARIES}
  onnxruntime
)
target_link_directories(${target} PRIVATE ${TORCH_INCLUDE_DIRS})
