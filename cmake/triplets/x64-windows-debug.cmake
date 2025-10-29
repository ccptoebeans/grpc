set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PLATFORM_TOOLSET v141)
set(VCPKG_BUILD_TYPE "release")

set(VCPKG_CMAKE_SYSTEM_VERSION "10.0.17763.0")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreadedDLL)

set(CARBON_BUILD_TYPE "Debug")

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
