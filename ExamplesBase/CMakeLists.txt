cmake_minimum_required(VERSION 3.14)

project(ExamplesBase LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_library(ExamplesBase SHARED
  ExamplesBase_global.h

  InteractiveObjects/interactivecurve.cpp
  InteractiveObjects/interactivecurve.h
  InteractiveObjects/interactivenormal.cpp
  InteractiveObjects/interactivenormal.h
  InteractiveObjects/interactivefacenormal.cpp
  InteractiveObjects/interactivefacenormal.h
  InteractiveObjects/interactivefacenormalserializer.cpp
  InteractiveObjects/interactivefacenormalserializer.h

  ModelLoader/abstractmodelloader.h
  ModelLoader/breploader.cpp
  ModelLoader/breploader.h
  ModelLoader/igesloader.cpp
  ModelLoader/igesloader.h
  ModelLoader/modelloaderfactorymethod.cpp
  ModelLoader/modelloaderfactorymethod.h
  ModelLoader/objloader.cpp
  ModelLoader/objloader.h
  ModelLoader/steploader.cpp
  ModelLoader/steploader.h
  ModelLoader/stlloader.cpp
  ModelLoader/stlloader.h

  utility.cpp
  utility.h

  Viewport/aspectwindow.cpp
  Viewport/aspectwindow.h
  Viewport/viewport.cpp
  Viewport/viewport.h
)

target_link_libraries(ExamplesBase PRIVATE Qt5::Widgets)

target_link_libraries(ExamplesBase PRIVATE
    TKernel TKMath TKService TKV3d TKOpenGl
    TKBRep TKIGES TKSTL TKVRML TKSTEP TKSTEPAttr TKSTEP209
    TKSTEPBase TKGeomBase TKGeomAlgo TKG3d TKG2d
    TKXSBase TKShHealing TKHLR TKTopAlgo TKMesh TKPrim
    TKCDF TKBool TKBO TKFillet TKOffset TKLCAF TKCAF TKVCAF
    TKBin TKXml TKRWMesh
    )

target_compile_definitions(ExamplesBase PRIVATE EXAMPLESBASE_LIBRARY)
