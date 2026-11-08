set(PLATFORM_SHARED_DIR ${CMAKE_CURRENT_LIST_DIR})

add_definitions(-DBH_PLATFORM_INFINOS)

include_directories(${PLATFORM_SHARED_DIR})
include_directories(${PLATFORM_SHARED_DIR}/../include)

file(GLOB_RECURSE source_all ${PLATFORM_SHARED_DIR}/*.c)

set(PLATFORM_SHARED_SOURCE
    ${source_all}
    ${PLATFORM_COMMON_MATH_SOURCE}
)

# Picolibc headers for the AArch64 freestanding target.
set(PICOLIBC_ROOT "/usr/lib/picolibc/aarch64-linux-gnu")
include_directories(SYSTEM "${PICOLIBC_ROOT}/include")
