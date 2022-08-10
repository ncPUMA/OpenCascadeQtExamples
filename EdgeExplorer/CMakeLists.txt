cmake_minimum_required(VERSION 3.5)

project(EdgeExplorer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_SOURCES
    aspectwindow.cpp
    aspectwindow.h
    brepserializer.cpp
    brepserializer.h
    main.cpp
    mainwindow.cpp
    mainwindow.h

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

    normaldetector.cpp
    normaldetector.h
    viewport.cpp
    viewport.h
)

add_executable(EdgeExplorer
    ${PROJECT_SOURCES}
)

target_link_libraries(EdgeExplorer PRIVATE Qt5::Widgets)

target_link_libraries(EdgeExplorer PRIVATE
    TKernel TKMath TKService TKV3d TKOpenGl
    TKBRep TKIGES TKSTL TKVRML TKSTEP TKSTEPAttr TKSTEP209
    TKSTEPBase TKGeomBase TKGeomAlgo TKG3d TKG2d
    TKXSBase TKShHealing TKHLR TKTopAlgo TKMesh TKPrim
    TKCDF TKBool TKBO TKFillet TKOffset TKLCAF TKCAF TKVCAF
    TKBin TKXml TKRWMesh
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/EdgeExplorer/Models $<TARGET_FILE_DIR:${PROJECT_NAME}>/Models
                   )
