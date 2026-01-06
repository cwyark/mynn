#==========================================================================
# CMake helper that locates an ONNX Runtime installation and exposes it
#like the other vendored helpers in this project.
#
# Usage (in your CMakeLists.txt):
#   set(ONNXRUNTIME_VENDOR_DIR "/path/to/onnxruntime" CACHE PATH
#       "Root of an ONNX Runtime extraction containing cmake/onnxruntime-config.cmake.")
#   set(ONNXRUNTIME_VENDOR_DOWNLOAD_URL "<archive URL>" CACHE STRING
#       "Optional URL to fetch an ONNX Runtime bundle via FetchContent.")
#   option(ONNXRUNTIME_VENDOR_FORCE
#       "Force using the vendored ONNX Runtime even if a system installation is found." OFF)
#   include(cmake/FindOnnxRuntimeVendor.cmake)
#   # After inclusion, the ONNX Runtime targets (e.g. onnxruntime::onnxruntime)
#   should be available.
#==========================================================================

include(FetchContent)

set(ONNXRUNTIME_VENDOR_VERSION "1.23.2" CACHE STRING "ONNX Runtime release version to fetch.")
set(ONNXRUNTIME_VENDOR_DOWNLOAD_URL "" CACHE STRING
    "Optional URL pointing to an ONNX Runtime archive (tarball/zip). FetchContent will download and extract it.")
set(ONNXRUNTIME_VENDOR_DIR "" CACHE PATH
    "Root directory of an ONNX Runtime extraction (must contain cmake/onnxruntime-config.cmake).")
option(ONNXRUNTIME_VENDOR_FORCE
       "Require using the vendored ONNX Runtime even if a system installation is detected." OFF)

# When ONNXRUNTIME_VENDOR_DOWNLOAD_URL is explicitly set by the user, mirror the
# simple FetchContent-based pattern from the main CMakeLists and expose an
# imported "onnxruntime" target directly.
set(ONNXRUNTIME_URL "${ONNXRUNTIME_VENDOR_DOWNLOAD_URL}")
if (NOT ONNXRUNTIME_URL STREQUAL "")
  FetchContent_Declare(
    onnxruntime_prebuilt
    URL ${ONNXRUNTIME_URL}
  )
  FetchContent_MakeAvailable(onnxruntime_prebuilt)

  set(ONNXRUNTIME_ROOT "${onnxruntime_prebuilt_SOURCE_DIR}")
  set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT}/include")
  set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT}/lib")

  add_library(onnxruntime SHARED IMPORTED GLOBAL)
  if (APPLE)
    set_target_properties(onnxruntime PROPERTIES
      IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.dylib"
      INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )
  elseif (WIN32)
    set_target_properties(onnxruntime PROPERTIES
      IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/onnxruntime.dll"
      INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )
  else()
    set_target_properties(onnxruntime PROPERTIES
      IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
      INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )
  endif()

  # When we have explicitly constructed a prebuilt onnxruntime target, we can
  # return early and avoid the rest of the vendoring logic below.
  message(STATUS "Using prebuilt onnxruntime from ${ONNXRUNTIME_ROOT}")
  return()
endif()

# Provide per-platform defaults for the ONNX Runtime bundle /if/ no override URL was supplied.
# This block only sets defaults when neither a directory nor a download URL were provided.
if (ONNXRUNTIME_VENDOR_DIR STREQUAL "" AND ONNXRUNTIME_VENDOR_DOWNLOAD_URL STREQUAL "")
  set(_onnx_release_base_url
      "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VENDOR_VERSION}/")
  if (WIN32)
    set(_onnx_default_archive "onnxruntime-win-x64-${ONNXRUNTIME_VENDOR_VERSION}.zip")
  elseif (APPLE)
    set(_onnx_default_archive "onnxruntime-osx-arm64-${ONNXRUNTIME_VENDOR_VERSION}.tgz")
  elseif (UNIX)
    set(_onnx_default_archive "onnxruntime-linux-x64-${ONNXRUNTIME_VENDOR_VERSION}.tgz")
  else()
    message(FATAL_ERROR "No default ONNX Runtime archive is configured for this platform; set ONNXRUNTIME_VENDOR_DOWNLOAD_URL manually.")
  endif()

  set(ONNXRUNTIME_VENDOR_DOWNLOAD_URL "${_onnx_release_base_url}${_onnx_default_archive}"
      CACHE STRING "Optional URL pointing to an ONNX Runtime archive (tarball/zip). FetchContent will download and extract it."
      FORCE)
  message(STATUS "Defaulting ONNX Runtime download to ${ONNXRUNTIME_VENDOR_DOWNLOAD_URL}")
endif()

if (NOT ONNXRUNTIME_VENDOR_FORCE)
  find_package(onnxruntime QUIET)
  if (onnxruntime_FOUND)
    message(STATUS "Using system ONNX Runtime installation from ${onnxruntime_DIR}")
    return()
  endif()
endif()

if (ONNXRUNTIME_VENDOR_DOWNLOAD_URL)
  message(STATUS "Fetching ONNX Runtime from ${ONNXRUNTIME_VENDOR_DOWNLOAD_URL}")
  FetchContent_Declare(
    OnnxRuntimeVendor
    URL "${ONNXRUNTIME_VENDOR_DOWNLOAD_URL}"
    URL_HASH ""
  )

  FetchContent_GetProperties(OnnxRuntimeVendor)
  if (NOT OnnxRuntimeVendor_POPULATED)
    FetchContent_MakeAvailable(OnnxRuntimeVendor)
  endif()

  set(_onnx_candidate_dirs
    "${onnxruntimevendor_SOURCE_DIR}"
    "${onnxruntimevendor_SOURCE_DIR}/onnxruntime"
  )
  set(_onnx_fetch_root "")
  foreach (_candidate ${_onnx_candidate_dirs})
    if (EXISTS "${_candidate}/cmake/onnxruntime-config.cmake")
      set(_onnx_fetch_root "${_candidate}")
      break()
    endif()
  endforeach()

  if (_onnx_fetch_root STREQUAL "")
    message(FATAL_ERROR
      "Downloaded archive from ${ONNXRUNTIME_VENDOR_DOWNLOAD_URL} did not contain cmake/onnxruntime-config.cmake; "
      "point ONNXRUNTIME_VENDOR_DOWNLOAD_URL at a release bundle.")
  endif()

  set(ONNXRUNTIME_VENDOR_DIR "${_onnx_fetch_root}")
  message(STATUS "ONNX Runtime extracted to ${ONNXRUNTIME_VENDOR_DIR}")
endif()

if (ONNXRUNTIME_VENDOR_DIR STREQUAL "")
  message(FATAL_ERROR
    "ONNXRUNTIME_VENDOR_DIR is empty. Set it to the root of an ONNX Runtime bundle containing "
    "cmake/onnxruntime-config.cmake or provide ONNXRUNTIME_VENDOR_DOWNLOAD_URL.")
endif()

