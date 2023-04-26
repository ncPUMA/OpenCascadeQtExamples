cmake_minimum_required(VERSION 3.5)

project(SceneLoading VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H

    )

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    viewport.h
    viewport.cpp
    xcafsteploader.h
    xcafsteploader.cpp
)

add_executable(SceneLoading
    ${PROJECT_SOURCES}
)

target_include_directories(
  SceneLoading
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(SceneLoading PUBLIC ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(SceneLoading PRIVATE ${OpenCASCADE_LIBRARIES})
