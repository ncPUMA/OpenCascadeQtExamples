/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_INTERACTIVEFACENORMAL_H
#define EB_INTERACTIVEFACENORMAL_H

#include "../ExamplesBase_global.h"

#include <AIS_InteractiveObject.hxx>

class TopoDS_Face;

namespace ExamplesBase {

class InteractiveFaceNormalPrivate;

//! Create a selectable and movable normal presentation
class EXAMPLESBASE_EXPORT InteractiveFaceNormal : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveFaceNormal, AIS_InteractiveObject)
public:
    //! constract object
    //! @param face - face
    //! @param pntOnFace - point on face
    InteractiveFaceNormal(const TopoDS_Face &face, const gp_Pnt &pntOnFace);
    InteractiveFaceNormal(const TopoDS_Face &face, const gp_Pnt2d &uv, const gp_Quaternion &rotation);
    ~InteractiveFaceNormal();

    //! add text label to corner point
    void setLabel(const TCollection_AsciiString &txt);
    TCollection_AsciiString getLabel() const;
    void setUvLineWidth(Standard_Real width);

    TopoDS_Face face() const;

    gp_Pnt2d get2dPnt() const;
    gp_Pnt getPnt() const;
    bool isPicked(const Handle(SelectMgr_EntityOwner) &entity) const;
    gp_Quaternion getRotation() const;

protected:
    void HilightSelected(const Handle(PrsMgr_PresentationManager) &thePM,
                         const SelectMgr_SequenceOfOwner &) Standard_OVERRIDE;
    void ClearSelected() Standard_OVERRIDE;
    void HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
                               const Handle(Prs3d_Drawer) &theStyle,
                               const Handle(SelectMgr_EntityOwner) &theOwner) Standard_OVERRIDE;

    void ComputeSelection(const Handle(SelectMgr_Selection) &selection,
                          const Standard_Integer mode) Standard_OVERRIDE;

    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                     const Handle(V3d_View) &view,
                                     const Handle(SelectMgr_EntityOwner) &owner,
                                     const Graphic3d_Vec2i &from,
                                     const Graphic3d_Vec2i &to,
                                     const AIS_DragAction action) Standard_OVERRIDE;

    void Compute(const Handle(PrsMgr_PresentationManager) &,
                 const Handle(Prs3d_Presentation) &presentation,
                 const Standard_Integer theMode) Standard_OVERRIDE;

private:
    InteractiveFaceNormalPrivate *const d;
};

DEFINE_STANDARD_HANDLE(InteractiveFaceNormal, AIS_InteractiveObject)

}

#endif // EB_INTERACTIVEFACENORMAL_H
