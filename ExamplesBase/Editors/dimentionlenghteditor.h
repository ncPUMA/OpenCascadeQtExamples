#ifndef DIMENTIONLENGHTEDITOR_H
#define DIMENTIONLENGHTEDITOR_H

#include <PrsDim_LengthDimension.hxx>

namespace ExamplesBase {

class DimentionLenghtEditor : public PrsDim_LengthDimension
{
    DEFINE_STANDARD_RTTIEXT(DimentionLenghtEditor, PrsDim_LengthDimension)
public:
    DimentionLenghtEditor(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane);

    void update(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane);

    Standard_Real dimention() const;

protected:
    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                     const Handle(V3d_View) &view,
                                     const Handle(SelectMgr_EntityOwner) &owner,
                                     const Graphic3d_Vec2i &from,
                                     const Graphic3d_Vec2i &to,
                                     const AIS_DragAction action) Standard_OVERRIDE;

private:
    Standard_Real mStartDragValue = 0.;
    gp_Pnt mStartDragP1, mStartDragP2;
    Standard_Real vl;
};

DEFINE_STANDARD_HANDLE(DimentionLenghtEditor, PrsDim_LengthDimension)

}

#endif // DIMENTIONLENGHTEDITOR_H
