/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivedimentionlenght.h"

#include <Extrema_ExtElC.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Line.hxx>
#include <V3d_View.hxx>

#include "interactiveprimitive.h"

IMPLEMENT_STANDARD_RTTIEXT(InteractiveDimentionLenght, PrsDim_LengthDimension)

inline static void initAspects(InteractiveDimentionLenght *dimension)
{
    auto dAspect = dimension->DimensionAspect();
    if (!dAspect) {
        dAspect = new Prs3d_DimensionAspect;
    }

    auto lAspect = dAspect->LineAspect();
    if (!lAspect) {
        lAspect = new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 2.);
    } else {
        lAspect->SetWidth(2.);
    }
    dAspect->SetLineAspect(lAspect);
    dimension->SetDimensionAspect(dAspect);
}

InteractiveDimentionLenght::InteractiveDimentionLenght(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane)
    : PrsDim_LengthDimension(p1, p2, plane)
{
    initAspects(this);
}

InteractiveDimentionLenght::InteractiveDimentionLenght(const TopoDS_Face &face, const TopoDS_Edge &edge)
    : PrsDim_LengthDimension(face, edge)
{
    initAspects(this);
}

InteractiveDimentionLenght::InteractiveDimentionLenght(const TopoDS_Face &face1, const TopoDS_Face &face2)
    : PrsDim_LengthDimension(face1, face2)
{
    initAspects(this);
}

Standard_Boolean InteractiveDimentionLenght::ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                                             const Handle(V3d_View) &view,
                                                             const Handle(SelectMgr_EntityOwner) &owner,
                                                             const Graphic3d_Vec2i &from,
                                                             const Graphic3d_Vec2i &to,
                                                             const AIS_DragAction action) {
    switch (action) {
        case AIS_DragAction_Start: {
            mStartDragValue = GetValue();
            gp_Trsf combined;
            if (CombinedParentTransformation()) {
                combined = CombinedParentTransformation()->Trsf();
            }
            if (mySelectionGeom.DimensionLine.IsEmpty()) {
                return Standard_False;
            }
            auto line = mySelectionGeom.DimensionLine.First();
            if (mySelectionGeom.DimensionLine.Size() == 3) {
                line = mySelectionGeom.DimensionLine.Value(2);
            }
            if (line->Size() != 2) {
                return Standard_False;
            }
            mStartDragP1 = line->First().Transformed(combined);
            mStartDragP2 = line->Last().Transformed(combined);
            break;
        }

        case AIS_DragAction_Update: {
            // Get 3d points with projection vectors
            Graphic3d_Vec3d fromPnt, fromProj;
            view->ConvertWithProj(from.x(), from.y(),
                                  fromPnt.x(), fromPnt.y(), fromPnt.z(),
                                  fromProj.x(), fromProj.y(), fromProj.z());
            const gp_Lin fromLine(gp_Pnt(fromPnt.x(), fromPnt.y(), fromPnt.z()),
                                  gp_Dir(fromProj.x(), fromProj.y(), fromProj.z()));
            Graphic3d_Vec3d toPnt, toProj;
            view->ConvertWithProj(to.x(), to.y(),
                                  toPnt.x(), toPnt.y(), toPnt.z(),
                                  toProj.x(), toProj.y(), toProj.z());
            const gp_Lin toLine(gp_Pnt(toPnt.x(), toPnt.y(), toPnt.z()),
                                gp_Dir(toProj.x(), toProj.y(), toProj.z()));

            const gp_Lin arrowLine(mStartDragP1, gp_Vec(mStartDragP1, mStartDragP2));
            Extrema_ExtElC extremaFrom(fromLine, arrowLine, Precision::Angular());
            Extrema_ExtElC extremaTo(toLine, arrowLine, Precision::Angular());
            if (!extremaFrom.IsDone() || extremaFrom.IsParallel() || extremaFrom.NbExt() != 1
                    || !extremaTo.IsDone() || extremaTo.IsParallel() || extremaTo.NbExt() != 1) {
              // translation cannot be done co-directed with camera
              return Standard_False;
            }

            Extrema_POnCurv exPnt[4];
            extremaFrom.Points(1, exPnt[0], exPnt[1]);
            extremaTo.Points(1, exPnt[2], exPnt[3]);
            const gp_Pnt fromPos = exPnt[1].Value();
            const gp_Pnt toPos = exPnt[3].Value();
            Standard_Real value = mStartDragP1.Distance(toPos);
            if (value < Precision::Confusion()) {
                return Standard_False;
            }

            auto primitive = Handle(InteractivePrimitive)::DownCast(Parent());
            if (primitive) {
                primitive->handleDimentionLenght(this, value);
            }
            break;
        }

        case AIS_DragAction_Stop:
            break;

        case AIS_DragAction_Abort:
            auto primitive = Handle(InteractivePrimitive)::DownCast(Parent());
            if (primitive) {
                primitive->handleDimentionLenght(this, mStartDragValue);
            }
            break;
    }
    return Standard_True;
}
