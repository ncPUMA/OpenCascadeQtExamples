#ifndef INTERACTIVEDIMENTIONLENGHT_H
#define INTERACTIVEDIMENTIONLENGHT_H

#include <PrsDim_LengthDimension.hxx>

class InteractiveDimentionLenght : public PrsDim_LengthDimension
{
    DEFINE_STANDARD_RTTIEXT(InteractiveDimentionLenght, PrsDim_LengthDimension)
public:
    InteractiveDimentionLenght(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane);
    InteractiveDimentionLenght(const TopoDS_Face &face, const TopoDS_Edge &edge);
    InteractiveDimentionLenght(const TopoDS_Face &face1, const TopoDS_Face &face2);

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
};

DEFINE_STANDARD_HANDLE(InteractiveDimentionLenght, PrsDim_LengthDimension)

#endif // INTERACTIVEDIMENTIONLENGHT_H
