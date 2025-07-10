cmake_minimum_required(VERSION 3.18)
project(ndk_binder_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ANDROID_MIN_SDK_VERSION 24)

include_directories(${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${CMAKE_ANDROID_NDK}/sources/cxx-stl/llvm-libc++/include)
include_directories(${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include)

# Add source files
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link against required NDK libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    android
    log
)

