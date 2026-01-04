#
# This module uses CMake's FetchContent to download and make ImGui available
# to the current project. The ImGui version and URL can be overridden.
#
# Usage (in your CMakeLists.txt):
#   set(IMGUI_VENDOR_VERSION "1.92.5" CACHE STRING "ImGui version to fetch")
#   set(IMGUI_VENDOR_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VENDOR_VERSION}.tar.gz" CACHE STRING "ImGui source tarball URL")
#   include(cmake/FindImGuiVendor.cmake)
#   # After inclusion, an ImGui import target should be available if ImGui's
#   # CMake configuration defines one (for example, ImGui::ImGui).
#

include(FetchContent)
include(FindSDL3Vendor)

# Allow user to override the ImGui version from the cache or command line.
set(IMGUI_VENDOR_VERSION "1.92.5" CACHE STRING "ImGui version to fetch via FetchContent")

# Default URL can be overridden as well.
set(IMGUI_VENDOR_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VENDOR_VERSION}.tar.gz" CACHE STRING "ImGui source tarball URL for ImGui")

# Optionally allow external ImGui installation instead of vendored one.
option(IMGUI_VENDOR_FORCE "Force using vendored ImGui via FetchContent" ON)

if (NOT IMGUI_VENDOR_FORCE)
  # Try to find an already installed ImGui first.
  find_package(ImGui QUIET)
  if (ImGui_FOUND)
    message(STATUS "Using system ImGui installation")
    return()
  endif()
endif()

message(STATUS "Fetching ImGui v${IMGUI_VENDOR_VERSION} from ${IMGUI_VENDOR_URL}")

FetchContent_Declare(
    ImGui
    URL              "${IMGUI_VENDOR_URL}"
    URL_HASH         ""
)

FetchContent_GetProperties(ImGui)
if (NOT imgui_POPULATED)
  FetchContent_MakeAvailable(ImGui)
endif ()

# At this point, ImGui's CMake project (if present) should have created an
# imported target. We do not enforce the exact name here, but consumers can
# link against ImGui::ImGui if it exists.

# Fall back to compiling a simple ImGui library only if the upstream project
# did not already export a usable target.
if (NOT TARGET ImGui::ImGui)
  set(IMGUI_VENDOR_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
  )

  add_library(imgui_vendor STATIC ${IMGUI_VENDOR_SOURCES})
  target_include_directories(imgui_vendor PUBLIC ${imgui_SOURCE_DIR})
  target_link_libraries(imgui_vendor PUBLIC SDL3::SDL3)
  add_library(ImGui::ImGui ALIAS imgui_vendor)

  message(STATUS "ImGui::ImGui target created from vendored sources")
endif ()

# After ensuring the target exists (either upstream or fallback) we can give
# users a clearer status.
if (TARGET ImGui::ImGui)
  message(STATUS "ImGui::ImGui target is available from vendored ImGui")
else ()
  message(WARNING "ImGui vendored build completed but ImGui::ImGui target was not found. Check ImGui's CMake configuration and adjust usage accordingly.")
endif ()
