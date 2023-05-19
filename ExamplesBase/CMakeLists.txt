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

  Editors/dimentionlenghteditor.h
  Editors/dimentionlenghteditor.cpp
  Editors/interactiveobjectcuboideditor.h
  Editors/interactiveobjectcuboideditor.cpp
  Editors/interactiveobjectcylindereditor.h
  Editors/interactiveobjectcylindereditor.cpp
  Editors/interactiveobjecteditor.h
  Editors/interactiveobjecteditor.cpp
  Editors/interactiveobjecteditorcreator.h
  Editors/interactiveobjecteditorcreator.cpp
  Editors/interactiveobjectsurfacecircleofrevoleditor.h
  Editors/interactiveobjectsurfacecircleofrevoleditor.cpp
  Editors/interactiveobjectsurfaceellipseofrevoleditor.h
  Editors/interactiveobjectsurfaceellipseofrevoleditor.cpp
  Editors/interactiveobjectsurfacehyperofrevoleditor.h
  Editors/interactiveobjectsurfacehyperofrevoleditor.cpp
  Editors/interactiveobjectsurfaceparabofrevoleditor.h
  Editors/interactiveobjectsurfaceparabofrevoleditor.cpp
  Editors/interactiveobjectsurfaceplaneeditor.h
  Editors/interactiveobjectsurfaceplaneeditor.cpp
  Editors/manipulatoreditor.h
  Editors/manipulatoreditor.cpp
  Editors/point.h
  Editors/point.cpp

  ObjectModels/interactiveobjectitemmodel.h
  ObjectModels/interactiveobjectitemmodel.cpp
  ObjectModels/interactiveobjectitemmodelcreator.h
  ObjectModels/interactiveobjectitemmodelcreator.cpp
  ObjectModels/interactiveobjectitemmodelcuboid.h
  ObjectModels/interactiveobjectitemmodelcuboid.cpp
  ObjectModels/interactiveobjectitemmodelcylinder.h
  ObjectModels/interactiveobjectitemmodelcylinder.cpp
  ObjectModels/interactiveobjectitemmodeldelegate.h
  ObjectModels/interactiveobjectitemmodeldelegate.cpp
  ObjectModels/interactiveobjectitemmodelshape.h
  ObjectModels/interactiveobjectitemmodelshape.cpp
  ObjectModels/interactiveobjectitemmodelsurface.h
  ObjectModels/interactiveobjectitemmodelsurface.cpp
  ObjectModels/interactiveobjectitemmodelsurfacecircleofrevol.h
  ObjectModels/interactiveobjectitemmodelsurfacecircleofrevol.cpp
  ObjectModels/interactiveobjectitemmodelsurfaceellipseofrevol.h
  ObjectModels/interactiveobjectitemmodelsurfaceellipseofrevol.cpp
  ObjectModels/interactiveobjectitemmodelsurfacehyperofrevol.h
  ObjectModels/interactiveobjectitemmodelsurfacehyperofrevol.cpp
  ObjectModels/interactiveobjectitemmodelsurfaceparabofrevol.h
  ObjectModels/interactiveobjectitemmodelsurfaceparabofrevol.cpp
  ObjectModels/interactiveobjectitemmodelsurfaceplane.h
  ObjectModels/interactiveobjectitemmodelsurfaceplane.cpp
  ObjectModels/interactiveobjectitemmodelsurfacerevolution.h
  ObjectModels/interactiveobjectitemmodelsurfacerevolution.cpp
  ObjectModels/objectstreemodel.h
  ObjectModels/objectstreemodel.cpp

  Objects/interactivecuboid.h
  Objects/interactivecuboid.cpp
  Objects/interactivecylinder.h
  Objects/interactivecylinder.cpp
  Objects/interactiveobject.h
  Objects/interactiveobject.cpp
  Objects/interactiveshape.h
  Objects/interactiveshape.cpp
  Objects/interactivesurface.h
  Objects/interactivesurface.cpp
  Objects/interactivesurfacecircleofrevol.h
  Objects/interactivesurfacecircleofrevol.cpp
  Objects/interactivesurfaceellipseofrevol.h
  Objects/interactivesurfaceellipseofrevol.cpp
  Objects/interactivesurfacehyperofrevol.h
  Objects/interactivesurfacehyperofrevol.cpp
  Objects/interactivesurfaceparabofrevol.h
  Objects/interactivesurfaceparabofrevol.cpp
  Objects/interactivesurfaceplane.h
  Objects/interactivesurfaceplane.cpp
  Objects/interactivesurfacerevolution.h
  Objects/interactivesurfacerevolution.cpp
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
