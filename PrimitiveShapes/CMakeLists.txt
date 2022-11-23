cmake_minimum_required(VERSION 3.5)

project(PrimitiveShapes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H

    )

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp

    Primitives/interactiveprimitive.h
    Primitives/interactiveprimitive.cpp
    Primitives/interactiveprimitivebox.h
    Primitives/interactiveprimitivebox.cpp
    Primitives/interactiveprimitivecone.h
    Primitives/interactiveprimitivecone.cpp
    Primitives/interactiveprimitivecylinder.h
    Primitives/interactiveprimitivecylinder.cpp
    Primitives/interactiveprimitiveserializer.h
    Primitives/interactiveprimitiveserializer.cpp
    Primitives/interactivedimentionlenght.h
    Primitives/interactivedimentionlenght.cpp

    viewport.cpp
)

add_executable(PrimitiveShapes
    ${PROJECT_SOURCES}
)

target_include_directories(
  PrimitiveShapes
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(PrimitiveShapes PUBLIC ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(PrimitiveShapes PRIVATE
    TKernel TKMath TKService TKV3d TKOpenGl
    TKBRep TKIGES TKSTL TKVRML TKSTEP TKSTEPAttr TKSTEP209
    TKSTEPBase TKGeomBase TKGeomAlgo TKG3d TKG2d
    TKXSBase TKShHealing TKHLR TKTopAlgo TKMesh TKPrim
    TKCDF TKBool TKBO TKFillet TKOffset TKLCAF TKCAF TKVCAF
    TKBin TKXml TKRWMesh
    )
