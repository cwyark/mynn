include(FetchContent)
include(FindSDL3Vendor)

set(IMGUI_VENDOR_VERSION "1.92.5" CACHE STRING "ImGui version to fetch via FetchContent")
set(IMGUI_VENDOR_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VENDOR_VERSION}.tar.gz" CACHE STRING "ImGui source tarball URL for ImGui")

message(STATUS "Fetching ImGui v${IMGUI_VENDOR_VERSION} from ${IMGUI_VENDOR_URL}")

FetchContent_Declare(
    imgui
    URL       "${IMGUI_VENDOR_URL}"
    URL_HASH  ""
)

FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
  FetchContent_MakeAvailable(imgui)
endif ()

if (NOT TARGET imgui_vendor)
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
  add_library(imgui::imgui ALIAS imgui_vendor)
  add_library(imgui ALIAS imgui_vendor)

  message(STATUS "imgui::imgui target created from vendored sources")
endif ()

if (TARGET imgui::imgui)
  message(STATUS "imgui::imgui target is available from vendored ImGui")
else ()
  message(WARNING "imgui::imgui vendored build completed but imgui::imgui target was not found. Check ImGui's CMake configuration and adjust usage accordingly.")
endif ()
