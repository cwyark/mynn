include(FetchContent)

set(ONNX_VENDOR_VERSION "1.20.0" CACHE STRING "ONNX release version to fetch.")
set(ONNX_VENDOR_URL "https://github.com/onnx/onnx/archive/refs/tags/v${ONNX_VENDOR_VERSION}.tar.gz" CACHE STRING "Optional URL pointing to an ONNX archive (tarball/zip). FetchContent will download and extract it.")

option(ONNX_VENDOR_FORCE "Require using the vendored ONNX Runtime even if a system installation is detected." ON)

if (NOT ONNX_VENDOR_FORCE)
  find_package(onnx QUIET)
  if (ONNX_FOUND)
    message(STATUS "Using system onnx installation")
    return()
  endif()
endif()

message(STATUS "Fetching Onnx v${ONNX_VENDOR_VERSION} from ${ONNX_VENDOR_URL}")

FetchContent_Declare(
    onnx
    URL       "${ONNX_VENDOR_URL}"
    URL_HASH  ""
  )
FetchContent_MakeAvailable(onnx)

if(DEFINED Protobuf_INCLUDE_DIRS)
  foreach(onnx_target IN ITEMS onnx_object onnx_proto_object)
    if(TARGET ${onnx_target})
      target_include_directories(${onnx_target} PUBLIC
        $<BUILD_INTERFACE:${Protobuf_INCLUDE_DIRS}>
      )
    endif()
  endforeach()
endif()
