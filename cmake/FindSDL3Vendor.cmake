#
# Usage (in your CMakeLists.txt):
#   set(SDL3_VENDOR_VERSION "3.4.0" CACHE STRING "SDL3 version to fetch")
#   set(SDL3_VENDOR_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-${SDL3_VENDOR_VERSION}.tar.gz" CACHE STRING "SDL3 source tarball URL")
#   include(cmake/FindSDL3Vendor.cmake)
#   # After inclusion, the SDL3::SDL3 target should be available if SDL3 builds as such.
#

include(FetchContent)

# Allow user to override the SDL3 version from the cache or command line.
set(SDL3_VENDOR_VERSION "3.4.0" CACHE STRING "SDL3 version to fetch via FetchContent")

# Default URL can be overridden as well.
set(SDL3_VENDOR_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-${SDL3_VENDOR_VERSION}.tar.gz" CACHE STRING "SDL3 source tarball URL for SDL3")

# Optionally allow external SDL3 installation instead of vendored one.
option(SDL3_VENDOR_FORCE "Force using vendored SDL3 via FetchContent" ON)

if (NOT SDL3_VENDOR_FORCE)
  # Try to find an already installed SDL3 first.
  find_package(SDL3 QUIET)
  if (SDL3_FOUND)
    message(STATUS "Using system SDL3 installation")
    return()
  endif()
endif()

message(STATUS "Fetching SDL3 v${SDL3_VENDOR_VERSION} from ${SDL3_VENDOR_URL}")

FetchContent_Declare(
    SDL3
    URL              "${SDL3_VENDOR_URL}"
    URL_HASH         ""
)

FetchContent_GetProperties(SDL3)
if (NOT sdl3_POPULATED)
  FetchContent_MakeAvailable(SDL3)
endif ()

if (TARGET SDL3::SDL3)
  message(STATUS "SDL3::SDL3 target is available from vendored SDL3")
else ()
  message(WARNING "SDL3 vendored build completed but SDL3::SDL3 target was not found. Check SDL3's CMake configuration and adjust usage accordingly.")
endif ()
