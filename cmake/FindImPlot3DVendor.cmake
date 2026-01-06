include(FetchContent)
include(FindImGuiVendor)

set(IMPLOT3D_VENDOR_GIT_REPOSITORY "https://github.com/brenocq/implot3d.git" CACHE STRING "ImPlot3D git repository URL for FetchContent")
set(IMPLOT3D_VENDOR_GIT_TAG "5981bc5" CACHE STRING "ImPlot3D git tag or commit to fetch via FetchContent")

message(STATUS "Fetching ImPlot3D from ${IMPLOT3D_VENDOR_GIT_REPOSITORY} (tag: ${IMPLOT3D_VENDOR_GIT_TAG})")

FetchContent_Declare(
  ImPlot3D
  GIT_REPOSITORY "${IMPLOT3D_VENDOR_GIT_REPOSITORY}"
  GIT_TAG        "${IMPLOT3D_VENDOR_GIT_TAG}"
)

FetchContent_GetProperties(ImPlot3D)
if (NOT implot3d_POPULATED)
  FetchContent_MakeAvailable(ImPlot3D)
endif ()

if (NOT TARGET implot3d_vendor)
  file(GLOB IMPLOT3D_VENDOR_SOURCES
    ${implot3d_SOURCE_DIR}/implot3d.cpp
    ${implot3d_SOURCE_DIR}/implot3d.h
    ${implot3d_SOURCE_DIR}/implot3d_items.cpp
    ${implot3d_SOURCE_DIR}/implot3d_meshes.cpp
    ${implot3d_SOURCE_DIR}/implot3d_internal.h
  )

  add_library(implot3d_vendor STATIC ${IMPLOT3D_VENDOR_SOURCES})
  target_link_libraries(implot3d_vendor PUBLIC imgui::imgui)
  add_library(imgui::ImPlot3D ALIAS implot3d_vendor)

  message(STATUS "imgui::ImPlot3D target created from vendored sources")
endif ()

if (TARGET imgui::ImPlot2D)
  message(STATUS "imgui::ImPlot3D target is available from vendored ImPlot3D")
else ()
  message(WARNING "ImPlot3D vendored build completed but imgui::ImPlot3D target was not found. Check ImPlot3D's CMake configuration and adjust usage accordingly.")
endif ()
