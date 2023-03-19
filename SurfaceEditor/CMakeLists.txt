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

    Editors/dimentionlenghteditor.h
    Editors/dimentionlenghteditor.cpp
    Editors/interactiveobjecteditor.h
    Editors/interactiveobjecteditor.cpp
    Editors/interactiveobjecteditorcreator.h
    Editors/interactiveobjecteditorcreator.cpp
    Editors/interactiveobjectsurfacecircleofrevoleditor.h
    Editors/interactiveobjectsurfacecircleofrevoleditor.cpp
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
    ObjectModels/interactiveobjectitemmodeldelegate.h
    ObjectModels/interactiveobjectitemmodeldelegate.cpp
    ObjectModels/interactiveobjectitemmodelshape.h
    ObjectModels/interactiveobjectitemmodelshape.cpp
    ObjectModels/interactiveobjectitemmodelsurface.h
    ObjectModels/interactiveobjectitemmodelsurface.cpp
    ObjectModels/interactiveobjectitemmodelsurfacecircleofrevol.h
    ObjectModels/interactiveobjectitemmodelsurfacecircleofrevol.cpp
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

    Objects/interactiveobject.h
    Objects/interactiveobject.cpp
    Objects/interactiveshape.h
    Objects/interactiveshape.cpp
    Objects/interactivesurface.h
    Objects/interactivesurface.cpp
    Objects/interactivesurfacecircleofrevol.h
    Objects/interactivesurfacecircleofrevol.cpp
    Objects/interactivesurfacehyperofrevol.h
    Objects/interactivesurfacehyperofrevol.cpp
    Objects/interactivesurfaceparabofrevol.h
    Objects/interactivesurfaceparabofrevol.cpp
    Objects/interactivesurfaceplane.h
    Objects/interactivesurfaceplane.cpp
    Objects/interactivesurfacerevolution.h
    Objects/interactivesurfacerevolution.cpp
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
