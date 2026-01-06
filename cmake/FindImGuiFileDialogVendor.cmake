# https://github.com/aiekick/ImGuiFileDialog

include(FetchContent)
include(FindImGuiVendor)

set(IMGUIFILEDIALOG_VENDOR_VERSION "0.6.8" CACHE STRING "ImGuiFileDialog version to fetch via FetchContent")

set(IMGUIFILEDIALOG_VENDOR_URL "https://github.com/aiekick/ImGuiFileDialog/archive/refs/tags/v${IMGUIFILEDIALOG_VENDOR_VERSION}.tar.gz"
  CACHE STRING "ImGuiFileDialog source tarball URL")

message(STATUS "Fetching ImGuiFileDialog v${IMGUIFILEDIALOG_VENDOR_VERSION} from ${IMGUIFILEDIALOG_VENDOR_URL}")

FetchContent_Declare(
  ImGuiFileDialog
  URL       "${IMGUIFILEDIALOG_VENDOR_URL}"
  URL_HASH  ""
)

FetchContent_GetProperties(ImGuiFileDialog)
if (NOT imguifiledialog_POPULATED)
  FetchContent_MakeAvailable(ImGuiFiledialog)
endif ()

if (TARGET ImGuiFileDialog AND NOT TARGET imguifiledialog_vendor)
  target_link_libraries(ImGuiFileDialog PUBLIC imgui)
  add_library(imguifiledialog_vendor ALIAS ImGuiFileDialog)
  add_library(imgui::ImGuiFileDialog ALIAS ImGuiFileDialog)
endif ()

if (TARGET imgui::ImGuiFileDialog)
  message(STATUS "imgui::ImGuiFileDialog target is available from vendored ImGuiFileDialog")
else ()
  message(WARNING "ImGuiFileDialog vendored build completed but imgui::ImGuiFileDialog target was not found. Check ImGuiFileDialog's CMake configuration and adjust usage accordingly.")
endif ()
