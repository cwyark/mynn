include(FetchContent)

set(PROTOBUF_VENDOR_VERSION "33.2" CACHE STRING "Protobuf release version to fetch.")
set(PROTOBUF_VENDOR_URL
    "https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VENDOR_VERSION}/protoc-${PROTOBUF_VENDOR_VERSION}-osx-universal_binary.zip"
    CACHE STRING
    "Optional URL pointing to a Protobuf release archive (tarball/zip). FetchContent will download and extract it.")

# NOTE: it's not easy to compile protobuf from source. So it is suggested that install
# protobuf by your system, e.g. brew, apt or others.
option(PROTOBUF_VENDOR_FORCE
  "Require using the vendored Protobuf even if a system installation is detected." OFF)

if (NOT PROTOBUF_VENDOR_FORCE)
  find_package(Protobuf REQUIRED)
  if (Protobuf_FOUND)
    message(STATUS "Use system Protobuf installation")
    return()
  endif()
endif()

FetchContent_Declare(
  ProtobufVendor
  URL "${PROTOBUF_VENDOR_URL}"
  URL_HASH ""
)
FetchContent_MakeAvailable(ProtobufVendor)

if (NOT TARGET protobuf)

endif()
