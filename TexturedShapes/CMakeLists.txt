cmake_minimum_required(VERSION 3.5)

project(TexturedShapes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp

    viewport.cpp
)

add_executable(TexturedShapes
    ${PROJECT_SOURCES}
)

target_include_directories(
  TexturedShapes
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(TexturedShapes PUBLIC ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(TexturedShapes PRIVATE
    TKernel TKMath TKService TKV3d TKOpenGl
    TKBRep TKIGES TKSTL TKVRML TKSTEP TKSTEPAttr TKSTEP209
    TKSTEPBase TKGeomBase TKGeomAlgo TKG3d TKG2d
    TKXSBase TKShHealing TKHLR TKTopAlgo TKMesh TKPrim
    TKCDF TKBool TKBO TKFillet TKOffset TKLCAF TKCAF TKVCAF
    TKBin TKXml TKRWMesh
    )
