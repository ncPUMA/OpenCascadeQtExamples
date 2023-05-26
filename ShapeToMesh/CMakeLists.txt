cmake_minimum_required(VERSION 3.5)

project(ShapeToMesh VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H

    )

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    viewport.h
    viewport.cpp
)

add_executable(ShapeToMesh
    ${PROJECT_SOURCES}
)

target_include_directories(
  ShapeToMesh
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(ShapeToMesh
    PUBLIC
    ExamplesBase
    PRIVATE
    Qt5::Widgets
    ${OpenCASCADE_LIBRARIES}
)
