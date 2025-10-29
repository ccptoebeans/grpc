set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE "release")
set(VCPKG_OSX_DEPLOYMENT_TARGET 10.14)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

set(CARBON_BUILD_TYPE "Internal")

if (PORT MATCHES "zlib")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "protobuf")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "curl")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "openssl")
    set(CARBON_x86_64_TRIPLET "x64-osx-release")
    set(CARBON_arm64_TRIPLET "arm64-osx-release")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "abseil")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "c-ares")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "re2")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()

if (PORT MATCHES "grpc")
    set(VCPKG_LIBRARY_LINKAGE static)
endif ()
