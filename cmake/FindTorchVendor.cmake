include(FetchContent)

set(TORCH_VENDOR_VERSION "2.9.1" CACHE STRING "Libtorch release version to fetch.")
set(TORCH_VENDOR_URL "https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-${TORCH_VENDOR_VERSION}.zip" CACHE STRING
    "Optional LibTorch archive URL (zip/tarball). If set, FetchContent_Populate will download and extract it.")

  option(TORCH_VENDOR_FORCE "Require using the vendored LibTorch even if a system Torch installation is detected." ON)

if (NOT TORCH_VENDOR_FORCE)
  find_package(Torch QUIET)
  if (Torch_FOUND)
    message(STATUS "Using system Torch installation")
    return()
  endif()
endif()

message(STATUS "Fetching Libtorch v${TORCH_VENDOR_VERSION} from ${TORCH_VENDOR_URL}")

FetchContent_Declare(
    libtorch_prebuilt
    URL       "${TORCH_VENDOR_URL}"
    URL_HASH  ""
  )
FetchContent_MakeAvailable(libtorch_prebuilt)

if (NOT TARGET torch)
  #list(APPEND CMAKE_MODULE_PATH "${libtorch_prebuilt_SOURCE_DIR}/share/cmake")
  find_package(Torch REQUIRED PATHS "${libtorch_prebuilt_SOURCE_DIR}/share/cmake")
endif()
