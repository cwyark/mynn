# Repository: https://github.com/Fattorino/ImNodeFlow.git

include(FetchContent)
include(FindImGuiVendor)

# Allow user to override the ImNodeFlow repository and tag.
set(IMNODEFLOW_VENDOR_GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git" CACHE STRING "ImNodeFlow git repository URL for FetchContent")
set(IMNODEFLOW_VENDOR_GIT_TAG "aef96d1" CACHE STRING "ImNodeFlow git tag or commit to fetch via FetchContent")

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

if (NOT TARGET ImNodeFlow)
  file(GLOB IMNODEFLOW_VENDOR_SOURCES
    ${imnodeflow_SOURCE_DIR}/src/*.cpp
  )

  add_library(imnodeflow_vendor STATIC ${IMNODEFLOW_VENDOR_SOURCES})
  target_include_directories(imnodeflow_vendor PUBLIC
    ${imnodeflow_SOURCE_DIR}/include
    ${imnodeflow_SOURCE_DIR}/src
  )
  target_compile_definitions(imnodeflow_vendor PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
  target_link_libraries(imnodeflow_vendor PUBLIC ImGui)
  add_library(ImNodeFlow ALIAS imnodeflow_vendor)

  message(STATUS "ImNodeFlow target created from vendored sources")
endif ()

if (TARGET ImNodeFlow)
  if (NOT TARGET imnodeflow_vendor)
    target_compile_definitions(ImNodeFlow PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
  endif()
  message(STATUS "ImNodeFlow target is available from vendored ImNodeFlow")
else ()
  message(WARNING "ImNodeFlow vendored build completed but ImNodeFlow target was not found. Check ImNodeFlow's CMake configuration and adjust usage accordingly.")
endif ()
