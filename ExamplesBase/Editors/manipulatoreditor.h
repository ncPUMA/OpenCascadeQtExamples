#ifndef MANIPULATOREDITOR_H
#define MANIPULATOREDITOR_H

#include <AIS_Manipulator.hxx>

namespace ExamplesBase {

class ManipulatorEditor : public AIS_Manipulator
{
    DEFINE_STANDARD_RTTIEXT(ManipulatorEditor, AIS_Manipulator)

public:
    ManipulatorEditor();

    gp_Trsf getTransform() const;

    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext)& theCtx,
                                     const Handle(V3d_View)& theView,
                                     const Handle(SelectMgr_EntityOwner)& theOwner,
                                     const Graphic3d_Vec2i& theDragFrom,
                                     const Graphic3d_Vec2i& theDragTo,
                                     const AIS_DragAction theAction) Standard_OVERRIDE;

private:
    gp_Trsf transform;
};

DEFINE_STANDARD_HANDLE(ManipulatorEditor, AIS_Manipulator)

}

#endif // MANIPULATOREDITOR_H
