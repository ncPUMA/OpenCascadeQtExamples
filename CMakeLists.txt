cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(OpenCascadeQtExamples)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (QT_MIN_VERSION "5.15.0")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CONFIG config.cmake CACHE STRING "User configuration")
if(EXISTS ${CMAKE_SOURCE_DIR}/${CONFIG})
    message(STATUS "Using config: ${CONFIG}")
    set(CONFIG_DIR ${CMAKE_SOURCE_DIR})
    include(${CONFIG})
endif()

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)

find_package(OpenCASCADE REQUIRED PATHS ${OpenCASCADE_DIR} NO_DEFAULT_PATH)
include_directories(${OpenCASCADE_INCLUDE_DIR})

add_subdirectory(ExamplesBase)
add_subdirectory(EdgeExplorer)
add_subdirectory(PrimitiveShapes)
add_subdirectory(ModelLoader)
add_subdirectory(SurfaceEditor)
add_subdirectory(SurfaceColor)
add_subdirectory(SceneLoading)
add_subdirectory(Arrows)
add_subdirectory(BooleanOps)
add_subdirectory(ShapeToMesh)
add_subdirectory(TexturedShapes)

file(COPY Models/ DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Models")
file(COPY Textures/ DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Textures")
