cmake_minimum_required(VERSION 3.5)

project(SurfaceEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_H

    )

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    viewport.h
    viewport.cpp

    Surfaces/interactiveobject.h
    Surfaces/interactiveobject.cpp
    Surfaces/interactiveobjectitemmodel.h
    Surfaces/interactiveobjectitemmodel.cpp
    Surfaces/interactiveobjectitemmodelcreator.h
    Surfaces/interactiveobjectitemmodelcreator.cpp
    Surfaces/interactiveobjectitemmodeldelegate.h
    Surfaces/interactiveobjectitemmodeldelegate.cpp
    Surfaces/interactiveobjectitemmodelsurface.h
    Surfaces/interactiveobjectitemmodelsurface.cpp
    Surfaces/interactiveobjectitemmodelsurfaceplane.h
    Surfaces/interactiveobjectitemmodelsurfaceplane.cpp
    Surfaces/interactivesurface.h
    Surfaces/interactivesurface.cpp
    Surfaces/interactivesurfaceplane.h
    Surfaces/interactivesurfaceplane.cpp
)

add_executable(SurfaceEditor
    ${PROJECT_SOURCES}
)

target_include_directories(
  SurfaceEditor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

target_link_libraries(SurfaceEditor PUBLIC ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(SurfaceEditor PRIVATE
    TKernel TKMath TKService TKV3d TKOpenGl
    TKBRep TKIGES TKSTL TKVRML TKSTEP TKSTEPAttr TKSTEP209
    TKSTEPBase TKGeomBase TKGeomAlgo TKG3d TKG2d
    TKXSBase TKShHealing TKHLR TKTopAlgo TKMesh TKPrim
    TKCDF TKBool TKBO TKFillet TKOffset TKLCAF TKCAF TKVCAF
    TKBin TKXml TKRWMesh
    )
