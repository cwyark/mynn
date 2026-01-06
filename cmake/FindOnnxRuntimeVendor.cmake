include(FetchContent)

set(ONNXRUNTIME_VENDOR_VERSION "1.23.2" CACHE STRING "ONNX Runtime release version to fetch.")
set(ONNXRUNTIME_VENDOR_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VENDOR_VERSION}/onnxruntime-osx-arm64-${ONNXRUNTIME_VENDOR_VERSION}.tgz" CACHE STRING
    "Optional URL pointing to an ONNX Runtime archive (tarball/zip). FetchContent will download and extract it.")

option(ONNXRUNTIME_VENDOR_FORCE
  "Require using the vendored ONNX Runtime even if a system installation is detected." ON)

if (NOT ONNXRUNTIME_VENDOR_FORCE)
  find_package(onnxruntime QUIET)
  if (ONNXRUNTIME_FOUND)
    message(STATUS "Using system onnxruntime installation")
    return()
  endif()
endif()

message(STATUS "Fetching OnnxRuntime v${ONNXRUNTIME_VENDOR_VERSION} from ${ONNXRUNTIME_VENDOR_URL}")

FetchContent_Declare(
    onnxruntime_prebuilt
    URL       "${ONNXRUNTIME_VENDOR_URL}"
    URL_HASH  ""
  )
FetchContent_MakeAvailable(onnxruntime_prebuilt)

if (NOT TARGET onnxruntime)

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

  message(STATUS "OnnxRuntime target created from vendored source")

endif()
