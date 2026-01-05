#==========================================================================
# CMake helper that locates a LibTorch installation and exposes it like the
# rest of the vendor helpers in this project.
#
# Usage:
#   set(TORCH_VENDOR_DIR "/path/to/libtorch" CACHE PATH "Root of LibTorch distribution.")
#   set(TORCH_VENDOR_DOWNLOAD_URL "<libtorch archive URL>" CACHE STRING
#       "Optional URL to fetch a LibTorch archive via FetchContent.")
#   option(TORCH_VENDOR_FORCE "Force using the vendored LibTorch even if a system Torch is found." OFF)
#   include(cmake/FindTorchVendor.cmake)
#   # After inclusion, the usual Torch targets/variables (e.g. Torch::Torch,
#   # TORCH_LIBRARIES, Torch_INCLUDE_DIRS) should be available.
#==========================================================================

include(FetchContent)

set(TORCH_VENDOR_DOWNLOAD_URL "" CACHE STRING
    "Optional LibTorch archive URL (zip/tarball). If set, FetchContent_Populate will download and extract it.")
if (APPLE AND TORCH_VENDOR_DOWNLOAD_URL STREQUAL "")
  set(TORCH_VENDOR_DOWNLOAD_URL
      "https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.9.1.zip"
      CACHE STRING
      "Optional LibTorch archive URL (zip/tarball). This defaults to the macOS arm64 CPU bundle."
      FORCE)
endif()
set(TORCH_VENDOR_DIR "" CACHE PATH
    "Root directory of a LibTorch extraction (must contain share/cmake/Torch)")
option(TORCH_VENDOR_FORCE "Require using the vendored LibTorch even if a system Torch installation is detected." OFF)

if (NOT TORCH_VENDOR_FORCE)
  find_package(Torch QUIET)
  if (Torch_FOUND)
    message(STATUS "Using system Torch installation from ${Torch_DIR}")
    return()
  endif()
endif()

if (TORCH_VENDOR_DOWNLOAD_URL)
  message(STATUS "Fetching LibTorch from ${TORCH_VENDOR_DOWNLOAD_URL}")
  FetchContent_Declare(
    TorchVendor
    URL "${TORCH_VENDOR_DOWNLOAD_URL}"
    URL_HASH ""
  )
  FetchContent_GetProperties(TorchVendor)
  if (NOT TorchVendor_POPULATED)
    FetchContent_MakeAvailable(TorchVendor)
  endif()

  set(_torch_candidate_dirs
    "${torchvendor_SOURCE_DIR}"
    "${torchvendor_SOURCE_DIR}/libtorch"
  )
  set(_torch_fetch_root "")
  foreach (_candidate ${_torch_candidate_dirs})
    if (EXISTS "${_candidate}/share/cmake/Torch/TorchConfig.cmake")
      set(_torch_fetch_root "${_candidate}")
      break()
    endif()
  endforeach()

  if (_torch_fetch_root STREQUAL "")
    message(FATAL_ERROR
      "Downloaded archive from ${TORCH_VENDOR_DOWNLOAD_URL} did not expose share/cmake/Torch/TorchConfig.cmake; "
      "please point TORCH_VENDOR_DOWNLOAD_URL to a LibTorch release bundle.")
  endif()

  set(TORCH_VENDOR_DIR "${_torch_fetch_root}")
  message(STATUS "LibTorch extracted to ${TORCH_VENDOR_DIR}")
endif()

if (TORCH_VENDOR_DIR STREQUAL "")
  message(FATAL_ERROR
    "TORCH_VENDOR_DIR is empty. Set it to the root of a LibTorch distribution before including FindTorchVendor.cmake or supply TORCH_VENDOR_DOWNLOAD_URL.")
endif()

set(_torch_config_dir "${TORCH_VENDOR_DIR}/share/cmake/Torch")
if (NOT EXISTS "${_torch_config_dir}/TorchConfig.cmake")
  message(FATAL_ERROR
    "The specified TORCH_VENDOR_DIR (${TORCH_VENDOR_DIR}) does not contain a TorchConfig.cmake file. "
    "Verify that the directory points to the root of a LibTorch bundle.")
endif()

find_package(Torch REQUIRED PATHS "${_torch_config_dir}")
message(STATUS "Using vendred LibTorch from ${TORCH_VENDOR_DIR}")
