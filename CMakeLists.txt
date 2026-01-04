

cmake_minimum_required(VERSION 4.0.0)

project(mynn)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find Lobraries: SDL3, ImGui
include(FindSDL3Vendor)
include(FindImGuiVendor)
include(FindImNodeFlowVendor)


set(target "${CMAKE_PROJECT_NAME}")
set(source_files
  src/main.cpp
  src/node.cpp
  src/node.h
)

add_executable(${target} ${source_files})
target_link_libraries(${target} PRIVATE
  SDL3::SDL3
  ImGui::ImGui
  ImNodeFlow::ImNodeFlow
)
