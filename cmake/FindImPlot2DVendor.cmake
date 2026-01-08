include(FetchContent)
include(FindImGuiVendor)

# Allow user to override the ImNodeFlow repository and tag.
set(IMPLOT2D_VENDOR_GIT_REPOSITORY "https://github.com/epezent/implot.git" CACHE STRING "ImPlot2D git repository URL for FetchContent")
set(IMPLOT2D_VENDOR_GIT_TAG "81b8b19" CACHE STRING "ImPlot2D git tag or commit to fetch via FetchContent")

message(STATUS "Fetching ImPlot2D from ${IMPLOT2D_VENDOR_GIT_REPOSITORY} (tag: ${IMPLOT2D_VENDOR_GIT_TAG})")

FetchContent_Declare(
  ImPlot2D
  GIT_REPOSITORY "${IMPLOT2D_VENDOR_GIT_REPOSITORY}"
  GIT_TAG        "${IMPLOT2D_VENDOR_GIT_TAG}"
)

FetchContent_GetProperties(ImPlot2D)
if (NOT implot2d_POPULATED)
  FetchContent_MakeAvailable(ImPlot2D)
endif ()

if (NOT TARGET implot2d_vendor)
  file(GLOB IMPLOT2D_VENDOR_SOURCES
    ${implot2d_SOURCE_DIR}/implot.cpp
    ${implot2d_SOURCE_DIR}/implot.h
    ${implot2d_SOURCE_DIR}/implot_items.cpp
    ${implot2d_SOURCE_DIR}/implot_internal.h
  )

  add_library(implot2d_vendor STATIC ${IMPLOT2D_VENDOR_SOURCES})
  target_link_libraries(implot2d_vendor PUBLIC imgui::imgui)
  add_library(imgui::ImPlot2D ALIAS implot2d_vendor)

  message(STATUS "imgui::ImPlot2D target created from vendored sources")
endif ()

if (TARGET imgui::ImPlot2D)
  message(STATUS "imgui::ImPlot2D target is available from vendored ImPlot2D")
else ()
  message(WARNING "ImPlot2D vendored build completed but imgui::ImPlot2D target was not found. Check ImPlot2D's CMake configuration and adjust usage accordingly.")
endif ()
