#include "manipulatoreditor.h"

#include "interactiveobjecteditor.h"

IMPLEMENT_STANDARD_RTTIEXT(ManipulatorEditor, AIS_Manipulator)

ManipulatorEditor::ManipulatorEditor()
    : AIS_Manipulator()
{

}

gp_Trsf ManipulatorEditor::getTransform() const
{
    return transform;
}

Standard_Boolean ManipulatorEditor::ProcessDragging(const Handle(AIS_InteractiveContext)& theCtx,
                                                    const Handle(V3d_View)& theView,
                                                    const Handle(SelectMgr_EntityOwner)& theOwner,
                                                    const Graphic3d_Vec2i& theDragFrom,
                                                    const Graphic3d_Vec2i& theDragTo,
                                                    const AIS_DragAction theAction)
{
    switch (theAction) {
        case AIS_DragAction_Start:
            transform = gp_Trsf();
            if (HasActiveMode()) {
                StartTransform (theDragFrom.x(), theDragFrom.y(), theView);
                return Standard_True;
            }
            break;

        case AIS_DragAction_Update:{
            gp_Trsf trsf;
            ObjectTransformation(theDragTo.x(), theDragTo.y(), theView, trsf);
            transform = trsf;
            if ((myCurrentMode == AIS_MM_Translation      && myBehaviorOnTransform.FollowTranslation)
             || (myCurrentMode == AIS_MM_Rotation         && myBehaviorOnTransform.FollowRotation)
             || (myCurrentMode == AIS_MM_TranslationPlane && myBehaviorOnTransform.FollowDragging))
            {
              gp_Pnt aPos  = myStartPosition.Location().Transformed (transform);
              gp_Dir aVDir = myStartPosition.Direction().Transformed (transform);
              gp_Dir aXDir = myStartPosition.XDirection().Transformed (transform);
              SetPosition (gp_Ax2 (aPos, aVDir, aXDir));
            }
            if (IsAttached()) {
                auto object = Handle(InteractiveObjectEditor)::DownCast(Object());
                if (object) {
                    object->handleEditor(this);
                }
            }
            return Standard_True;
}
        case AIS_DragAction_Abort:
            StopTransform (false);
            transform = gp_Trsf();
            if (IsAttached()) {
                auto object = Handle(InteractiveObjectEditor)::DownCast(Object());
                if (object) {
                    object->handleEditor(this);
                }
            }
            return Standard_True;

        case AIS_DragAction_Stop:
            break;
    }
    return Standard_False;
}
