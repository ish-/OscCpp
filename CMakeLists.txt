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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIRECTORY})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

set(SAMPLE_NAME "")
set(SAMPLE_DIR ${PROJECT_SOURCE_DIR}/${SAMPLE_NAME})
file(GLOB SAMPLE_SOURCES ${SAMPLE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SAMPLE_DIR}/main.cpp ${SOURCES} ${SAMPLE_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_DIRECTORY}"
)
include_directories(./ext/)

include_directories("${PROJECT_SOURCE_DIR}")

# !!!!!! copy resources
set(RESOURCE_FILES
    ${PROJECT_SOURCE_DIR}/shaders
)
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/resources/shaders
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/resources/config.json
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Copying resource files to build directory"
)

# !!!!!! LIBS
include(FetchContent)

# !!!!!! RAYLIB
set(GLFW_USE_HYBRID_HPG 1)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# !!!!!! MinimalCppSocket
set(BUILD_MinimalCppSocket_SAMPLES OFF CACHE BOOL "" FORCE) # you don't want the samples in this case
FetchContent_Declare(
    min_sock
    GIT_REPOSITORY https://github.com/andreacasalino/Minimal-Socket
    GIT_TAG        v.3.1
)
FetchContent_MakeAvailable(min_sock)
target_link_libraries(${PROJECT_NAME} PRIVATE
    MinimalSocket
)

# !!!!!! JSON
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)