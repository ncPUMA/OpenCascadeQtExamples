#include "viewport.h"

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_ExtCC.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRep_Tool.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_CartesianPoint.hxx>
#include <gp_Quaternion.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include <ExamplesBase/InteractiveObjects/interactivecurve.h>
#include <ExamplesBase/InteractiveObjects/interactivenormal.h>
#include <ExamplesBase/InteractiveObjects/interactivefacenormal.h>
#include <ExamplesBase/InteractiveObjects/interactivefacenormalserializer.h>

#include "brepserializer.h"
#include "normaldetector.h"

static constexpr size_t sPerfPointsCount = 1000000ul;

class CurveObserver : public ExamplesBase::InteractiveCurve::Observer
{
    friend class Viewport;

    void handleChanged() override {
        if (mViewport) {
            mViewport->debugCurve();
        }
    }

    Viewport *mViewport = nullptr;;
};

class ViewportPrivate
{
    friend class Viewport;

    static bool findEdgeOnFace(const TopoDS_Face &face, const TopoDS_Edge &edge, TopoDS_Edge &edgeOnFace) {
        for (TopExp_Explorer faceExplorer(face, TopAbs_EDGE); faceExplorer.More(); faceExplorer.Next()) {
            auto &curEdge = TopoDS::Edge(faceExplorer.Current());
            BRepExtrema_ExtCC dist(curEdge, edge);
            if (dist.NbExt() == 1 && dist.SquareDistance(1) < Precision::Confusion()) {
                edgeOnFace = curEdge;
                return true;
            }
        }
        return false;
    }

    void drawEdge(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer,
                  const std::string &serializedEdge, const std::string &serializedFace, bool drawDeriatives) {
        auto edge = BrepSerializer::deserialize(serializedEdge);
        auto face = BrepSerializer::deserialize(serializedFace);
        if (edge.ShapeType() != TopAbs_EDGE || face.ShapeType() != TopAbs_FACE) {
            return;
        }

        // draw face
        auto faceObj = new AIS_Shape(face);
        mModel->AddChild(faceObj);
        context->Display(faceObj, Standard_False);
        context->Deactivate(faceObj);
        mEdgeFaceLines.push_back(faceObj);
        // draw edge
        auto edgeObj = new AIS_Shape(edge);
        mModel->AddChild(edgeObj);
        auto drawer = edgeObj->Attributes();
        auto aspect = drawer->WireAspect();
        if (!aspect) {
            aspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 2.);
        } else {
            aspect->SetColor(Quantity_NOC_RED);
            aspect->SetWidth(2.);
        }
        drawer->SetWireAspect(aspect);
        context->SetLocalAttributes(edgeObj, drawer, Standard_False);
        context->Display(edgeObj, Standard_False);
        context->Deactivate(edgeObj);
        mEdgeFaceLines.push_back(edgeObj);

        const TopoDS_Face curFace = TopoDS::Face(face);
        TopoDS_Edge faceEdge;
        if (!findEdgeOnFace(curFace, TopoDS::Edge(edge), faceEdge)) {
            return;
        }
        // calc normal and deriatives
        BRepAdaptor_Curve curve(faceEdge, curFace);
        const Standard_Real Umin = curve.FirstParameter();
        const Standard_Real Umax = curve.LastParameter();
        const int pointCount = 5;
        for (int i = 0; i < pointCount; ++i) {
            const Standard_Real U = Umin + (Umax - Umin) /
                    static_cast <Standard_Real> (pointCount) * static_cast <Standard_Real> (i);
            gp_Pnt P;
            gp_Vec V1, V2, V3;
            curve.D2(U, P, V1, V2);
            const gp_Dir normal = NormalDetector::getNormal(TopoDS::Face(face), P);

            const gp_Pnt normalEnd = P.Translated(normal.XYZ() * 5);
            auto lineN = new AIS_Line(new Geom_CartesianPoint(P), new Geom_CartesianPoint(normalEnd));
            lineN->SetWidth(2.);
            mModel->AddChild(lineN);
            context->Display(lineN, Standard_False);
            context->Deactivate(lineN);
            context->SetZLayer(lineN, depthOffLayer);
            mEdgeFaceLines.push_back(lineN);

            if (!drawDeriatives) {
                continue;
            }

            const gp_Pnt V1end = P.Translated(V1.Normalized() * 5);
            auto lineV1 = new AIS_Line(new Geom_CartesianPoint(P), new Geom_CartesianPoint(V1end));
            lineV1->SetWidth(2.);
            lineV1->SetColor(Quantity_NOC_GREEN);
            mModel->AddChild(lineV1);
            context->Display(lineV1, Standard_False);
            context->Deactivate(lineV1);
            context->SetZLayer(lineV1, depthOffLayer);
            mEdgeFaceLines.push_back(lineV1);

            const gp_Pnt V2end = P.Translated(V2.Normalized() * 5);
            auto lineV2 = new AIS_Line(new Geom_CartesianPoint(P), new Geom_CartesianPoint(V2end));
            lineV2->SetWidth(2.);
            lineV2->SetColor(Quantity_NOC_VIOLET);
            mModel->AddChild(lineV2);
            context->Display(lineV2, Standard_False);
            context->Deactivate(lineV2);
            context->SetZLayer(lineV2, depthOffLayer);
            mEdgeFaceLines.push_back(lineV2);
        }
    }


    bool selectionChanged(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer) {
        if (mStartCut) {
            return false;
        }

        for (const auto &o : mEdgeFaceLines) {
            context->Remove(o, Standard_False);
        }
        mEdgeFaceLines.clear();

        context->SetSelectionModeActive(mModel,
                                        AIS_Shape::SelectionMode(TopAbs_FACE),
                                        Standard_False);
        context->SetSelectionModeActive(mModel,
                                        AIS_Shape::SelectionMode(TopAbs_EDGE),
                                        Standard_True,
                                        AIS_SelectionModesConcurrency_Single);

        Handle(StdSelect_BRepOwner) owner;
        context->InitSelected();
        if (context->MoreSelected()) {
            owner = Handle(StdSelect_BRepOwner)::DownCast(context->SelectedOwner());
        }
        if (owner) {
            if (owner->Shape().ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(owner->Shape());
                TopoDS_Face face;
                if (findFaceByEdge(edge, face)) {
                    mPreviosEdge = BrepSerializer::serialize(edge);
                    const std::string serializedFace = BrepSerializer::serialize(face);
                    drawEdge(context, depthOffLayer, mPreviosEdge, serializedFace, false);

                    context->SetSelectionModeActive(mModel,
                                                    AIS_Shape::SelectionMode(TopAbs_EDGE),
                                                    Standard_False);
                    context->SetSelectionModeActive(mModel,
                                                    AIS_Shape::SelectionMode(TopAbs_FACE),
                                                    Standard_True,
                                                    AIS_SelectionModesConcurrency_Single);
                }
            } else if (!mPreviosEdge.empty() && owner->Shape().ShapeType() == TopAbs_FACE) {
                const std::string serializedFace = BrepSerializer::serialize(TopoDS::Face(owner->Shape()));
                drawEdge(context, depthOffLayer, mPreviosEdge, serializedFace, true);

                context->SetSelectionModeActive(mModel,
                                                AIS_Shape::SelectionMode(TopAbs_FACE),
                                                Standard_False);
                context->SetSelectionModeActive(mModel,
                                                AIS_Shape::SelectionMode(TopAbs_EDGE),
                                                Standard_True,
                                                AIS_SelectionModesConcurrency_Single);
                context->ClearSelected(Standard_False);
            } else {
                context->ClearSelected(Standard_False);
            }
        }
        return true;
    }

    inline static std::vector <TopoDS_Edge> selectedEdges(const Handle(AIS_InteractiveContext) &context) {
        std::vector <TopoDS_Edge> edges;
        for(context->InitSelected(); context->MoreSelected(); context->NextSelected()) {
            auto edgeOwner = Handle(StdSelect_BRepOwner)::DownCast(context->SelectedOwner());
            if (edgeOwner && edgeOwner->Shape().ShapeType() == TopAbs_EDGE) {
                edges.push_back(TopoDS::Edge(edgeOwner->Shape()));
            }
        }
        return edges;
    }

    bool findFaceByEdge(const TopoDS_Edge &edge, TopoDS_Face &face) const {
        for (TopExp_Explorer faceExplorer(mModel->Shape(), TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            auto &curFace = TopoDS::Face(faceExplorer.Current());
            if (!curFace.IsNull()) {
                for (TopExp_Explorer edgeExplorer(curFace, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
                    const auto &edgeOnFace = TopoDS::Edge(edgeExplorer.Current());
                    if (!edgeOnFace.IsNull() && edgeOnFace.IsEqual(edge)) {
                        face = curFace;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool findFaceByPoint(const gp_Pnt &localPnt, TopoDS_Face &face) const {
        BRepBuilderAPI_MakeVertex builder(localPnt);
        std::map <Standard_Real, TopoDS_Face> faces;
        for (TopExp_Explorer anExp(mModel->Shape(), TopAbs_FACE); anExp.More(); anExp.Next()) {
            auto &curFace = TopoDS::Face(anExp.Current());
            BRepExtrema_DistShapeShape extrema(curFace, builder.Vertex(), Extrema_ExtFlag_MINMAX, Extrema_ExtAlgo_Tree);
            if (extrema.IsDone()) {
                faces[extrema.Value()] = curFace;
            }
        }
        if (!faces.empty()) {
            face = faces.cbegin()->second;
            return true;
        }
        return false;
    }

    void addNormal(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer, const gp_Pnt &pnt) {
        TopoDS_Face face;
        if (!findFaceByPoint(pnt, face)) {
            return;
        }

        auto normal = NormalDetector::getNormal(face, pnt);
        auto normalObj = new ExamplesBase::InteractiveNormal;
        normalObj->setLabel("Р1"); //cyrilic test
        gp_Trsf trsf;
        trsf.SetTranslationPart(gp_Vec(pnt.XYZ()));
        gp_Quaternion quat;
        quat.SetRotation(gp::DZ().XYZ(), normal.XYZ());
        trsf.SetRotationPart(quat);
        normalObj->SetLocalTransformation(trsf);
        mModel->AddChild(normalObj);
        context->Display(normalObj, Standard_True);
        context->SetZLayer(normalObj, depthOffLayer);
    }

    void addFaceNormal(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer, const gp_Pnt &pnt) {
        TopoDS_Face face;
        if (!findFaceByPoint(pnt, face)) {
            return;
        }

        auto normalObj = new ExamplesBase::InteractiveFaceNormal(face, pnt);
        normalObj->setLabel("РN1"); //cyrilic test
        mModel->AddChild(normalObj);
        context->Display(normalObj, Standard_True);
        context->SetZLayer(normalObj, depthOffLayer);
    }

    void addStartCut(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer, const gp_Pnt &pnt) {
        if (!findFaceByPoint(pnt, mStartCutFace)) {
            return;
        }

        mStartCut = new AIS_Point(new Geom_CartesianPoint(pnt));
        mModel->AddChild(mStartCut);
        context->Display(mStartCut, Standard_False);
        context->Deactivate(mStartCut);
        context->SetZLayer(mStartCut, depthOffLayer);

        context->SetSelectionModeActive(mModel,
                                        AIS_Shape::SelectionMode(TopAbs_FACE),
                                        Standard_True,
                                        AIS_SelectionModesConcurrency_Single);
        context->SetSelectionModeActive(mModel,
                                        AIS_Shape::SelectionMode(TopAbs_EDGE),
                                        Standard_False);
    }

    bool redrawCutCurve(const Handle(AIS_InteractiveContext) &context, const Handle(V3d_View) &view,
                        Standard_Integer depthOffLayer, const Graphic3d_Vec2i &pos) {
        if (mStartCut) {
            if (mCutLine) {
                context->Remove(mCutLine, Standard_False);
            }
            if (mEndCut) {
                context->Remove(mEndCut, Standard_False);
                mEndCut.reset(nullptr);
            }

            context->MainSelector()->Pick(pos.x(), pos.y(), view);
            auto owner = context->MainSelector()->Picked(1);
            if (owner && owner->Selectable() == mModel) {
                auto point = context->MainSelector()->PickedPoint(1);
                point.Transform(context->Location(mModel).Transformation().Inverted());
                TopoDS_Face face;
                if (findFaceByPoint(point, face) && face == mStartCutFace) {
                    mEndCut = new AIS_Point(new Geom_CartesianPoint(point));
                    mModel->AddChild(mEndCut);
                    context->Display(mEndCut, Standard_False);
                    context->Deactivate(mEndCut);
                    context->SetZLayer(mEndCut, depthOffLayer);

                    auto surf = BRep_Tool::Surface(mStartCutFace);
                    Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(surf);
                    auto startUV = surfAnalis->ValueOfUV(mStartCut->Component()->Pnt(), Precision::Confusion());
                    auto endUV = surfAnalis->ValueOfUV(point, Precision::Confusion());
                    Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(startUV, endUV);
                    Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_Curve(curve),
                                                         new BRepAdaptor_Surface(mStartCutFace));
                    BRepBuilderAPI_MakeEdge builder(curve, surf);
                    mCutLine = new AIS_Shape(builder.Edge());
                    mModel->AddChild(mCutLine);
                    context->Display(mCutLine, Standard_False);
                    context->Deactivate(mCutLine);
                    context->SetZLayer(mCutLine, depthOffLayer);
                }
            }
            return true;
        }
        return false;
    }

    bool endCut(const Handle(AIS_InteractiveContext) &context, Standard_Integer depthOffLayer) {
        if (!mStartCut) {
            return false;
        }

        if (mEndCut) {
            if (mCurve) {
                context->Remove(mCurve, Standard_False);
            }

            mCurve = new ExamplesBase::InteractiveCurve(mStartCutFace,
                                                        mStartCut->Component()->Pnt(),
                                                        mEndCut->Component()->Pnt());
            mModel->AddChild(mCurve);
            context->Display(mCurve, Standard_False);
            context->SetZLayer(mCurve, depthOffLayer);
            mCurve->addObserver(&mCurveObserver);
        }

        context->Remove(mStartCut, Standard_False);
        if (mEndCut) {
            context->Remove(mEndCut, Standard_False);
        }
        if (mCutLine) {
            context->Remove(mCutLine, Standard_False);
        }
        mStartCut.reset(nullptr);
        mEndCut.reset(nullptr);
        mCutLine.reset(nullptr);
        return true;
    }

    Handle(AIS_Shape) mModel;
    std::vector <Handle(AIS_InteractiveObject)> mEdgeFaceLines;
    std::string mPreviosEdge;

    TopoDS_Face mStartCutFace;
    Handle(AIS_Point) mStartCut, mEndCut;
    Handle(AIS_Shape) mCutLine;

    Handle(ExamplesBase::InteractiveCurve) mCurve;
    CurveObserver mCurveObserver;

    NCollection_DataMap <TopoDS_Face, QByteArray> mSerializedNormals;
    NCollection_DataMap <TopoDS_Face, QByteArray> mSerializedCurves;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    Handle(AIS_Trihedron) trihedron;
    if (loadModel(QStringLiteral("../Models/tube_with_cuts.step"), d_ptr->mModel, trihedron)) {
        auto ctx = context();
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
        ctx->SetSelectionModeActive(d_ptr->mModel,
                                    AIS_Shape::SelectionMode(TopAbs_EDGE),
                                    Standard_True,
                                    AIS_SelectionModesConcurrency_Single);
    }
    d_ptr->mCurveObserver.mViewport = this;
}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::debugCurve() const
{
    auto curvesCount = d_ptr->mCurve->curvesCount();
    for (size_t i = 0; i < curvesCount; ++i) {
        Standard_Real min, max;
        if (d_ptr->mCurve->getMinMaxUParameter(i, min, max)) {
            qDebug() << i << "min:" << min << "max:" << max;
        } else {
            qDebug() << i << "no min max";
        }
    }
}

void Viewport::slNormalV1Test()
{
    auto action = qobject_cast <QAction *> (sender());
    auto edges = d_ptr->selectedEdges(context());
    if (edges.empty()) {
        qDebug() << action->text() << tr("Edge not selected");
        return;
    }

    auto edge = edges.front();
    // curve
    TopLoc_Location loc;
    Standard_Real F = 0.;
    Standard_Real L = 0.;
    auto curve = BRep_Tool::Curve(edge, loc, F, L);
    // points on curve
    std::vector <gp_Pnt> points;
    points.reserve(sPerfPointsCount);
    auto step = (L - F) / static_cast <Standard_Real> (sPerfPointsCount);
    QElapsedTimer t;
    t.start();
    for (auto i = 0; i < sPerfPointsCount; ++i) {
        auto p = F + step * static_cast <Standard_Real> (i);
        points.push_back(curve->Value(p));
    }
    qDebug() << "Find" << points.size() << "points on curve by" << t.elapsed() << "ms.";

    TopoDS_Face face;
    if (!d_ptr->findFaceByEdge(edge, face)) {
        qDebug() << action->text() << tr("Face not finded");
        return;
    }
    // normals
    std::vector <gp_Dir> normals;
    normals.reserve(points.size());
    t.restart();
    normals = NormalDetector::getNormals(face, points);
    qDebug() << "Find" << normals.size() << "normals on curve by" << t.elapsed() << "ms.";
}

bool Viewport::selectionChanged()
{
    return d_ptr->selectionChanged(context(), depthOffLayer());
}

bool Viewport::mousePressed(QMouseEvent *)
{
    return d_ptr->endCut(context(), depthOffLayer());
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
        auto ctx = context();
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        if (ctx->MainSelector()->NbPicked()) {
            auto owner = ctx->MainSelector()->Picked(1);
            if (owner) {
                auto point = ctx->MainSelector()->PickedPoint(1);
                point.Transform(ctx->Location(d_ptr->mModel).Transformation().Inverted());
                bool ownerIsCurve = owner->Selectable() == d_ptr->mCurve;
                if (d_ptr->mCurve && !ownerIsCurve) {
                    for (const auto &p : d_ptr->mCurve->Children()) {
                        if (owner->Selectable() == p) {
                            ownerIsCurve = true;
                            break;
                        }
                    }
                }
                if (ownerIsCurve) {
                    QMenu menu;
                    auto entity = ctx->MainSelector()->OnePicked();
                    size_t curveIndex = 0u;
                    if (d_ptr->mCurve->isCurvePicked(entity, curveIndex)) {
                        menu.addAction(tr("Add point"), this, [this, curveIndex, point](){
                            d_ptr->mCurve->addCurve(curveIndex, point);
                        });
                        menu.addAction(tr("Create arc of circle"), this, [this, curveIndex, point](){
                            d_ptr->mCurve->addArcOfCircle(curveIndex, point);
                        });
                    }
                    size_t pointIndex = 0u;
                    if (d_ptr->mCurve->curvesCount() > 1 && d_ptr->mCurve->isPointPicked(entity, curveIndex, pointIndex)) {
                        menu.addAction(tr("Remove point"), this, [this, curveIndex](){
                            d_ptr->mCurve->removeCurve(curveIndex);
                        });
                    }
                    if (!menu.isEmpty()) {
                        menu.addSeparator();
                    }
                    menu.addAction(tr("Save"), this, [this](){
                        QJsonDocument doc(d_ptr->mCurve->toJson());
                        d_ptr->mSerializedCurves.UnBind(d_ptr->mCurve->face());
                        d_ptr->mSerializedCurves.Bind(d_ptr->mCurve->face(), doc.toJson());
                    });
                    if (!menu.isEmpty()) {
                        menu.addSeparator();
                    }
                    menu.addAction(tr("Remove cut line"), this, [this](){
                        context()->Remove(d_ptr->mCurve, Standard_True);
                        d_ptr->mCurve.reset(nullptr);
                    });
                    if (!menu.isEmpty()) {
                        menu.exec(event->globalPos());
                    }
                    return true;
                }

                auto normal = Handle(ExamplesBase::InteractiveFaceNormal)::DownCast(owner->Selectable());
                if (normal) {
                    QMenu menu;
                    menu.addAction(tr("Remove %1").arg(normal->getLabel().ToCString()), this, [this, normal](){
                        context()->Remove(normal, Standard_True);
                    });
                    menu.addAction(tr("Save %1").arg(normal->getLabel().ToCString()), this, [this, normal](){
                        QJsonDocument doc(ExamplesBase::InteractiveFaceNormalSerializer::serialize(normal));
                        d_ptr->mSerializedNormals.UnBind(normal->face());
                        d_ptr->mSerializedNormals.Bind(normal->face(), doc.toJson());
                    });
                    menu.exec(event->globalPos());
                    return true;
                }
            }
        }

        // Detect d_ptr->mModel under cursor
        ctx->SetSelectionModeActive(d_ptr->mModel,
                                   AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                   Standard_True,
                                   AIS_SelectionModesConcurrency_Single,
                                   Standard_False);
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        auto owner = ctx->MainSelector()->Picked(1);
        if (owner && owner->Selectable() == d_ptr->mModel) {
            auto point = ctx->MainSelector()->PickedPoint(1);
            point.Transform(ctx->Location(d_ptr->mModel).Transformation().Inverted());
            QMenu menu;
            menu.addAction(tr("Add normal"), this, [this, point](){
                d_ptr->addNormal(context(), depthOffLayer(), point);
            });
            menu.addAction(tr("Add face normal"), this, [this, ctx, point](){
                d_ptr->addFaceNormal(context(), depthOffLayer(), point);
            });
            menu.addAction(tr("Add cut line"), this, [this, point]{
                d_ptr->addStartCut(context(), depthOffLayer(), point);
            });

            TopoDS_Face face;
            if (d_ptr->findFaceByPoint(point, face)) {
                auto normalValue = d_ptr->mSerializedNormals.Seek(face);
                if (normalValue) {
                    menu.addSeparator();
                    menu.addAction(tr("Load point"), this, [this, normalValue]{
                        auto normal = ExamplesBase::InteractiveFaceNormalSerializer::deserialize(QJsonDocument::fromJson(*normalValue).object());
                        if (normal) {
                            d_ptr->mModel->AddChild(normal);
                            context()->Display(normal, Standard_True);
                            normal->SetZLayer(depthOffLayer());
                        }
                    });
                }

                auto curveValue = d_ptr->mSerializedCurves.Seek(face);
                if (curveValue) {
                    menu.addSeparator();
                    menu.addAction(tr("Load curve"), this, [this, curveValue]{
                        auto curve = ExamplesBase::InteractiveCurve::fromJson(QJsonDocument::fromJson(*curveValue).object());
                        if (curve) {
                            if (d_ptr->mCurve) {
                                context()->Remove(d_ptr->mCurve, Standard_False);
                            }

                            d_ptr->mCurve = curve;
                            d_ptr->mModel->AddChild(d_ptr->mCurve);
                            context()->Display(d_ptr->mCurve, Standard_False);
                            d_ptr->mCurve->SetZLayer(depthOffLayer());
                            d_ptr->mCurve->addObserver(&d_ptr->mCurveObserver);

                            context()->Remove(d_ptr->mStartCut, Standard_False);
                            if (d_ptr->mEndCut) {
                                context()->Remove(d_ptr->mEndCut, Standard_False);
                            }
                            if (d_ptr->mCutLine) {
                                context()->Remove(d_ptr->mCutLine, Standard_False);
                            }
                            d_ptr->mStartCut.reset(nullptr);
                            d_ptr->mEndCut.reset(nullptr);
                            d_ptr->mCutLine.reset(nullptr);
                        }
                    });
                }
            }

            menu.exec(event->globalPos());
        }
        context()->SetSelectionModeActive(d_ptr->mModel,
                                          AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                          Standard_False);
        return true;
    }
    return false;
}

bool Viewport::mouseMoved(QMouseEvent *event)
{
    const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
    return d_ptr->redrawCutCurve(context(), view(), depthOffLayer(), aPnt);
}
