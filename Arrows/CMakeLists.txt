cmake_minimum_required(VERSION 3.5)

project(Arrows VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H

    )

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    viewport.h
    viewport.cpp
    interactivearrowsequence.h
    interactivearrowsequence.cpp
)

add_executable(Arrows
    ${PROJECT_SOURCES}
)

target_include_directories(
  Arrows
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(Arrows PUBLIC ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(Arrows PRIVATE ${OpenCASCADE_LIBRARIES})
