#include "viewport.h"

#include <QDebug>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Tool.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_CartesianPoint.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <Standard_Version.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#if OCC_VERSION_HEX > 0x070500
#include <BRepAdaptor_Surface.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#else
#include <BRepAdaptor_HSurface.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#endif

#include <ExamplesBase/InteractiveObjects/interactivecurve.h>
#include <ExamplesBase/utility.h>

#include "Primitives/interactiveprimitivebox.h"
#include "Primitives/interactiveprimitivecone.h"
#include "Primitives/interactiveprimitivecylinder.h"
#include "Primitives/interactiveprimitiveserializer.h"

namespace opencascade {
inline uint qHash(const Handle(AIS_InteractiveObject) &obj, uint seed = 0) {
    return qHash(obj.get(), seed);
}
}

class ViewportPrivate
{
    friend class Viewport;

    void addPrimitive(const Handle(InteractivePrimitive) &primitive) {
        primitive->SetZLayer(q_ptr->depthOffLayer());
        mModel->AddChild(primitive);
        mPrimitives << primitive;
        q_ptr->context()->Display(primitive, Standard_False);
    }

    void removePrimitive(const Handle(InteractivePrimitive) &primitive) {
        auto it = std::find(mPrimitives.begin(), mPrimitives.end(), primitive);
        if (it != mPrimitives.cend()) {
            auto ctx = q_ptr->context();
            ctx->Remove(*it, Standard_False);
            mPrimitives.erase(it);
            removeTestPoint();
        }
    }

    void removeTestPoint() {
        auto ctx = q_ptr->context();
        ctx->Remove(mTestPoint, Standard_False);
        ctx->Remove(mTestNormal, Standard_False);
        ctx->Remove(mTestAngle, Standard_False);
        mTestPoint = nullptr;
        mTestNormal = nullptr;
        mTestAngle = nullptr;
    }

    Viewport *q_ptr;
    Handle(AIS_Shape) mModel;

    QVector <Handle(InteractivePrimitive)> mPrimitives;
    std::string mLastSerialized;

    Handle(InteractivePrimitive) mCutPrimitive;
    TopoDS_Face mStartCutFace;
    Handle(AIS_Point) mStartCut, mEndCut;
    Handle(AIS_Shape) mCutLine;

    Handle(AIS_Point) mTestPoint;
    Handle(AIS_Line) mTestNormal;
    Handle(AIS_Line) mTestAngle;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;
    auto ctx = context();
    Handle(AIS_Trihedron) trihedron;
    auto modelFileName = QStringLiteral("../Models/tube_with_cuts.step");
    if (loadModel(modelFileName, d_ptr->mModel, trihedron)) {
        gp_Trsf transform;
        transform.SetTranslationPart(gp_Vec(10, 20, 30));
        gp_Quaternion quat;
        quat.SetEulerAngles(gp_Extrinsic_XYZ, 10., 10., 10.);
        transform.SetRotationPart(quat);
        ctx->SetLocation(d_ptr->mModel, transform);
        ctx->SetLocation(trihedron, transform);

        ctx->SetSelectionModeActive(d_ptr->mModel,
                                    AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                    Standard_False);
    } else {
        qFatal("Can't load model %s", modelFileName.toLatin1().constData());
    }
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::selectionChanged()
{
    QSet <Handle(InteractivePrimitive)> selectedShapes;
    auto ctx = context();
    ctx->InitSelected();
    bool ret = false;
    while (ctx->MoreSelected())
    {
        auto primitive = Handle(InteractivePrimitive)::DownCast(ctx->SelectedInteractive());
        if (primitive) {
            ret = true;
            primitive->setManipulatorVisible(Standard_True);
            primitive->setAdvancedManipulatorsVisible(Standard_True);
        }
        selectedShapes << primitive;
        ctx->NextSelected ();
    }
    for (const auto &p : qAsConst(d_ptr->mPrimitives)) {
        if (!selectedShapes.contains(p)) {
            ret = true;
            p->setManipulatorVisible(Standard_False);
            p->setAdvancedManipulatorsVisible(Standard_False);
        }
    }
    return ret;
}

bool Viewport::mousePressed(QMouseEvent *event)
{
    if (!d_ptr->mStartCut) {
        return false;
    }

    if (d_ptr->mEndCut) {
        auto curve = new ExamplesBase::InteractiveCurve(d_ptr->mStartCutFace,
                                                        d_ptr->mStartCut->Component()->Pnt(),
                                                        d_ptr->mEndCut->Component()->Pnt());
        curve->SetZLayer(depthOffLayer());
        d_ptr->mCutPrimitive->AddChild(curve);
        context()->Deactivate(d_ptr->mCutPrimitive, AIS_Shape::SelectionMode(TopAbs_SHAPE));
        context()->Display(curve, Standard_False);
        context()->SetSelectionModeActive(curve,
                                          AIS_Shape::SelectionMode(TopAbs_EDGE),
                                          Standard_True,
                                          AIS_SelectionModesConcurrency_Single);
    }

    context()->Remove(d_ptr->mStartCut, Standard_False);
    d_ptr->mCutPrimitive->RemoveChild(d_ptr->mStartCut);
    if (d_ptr->mEndCut) {
        context()->Remove(d_ptr->mEndCut, Standard_False);
        d_ptr->mCutPrimitive->RemoveChild(d_ptr->mEndCut);
    }
    if (d_ptr->mCutLine) {
        context()->Remove(d_ptr->mCutLine, Standard_False);
        d_ptr->mCutPrimitive->RemoveChild(d_ptr->mCutLine);
    }
    d_ptr->mCutPrimitive = nullptr;
    d_ptr->mStartCutFace = TopoDS_Face();
    d_ptr->mStartCut = nullptr;
    d_ptr->mEndCut = nullptr;
    d_ptr->mCutLine = nullptr;
    return true;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    bool ret = false;
    if (event->button() == Qt::RightButton) {
        auto ctx = context();
        QMenu menu;
        if (ctx->MainSelector()->NbPicked() == 0) {
            menu.addAction(tr("Make Box"), this, [this](){
                d_ptr->addPrimitive(new InteractivePrimitiveBox);
            });
            menu.addAction(tr("Make Cone"), this, [this](){
                d_ptr->addPrimitive(new InteractivePrimitiveCone);
            });
            menu.addAction(tr("Make Cylinder"), this, [this](){
                d_ptr->addPrimitive(new InteractivePrimitiveCylinder);
            });
            if (!d_ptr->mLastSerialized.empty()) {
                menu.addSeparator();
                menu.addAction(tr("Load last saved"), this, [this](){
                    auto primitive = InteractivePrimitiveSerializer::deserialize(d_ptr->mLastSerialized);
                    if (!primitive) {
                        qDebug() << tr("Deserialize error");
                        return;
                    }

                    primitive->SetZLayer(depthOffLayer());
                    d_ptr->mModel->AddChild(primitive);
                    d_ptr->mPrimitives << primitive;
                    context()->Display(primitive, Standard_False);
                    d_ptr->mLastSerialized.clear();
                });
            }
        } else {
            auto point = ctx->MainSelector()->PickedPoint(1);
            ctx->InitDetected();
            QSet <Handle(AIS_InteractiveObject)> finded;
            while (ctx->MoreDetected()) {
                auto detected = ctx->DetectedInteractive();
                if (finded.contains(detected)) {
                    ctx->NextDetected();
                    continue;
                }
                finded << detected;
                // primitives menu
                auto primitive = Handle(InteractivePrimitive)::DownCast(detected);
                if (primitive) {
                    menu.addAction(tr("Add Cut Line"), this, [this, primitive, point, ctx](){
                        d_ptr->mCutPrimitive = primitive;
                        auto localPnt = point.Transformed(primitive->Transformation().Inverted());
                        d_ptr->mStartCutFace = ExamplesBase::findFaceByPoint(primitive->Shape(), localPnt);
                        if (!d_ptr->mStartCutFace.IsNull()) {
                            d_ptr->mStartCut = new AIS_Point(new Geom_CartesianPoint(localPnt));
                            d_ptr->mStartCut->SetZLayer(depthOffLayer());
                            primitive->AddChild(d_ptr->mStartCut);
                            ctx->Display(d_ptr->mStartCut, Standard_False);
                            ctx->Deactivate(d_ptr->mStartCut);
                        }
                    });
                    menu.addSeparator();
                    menu.addAction(tr("Save"), this, [this, primitive](){
                        d_ptr->mLastSerialized = InteractivePrimitiveSerializer::serialize(primitive);
                    });
                    menu.addSeparator();
                    menu.addAction(tr("Remove"), this, [this, primitive](){
                        d_ptr->removePrimitive(primitive);
                    });
                    ctx->NextDetected();
                    continue;
                }
                // cut line menu
                auto cutLine = Handle(ExamplesBase::InteractiveCurve)::DownCast(detected);
                if (cutLine) {
                    auto localPnt = point.Transformed(cutLine->Parent()->Transformation().Inverted());
                    auto entity = ctx->MainSelector()->OnePicked();
                    size_t curveIndex = 0u;
                    if (cutLine->isCurvePicked(entity, curveIndex)) {
                        menu.addAction(tr("Add point"), this, [cutLine, curveIndex, localPnt](){
                            cutLine->addCurve(curveIndex, localPnt);
                        });
                        menu.addAction(tr("Create arc of circle"), this, [cutLine, curveIndex, localPnt](){
                            cutLine->addArcOfCircle(curveIndex, localPnt);
                        });
                        menu.addSeparator();
                        menu.addAction(tr("Create point by \"U\" value"), this, [this, cutLine, curveIndex, localPnt](){
                            gp_Pnt projection;
                            gp_Quaternion rotation;
                            Standard_Real U = 0.;
                            cutLine->getUParameter(curveIndex, localPnt, projection, U);
                            qDebug() << "Get U" << U << "\n"
                                     << "Local" << localPnt.X() << localPnt.Y() << localPnt.Z() << "\n"
                                     << "Proj" << projection.X() << projection.Y() << projection.Z() << "\n";
                            cutLine->getPointOnCurve(curveIndex, U, projection, rotation);

                            auto ctx = context();
                            auto zLayer = depthOffLayer();
                            gp_Trsf trsf;
                            trsf.SetRotationPart(rotation);

                            d_ptr->removeTestPoint();
                            d_ptr->mTestPoint = new AIS_Point(new Geom_CartesianPoint(projection));
                            d_ptr->mTestPoint->SetZLayer(zLayer);
                            cutLine->AddChild(d_ptr->mTestPoint);
                            ctx->Display(d_ptr->mTestPoint, Standard_False);
                            ctx->Deactivate(d_ptr->mTestPoint);

                            gp_Dir normalDir(0., 0., 1.);
                            normalDir.Transform(trsf);
                            d_ptr->mTestNormal = new AIS_Line(new Geom_CartesianPoint(projection),
                                                              new Geom_CartesianPoint(projection.Translated(normalDir.XYZ() * 5.)));
                            d_ptr->mTestNormal->SetZLayer(zLayer);
                            cutLine->AddChild(d_ptr->mTestNormal);
                            ctx->Display(d_ptr->mTestNormal, Standard_False);
                            ctx->Deactivate(d_ptr->mTestNormal);

                            gp_Dir angleDir(1., 0., 0.);
                            angleDir.Transform(trsf);
                            d_ptr->mTestAngle = new AIS_Line(new Geom_CartesianPoint(projection),
                                                             new Geom_CartesianPoint(projection.Translated(angleDir.XYZ() * 5.)));
                            d_ptr->mTestAngle->SetColor(Quantity_NOC_BLUE);
                            d_ptr->mTestAngle->SetZLayer(zLayer);
                            cutLine->AddChild(d_ptr->mTestAngle);
                            ctx->Display(d_ptr->mTestAngle, Standard_False);
                            ctx->Deactivate(d_ptr->mTestAngle);
                        });
                        menu.addAction(tr("Calc curve lenght"), this, [this, cutLine, curveIndex, localPnt](){
                            Standard_Real l = 0.;
                            if (cutLine->getLength(curveIndex, l)) {
                                qDebug() << "Curve index" << curveIndex << "lenght" << l;
                            }
                        });
                    }
                    size_t pointIndex = 0u;
                    if (cutLine->curvesCount() > 1 && cutLine->isPointPicked(entity, curveIndex, pointIndex)) {
                        if (!menu.isEmpty()) {
                            menu.addSeparator();
                        }
                        menu.addAction(tr("Remove point"), this, [cutLine, curveIndex](){
                            cutLine->removeCurve(curveIndex);
                        });
                    }
                    if (!menu.isEmpty()) {
                        menu.addSeparator();
                    }
                    menu.addAction(tr("Remove cut line"), this, [this, cutLine](){
                        context()->Remove(cutLine, Standard_True);
                        auto primitive = Handle(InteractivePrimitive)::DownCast(cutLine->Parent());
                        if (primitive) {
                            primitive->RemoveChild(cutLine);
                            for (const auto &ch : primitive->Children()) {
                                if (Handle(ExamplesBase::InteractiveCurve)::DownCast(ch)) {
                                    return;
                                }
                            }
                            context()->Activate(primitive, AIS_Shape::SelectionMode(TopAbs_SHAPE));
                        }
                    });
                }
                ctx->NextDetected();
            }
        }
        if (!menu.isEmpty()) {
            ret = menu.exec(event->globalPos()) != nullptr;
        }
    }
    return ret;
}

bool Viewport::mouseMoved(QMouseEvent *event)
{
    auto ctx = context();
    if (!ctx) {
        return false;
    }

    if (d_ptr->mStartCut) {
        if (d_ptr->mEndCut) {
            ctx->Remove(d_ptr->mEndCut, Standard_False);
            d_ptr->mCutPrimitive->RemoveChild(d_ptr->mEndCut);
            d_ptr->mEndCut = nullptr;
        }
        if (d_ptr->mCutLine) {
            ctx->Remove(d_ptr->mCutLine, Standard_False);
            d_ptr->mCutPrimitive->RemoveChild(d_ptr->mCutLine);
            d_ptr->mCutLine = nullptr;
        }

        ctx->MainSelector()->Pick(event->pos().x(), event->pos().y(), view());
        if (ctx->MainSelector()->NbPicked() > 0) {
            auto startPointLocal = d_ptr->mStartCut->Component()->Pnt();
            auto endPoint = ctx->MainSelector()->PickedPoint(1);
            ctx->InitDetected();
            while (ctx->MoreDetected()) {
                auto primitive = Handle(InteractivePrimitive)::DownCast(ctx->DetectedInteractive());
                if (primitive && primitive == d_ptr->mCutPrimitive) {
                    auto endPointLocal = endPoint.Transformed(primitive->Transformation().Inverted());
                    auto endCutFace = ExamplesBase::findFaceByPoint(primitive->Shape(), endPointLocal);
                    if (!startPointLocal.IsEqual(endPointLocal, Precision::Confusion())
                            && endCutFace == d_ptr->mStartCutFace) {
                        d_ptr->mEndCut = new AIS_Point(new Geom_CartesianPoint(endPointLocal));
                        d_ptr->mEndCut->SetZLayer(depthOffLayer());
                        primitive->AddChild(d_ptr->mEndCut);
                        ctx->Display(d_ptr->mEndCut, Standard_False);
                        ctx->Deactivate(d_ptr->mEndCut);
                        auto surf = BRep_Tool::Surface(d_ptr->mStartCutFace);
                        Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(surf);
                        auto startUV = surfAnalis->ValueOfUV(startPointLocal, Precision::Confusion());
                        auto endUV = surfAnalis->ValueOfUV(endPointLocal, Precision::Confusion());
                        Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(startUV, endUV);
#if OCC_VERSION_HEX > 0x070500
                        Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_Curve(curve),
                                                             new BRepAdaptor_Surface(d_ptr->mStartCutFace));
#else
                        Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_HCurve(curve),
                                                             new BRepAdaptor_HSurface(d_ptr->mStartCutFace));
#endif
                        BRepBuilderAPI_MakeEdge builder(curve, surf);
                        d_ptr->mCutLine = new AIS_Shape(builder.Edge());
                        d_ptr->mCutLine->SetZLayer(depthOffLayer());
                        primitive->AddChild(d_ptr->mCutLine);
                        ctx->Display(d_ptr->mCutLine, Standard_False);
                        ctx->Deactivate(d_ptr->mCutLine);
                        break;
                    }
                }
                ctx->NextDetected();
            }
        }
        return true;
    }
    return false;
}
