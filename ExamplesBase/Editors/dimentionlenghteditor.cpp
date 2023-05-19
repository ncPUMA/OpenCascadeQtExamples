#include "dimentionlenghteditor.h"

#include <Extrema_ExtElC.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Line.hxx>
#include <V3d_View.hxx>

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

IMPLEMENT_STANDARD_RTTIEXT(DimentionLenghtEditor, PrsDim_LengthDimension)

inline static void initAspects(DimentionLenghtEditor *dimension)
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

DimentionLenghtEditor::DimentionLenghtEditor(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane)
    : PrsDim_LengthDimension(p1, p2, plane)
{
    initAspects(this);
    vl = GetValue();
}

void DimentionLenghtEditor::update(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &plane)
{
    SetMeasuredGeometry(p1, p2, plane);
    vl = GetValue();
}

Standard_Real DimentionLenghtEditor::dimention() const
{
    return vl;
}

Standard_Boolean DimentionLenghtEditor::ProcessDragging(const Handle(AIS_InteractiveContext) &context,
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

            vl = value;
            auto primitive = Handle(InteractiveObjectEditor)::DownCast(Parent());
            if (primitive) {
                primitive->handleEditor(this);
            }
            break;
        }

        case AIS_DragAction_Stop:
            break;

        case AIS_DragAction_Abort:
            vl = GetValue();
            auto primitive = Handle(InteractiveObjectEditor)::DownCast(Parent());
            if (primitive) {
                primitive->handleEditor(this);
            }
            break;
    }
    return Standard_True;
}

}
