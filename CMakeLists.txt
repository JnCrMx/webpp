cmake_minimum_required(VERSION 3.30.3)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webpp)

if(${PROJECT_NAME}_IS_TOP_LEVEL)
    set(CMAKE_SYSTEM_NAME WASI)
    set(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_SYSTEM_PROCESSOR wasm32)
    set(TARGET_TRIPLE wasm32-wasi)
    set(CMAKE_C_COMPILER_TARGET   ${TARGET_TRIPLE})
    set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
    set(CMAKE_LINK_DEPENDS_USE_LINKER OFF)
endif()

option(${PROJECT_NAME}_BUILD_EXAMPLES "Build examples" ${${PROJECT_NAME}_IS_TOP_LEVEL})
option(${PROJECT_NAME}_BUILD_STD_MODULE "Build std module" ${${PROJECT_NAME}_IS_TOP_LEVEL})
set(${PROJECT_NAME}_STD_MODULE_NAME stdModule CACHE STRING "Name of the std module target")

set(stdModule ${${PROJECT_NAME}_STD_MODULE_NAME})
if(${PROJECT_NAME}_BUILD_STD_MODULE)
    add_library(${stdModule} STATIC)
    target_sources(${stdModule} PUBLIC FILE_SET CXX_MODULES BASE_DIRS /usr/lib/llvm-19/share/libc++/v1/
        FILES /usr/lib/llvm-19/share/libc++/v1/std.cppm)
    target_compile_features(${stdModule} PUBLIC cxx_std_26)
    target_compile_options(${stdModule} PRIVATE -Wno-reserved-identifier)
    target_compile_options(${stdModule} PRIVATE -fno-rtti -fno-exceptions)
    target_compile_definitions(${stdModule} PRIVATE _LIBCPP_CSETJMP _LIBCPP_CSIGNAL)
endif()

add_subdirectory(src)

add_library(webpp::webpp ALIAS ${PROJECT_NAME})

if(${PROJECT_NAME}_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
