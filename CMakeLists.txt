cmake_minimum_required(VERSION 3.19)

project(OscCpp VERSION 1.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_DIRECTORY})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_USE_HYBRID_HPG 1)
# set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

set(SAMPLE_NAME "")
set(SAMPLE_DIR ${PROJECT_SOURCE_DIR}/${SAMPLE_NAME})
file(GLOB SAMPLE_SOURCES ${SAMPLE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SAMPLE_DIR}/main.cpp ${SOURCES} ${SAMPLE_SOURCES})
include_directories(./ext/)

include_directories("${PROJECT_SOURCE_DIR}")

# include(FindPkgConfig)
find_package(PkgConfig)
include(FetchContent)

# !!!!!! copy resources
set(RESOURCE_FILES
    ${PROJECT_SOURCE_DIR}/shaders
)
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/resources/shaders
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Copying resource files to build directory"
)

# !!!!!! use llvm clang
# find_program(CLANG_COMPILER clang)
# find_program(CLANGXX_COMPILER clang++)

# if(CLANG_COMPILER AND CLANGXX_COMPILER)
#     message(STATUS "Clang++ compiler: ${CLANGXX_COMPILER}")
#     set(CMAKE_C_COMPILER ${CLANG_COMPILER})
#     set(CMAKE_CXX_COMPILER ${CLANGXX_COMPILER})
# else()
#     message(FATAL_ERROR "Clang/Clang++ compilers not found")
# endif()

# !!!!!! RAYLIB

if(APPLE)
    message(STATUS "OS: Mac")
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_macos.tar.gz
    )
elseif(MSVC)
    message(STATUS "OS: Win64")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIRECTORY})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_DIRECTORY}"
    )

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
endif()
FetchContent_MakeAvailable(raylib)
# find_library(RAYLIB NAMES raylib REQUIRED)
# find_package(raylib REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)



# if(raylib_POPULATED)
#     message(STATUS "raylib successfully fetched and available")
#     message(STATUS ${CMAKE_BINARY_DIR}/_deps/raylib-src/lib/raylib.lib)
#     target_link_libraries(${PROJECT_NAME} PRIVATE raylib ${CMAKE_BINARY_DIR}/_deps/raylib-src/lib/raylib.lib)
#     target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/_deps/raylib-src/include)
# else()
#     message(FATAL_ERROR "raylib not found")
# endif()


