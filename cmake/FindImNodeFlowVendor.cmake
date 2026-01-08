# Repository: https://github.com/Fattorino/ImNodeFlow.git

include(FetchContent)
include(FindImGuiVendor)

# Allow user to override the ImNodeFlow repository and tag.
set(IMNODEFLOW_VENDOR_GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git" CACHE STRING "ImNodeFlow git repository URL for FetchContent")
set(IMNODEFLOW_VENDOR_GIT_TAG "aef96d1" CACHE STRING "ImNodeFlow git tag or commit to fetch via FetchContent")

message(STATUS "Fetching ImNodeFlow from ${IMNODEFLOW_VENDOR_GIT_REPOSITORY} (tag: ${IMNODEFLOW_VENDOR_GIT_TAG})")

FetchContent_Declare(
  ImNodeFlow
  GIT_REPOSITORY "${IMNODEFLOW_VENDOR_GIT_REPOSITORY}"
  GIT_TAG        "${IMNODEFLOW_VENDOR_GIT_TAG}"
)

FetchContent_GetProperties(ImNodeFlow)
if (NOT imnodeflow_POPULATED)
  FetchContent_MakeAvailable(ImNodeFlow)
endif ()

if (NOT TARGET imnodeflow_vendor)
  file(GLOB IMNODEFLOW_VENDOR_SOURCES
    ${imnodeflow_SOURCE_DIR}/src/*.cpp
  )

  add_library(imnodeflow_vendor STATIC ${IMNODEFLOW_VENDOR_SOURCES})
  target_include_directories(imnodeflow_vendor PUBLIC
    ${imnodeflow_SOURCE_DIR}/include
    ${imnodeflow_SOURCE_DIR}/src
  )
  target_compile_definitions(imnodeflow_vendor PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
  target_link_libraries(imnodeflow_vendor PUBLIC imgui::imgui)
  add_library(imgui::ImNodeFlow ALIAS imnodeflow_vendor)

  message(STATUS "imgui::ImNodeFlow target created from vendored sources")
endif ()

if (TARGET imgui::ImNodeFlow)
  message(STATUS "imgui::ImNodeFlow target is available from vendored ImNodeFlow")
else ()
  message(WARNING "ImNodeFlow vendored build completed but imgui::ImNodeFlow target was not found. Check ImNodeFlow's CMake configuration and adjust usage accordingly.")
endif ()
