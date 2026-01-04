#
# This module uses CMake's FetchContent to download and make ImNodeFlow
# available to the current project. The ImNodeFlow URL can be overridden.
#
# Repository: https://github.com/Fattorino/ImNodeFlow.git
#
# Usage (in your CMakeLists.txt):
#   set(IMNODEFLOW_VENDOR_GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git" CACHE STRING "ImNodeFlow git repository URL")
#   set(IMNODEFLOW_VENDOR_GIT_TAG "master" CACHE STRING "ImNodeFlow git tag or commit to fetch")
#   include(cmake/FindImNodeFlowVendor.cmake)
#   # After inclusion, an ImNodeFlow import target should be available if
#   # ImNodeFlow's CMake configuration defines one (for example,
#   # ImNodeFlow::ImNodeFlow). If not, this module will create a basic
#   # ImNodeFlow::ImNodeFlow target from the sources.
#

include(FetchContent)
include(FindImGuiVendor)

# Allow user to override the ImNodeFlow repository and tag.
set(IMNODEFLOW_VENDOR_GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git" CACHE STRING "ImNodeFlow git repository URL for FetchContent")
set(IMNODEFLOW_VENDOR_GIT_TAG "master" CACHE STRING "ImNodeFlow git tag or commit to fetch via FetchContent")

# Optionally allow external ImNodeFlow installation instead of vendored one.
option(IMNODEFLOW_VENDOR_FORCE "Force using vendored ImNodeFlow via FetchContent" ON)

if (NOT IMNODEFLOW_VENDOR_FORCE)
  # Try to find an already installed ImNodeFlow first.
  find_package(ImNodeFlow QUIET)
  if (ImNodeFlow_FOUND)
    message(STATUS "Using system ImNodeFlow installation")
    return()
  endif()
endif()

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

# At this point, ImNodeFlow's CMake project (if present) should have created an
# imported target. We do not enforce the exact name here, but consumers can
# link against ImNodeFlow::ImNodeFlow if it exists.

# Fall back to building a simple ImNodeFlow library only if the upstream
# project did not already export a usable target. The exact sources may need
# adjustment if the upstream project layout changes.
if (NOT TARGET ImNodeFlow::ImNodeFlow)
  file(GLOB IMNODEFLOW_VENDOR_SOURCES
    ${imnodeflow_SOURCE_DIR}/src/*.cpp
  )

  add_library(imnodeflow_vendor STATIC ${IMNODEFLOW_VENDOR_SOURCES})
  target_include_directories(imnodeflow_vendor PUBLIC
    ${imnodeflow_SOURCE_DIR}/include
    ${imnodeflow_SOURCE_DIR}/src
  )
  target_compile_definitions(imnodeflow_vendor PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
  target_link_libraries(imnodeflow_vendor PUBLIC ImGui::ImGui)
  add_library(ImNodeFlow::ImNodeFlow ALIAS imnodeflow_vendor)

  message(STATUS "ImNodeFlow::ImNodeFlow target created from vendored sources")
endif ()

# After ensuring the target exists (either upstream or fallback) we can give
# users a clearer status.
if (TARGET ImNodeFlow::ImNodeFlow)
  if (NOT TARGET imnodeflow_vendor)
    target_compile_definitions(ImNodeFlow::ImNodeFlow PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
  endif()
  message(STATUS "ImNodeFlow::ImNodeFlow target is available from vendored ImNodeFlow")
else ()
  message(WARNING "ImNodeFlow vendored build completed but ImNodeFlow::ImNodeFlow target was not found. Check ImNodeFlow's CMake configuration and adjust usage accordingly.")
endif ()
