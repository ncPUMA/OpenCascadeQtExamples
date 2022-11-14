#include "viewport.h"

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_Axis.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_ViewController.hxx>
#include <AIS_ViewCube.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <BOPTools_AlgoTools3D.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepExtrema_ExtCC.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRep_Tool.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Axis1Placement.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Curve.hxx>
#include <GeomLib.hxx>
#include <gp_Quaternion.hxx>
#include <Graphic3d_Vec2.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "aspectwindow.h"
#include "brepserializer.h"
#include "InteractiveObjects/interactivecurve.h"
#include "InteractiveObjects/interactivenormal.h"
#include "InteractiveObjects/interactivefacenormal.h"
#include "InteractiveObjects/interactivefacenormalserializer.h"
#include "ModelLoader/steploader.h"
#include "normaldetector.h"

static constexpr size_t sPerfPointsCount = 1000000ul;

class CurveObserver : public InteractiveCurve::Observer
{
    friend class ViewPort;

    void handleChanged() override {
        if (mViewport) {
            mViewport->debugCurve();
        }
    }

    ViewPort *mViewport = nullptr;;
};

class ViewPortPrivate : public AIS_ViewController
{
    friend class ViewPort;

    void init(QWidget *viewport) {
        // Driver
        static Handle(OpenGl_GraphicDriver) graphicDriver;
        Handle(Aspect_DisplayConnection) displayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
        displayConnection = new Aspect_DisplayConnection(OSD_Environment("DISPLAY").Value());
#endif
        graphicDriver = new OpenGl_GraphicDriver(displayConnection);

        //Viewer
        mViewer = new V3d_Viewer(graphicDriver);
        mViewer->SetDefaultViewSize(1000.);
        mViewer->SetDefaultViewProj(V3d_XposYposZneg);
        mViewer->SetComputedMode(Standard_True);
        mViewer->SetDefaultComputedMode(Standard_True);
        mViewer->SetDefaultLights();
        mViewer->SetLightOn();

        //ZLayer without depth-test
        mViewer->AddZLayer(mDepthOffLayer);
        Graphic3d_ZLayerSettings zSettings = mViewer->ZLayerSettings(mDepthOffLayer);
        zSettings.SetEnableDepthTest(Standard_False);
        mViewer->SetZLayerSettings(mDepthOffLayer, zSettings);

        //Context
        mContext = new AIS_InteractiveContext(mViewer);
        Handle(Prs3d_Drawer) drawer = mContext->DefaultDrawer();
        Handle(Prs3d_LineAspect) lAspect = drawer->FaceBoundaryAspect();
        lAspect->SetColor(Quantity_NOC_DARKSLATEGRAY);
        drawer->SetFaceBoundaryAspect(lAspect);
        drawer->SetFaceBoundaryDraw(Standard_True);

        //Add AIS_ViewCube
        auto ais_axis_cube = new AIS_ViewCube();
        mContext->Display(ais_axis_cube, Standard_False);
        mContext->SetDisplayMode(ais_axis_cube, AIS_Shaded, Standard_False);
        //Add trihedron
        auto coords = new Geom_Axis2Placement(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.));
        auto trihedron = new AIS_Trihedron(coords);
        mContext->Display(trihedron, Standard_False);
        mContext->Deactivate(trihedron);
        mView = mContext->CurrentViewer()->CreateView().get();

        //Aspect
        mAspect = new AspectWindow(viewport);
        mView->SetWindow(mAspect);
        if (!mAspect->IsMapped()) {
            mAspect->Map();
        }

        //Final
        mView->ChangeRenderingParams().IsAntialiasingEnabled = Standard_True;
        mView->MustBeResized();
        mView->ChangeRenderingParams().NbMsaaSamples = 8;
        mView->Redraw();
    }

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

    void drawEdge(const std::string &serializedEdge, const std::string &serializedFace, bool drawDeriatives) {
        auto edge = BrepSerializer::deserialize(serializedEdge);
        auto face = BrepSerializer::deserialize(serializedFace);
        if (edge.ShapeType() != TopAbs_EDGE || face.ShapeType() != TopAbs_FACE) {
            return;
        }

        // draw face
        auto faceObj = new AIS_Shape(face);
        mModel->AddChild(faceObj);
        mContext->Display(faceObj, Standard_False);
        mContext->Deactivate(faceObj);
        mEdgeFaceLines.push_back(faceObj);
        // draw edge
        auto edgeObj = new AIS_Shape(edge);
        mModel->AddChild(edgeObj);
        auto drawer = edgeObj->Attributes();
        auto aspect = drawer->WireAspect();
        aspect->SetColor(Quantity_NOC_RED);
        aspect->SetWidth(2.);
        drawer->SetWireAspect(aspect);
        mContext->SetLocalAttributes(edgeObj, drawer, Standard_False);
        mContext->Display(edgeObj, Standard_False);
        mContext->Deactivate(edgeObj);
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
            mContext->Display(lineN, Standard_False);
            mContext->Deactivate(lineN);
            mContext->SetZLayer(lineN, mDepthOffLayer);
            mEdgeFaceLines.push_back(lineN);

            if (!drawDeriatives) {
                continue;
            }

            const gp_Pnt V1end = P.Translated(V1.Normalized() * 5);
            auto lineV1 = new AIS_Line(new Geom_CartesianPoint(P), new Geom_CartesianPoint(V1end));
            lineV1->SetWidth(2.);
            lineV1->SetColor(Quantity_NOC_GREEN);
            mModel->AddChild(lineV1);
            mContext->Display(lineV1, Standard_False);
            mContext->Deactivate(lineV1);
            mContext->SetZLayer(lineV1, mDepthOffLayer);
            mEdgeFaceLines.push_back(lineV1);

            const gp_Pnt V2end = P.Translated(V2.Normalized() * 5);
            auto lineV2 = new AIS_Line(new Geom_CartesianPoint(P), new Geom_CartesianPoint(V2end));
            lineV2->SetWidth(2.);
            lineV2->SetColor(Quantity_NOC_VIOLET);
            mModel->AddChild(lineV2);
            mContext->Display(lineV2, Standard_False);
            mContext->Deactivate(lineV2);
            mContext->SetZLayer(lineV2, mDepthOffLayer);
            mEdgeFaceLines.push_back(lineV2);
        }
    }


    void OnSelectionChanged (const Handle(AIS_InteractiveContext)& theCtx,
                             const Handle(V3d_View)& theView) final {
        Q_ASSERT(theCtx == mContext && theView == mView);

        if (mStartCut) {
            return;
        }

        for (const auto &o : mEdgeFaceLines) {
            mContext->Remove(o, Standard_False);
        }
        mEdgeFaceLines.clear();

        mContext->SetSelectionModeActive(mModel,
                                         AIS_Shape::SelectionMode(TopAbs_FACE),
                                         Standard_False);
        mContext->SetSelectionModeActive(mModel,
                                         AIS_Shape::SelectionMode(TopAbs_EDGE),
                                         Standard_True,
                                         AIS_SelectionModesConcurrency_Single);

        Handle(StdSelect_BRepOwner) owner;
        mContext->InitSelected();
        if (mContext->MoreSelected()) {
            owner = Handle(StdSelect_BRepOwner)::DownCast(mContext->SelectedOwner());
        }
        if (owner) {
            if (owner->Shape().ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(owner->Shape());
                TopoDS_Face face;
                if (findFaceByEdge(edge, face)) {
                    mPreviosEdge = BrepSerializer::serialize(edge);
                    const std::string serializedFace = BrepSerializer::serialize(face);
                    drawEdge(mPreviosEdge, serializedFace, false);

                    mContext->SetSelectionModeActive(mModel,
                                                     AIS_Shape::SelectionMode(TopAbs_EDGE),
                                                     Standard_False);
                    mContext->SetSelectionModeActive(mModel,
                                                     AIS_Shape::SelectionMode(TopAbs_FACE),
                                                     Standard_True,
                                                     AIS_SelectionModesConcurrency_Single);
                }
            } else if (!mPreviosEdge.empty() && owner->Shape().ShapeType() == TopAbs_FACE) {
                const std::string serializedFace = BrepSerializer::serialize(TopoDS::Face(owner->Shape()));
                drawEdge(mPreviosEdge, serializedFace, true);

                mContext->SetSelectionModeActive(mModel,
                                                 AIS_Shape::SelectionMode(TopAbs_FACE),
                                                 Standard_False);
                mContext->SetSelectionModeActive(mModel,
                                                 AIS_Shape::SelectionMode(TopAbs_EDGE),
                                                 Standard_True,
                                                 AIS_SelectionModesConcurrency_Single);
                mContext->ClearSelected(Standard_False);
            } else {
                mContext->ClearSelected(Standard_False);
            }
        }
        mView->Redraw();
    }

    std::vector <TopoDS_Edge> selectedEdges() const {
        std::vector <TopoDS_Edge> edges;
        for(mContext->InitSelected(); mContext->MoreSelected(); mContext->NextSelected()) {
            auto edgeOwner = Handle(StdSelect_BRepOwner)::DownCast(mContext->SelectedOwner());
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

    void addNormal(const gp_Pnt &pnt) {
        TopoDS_Face face;
        if (!findFaceByPoint(pnt, face)) {
            return;
        }

        auto normal = NormalDetector::getNormal(face, pnt);
        auto normalObj = new InteractiveNormal;
        normalObj->setLabel("Р1"); //cyrilic test
        gp_Trsf trsf;
        trsf.SetTranslationPart(gp_Vec(pnt.XYZ()));
        gp_Quaternion quat;
        quat.SetRotation(gp::DZ().XYZ(), normal.XYZ());
        trsf.SetRotationPart(quat);
        normalObj->SetLocalTransformation(trsf);
        mModel->AddChild(normalObj);
        mContext->Display(normalObj, Standard_True);
        mContext->SetZLayer(normalObj, mDepthOffLayer);
        mView->Redraw();
    }

    void addFaceNormal(const gp_Pnt &pnt) {
        TopoDS_Face face;
        if (!findFaceByPoint(pnt, face)) {
            return;
        }

        auto normalObj = new InteractiveFaceNormal(face, pnt);
        normalObj->setLabel("РN1"); //cyrilic test
        mModel->AddChild(normalObj);
        mContext->Display(normalObj, Standard_True);
        mContext->SetZLayer(normalObj, mDepthOffLayer);
        mView->Redraw();
    }

    void addStartCut(const gp_Pnt &pnt) {
        if (!findFaceByPoint(pnt, mStartCutFace)) {
            return;
        }

        mStartCut = new AIS_Point(new Geom_CartesianPoint(pnt));
        mModel->AddChild(mStartCut);
        mContext->Display(mStartCut, Standard_False);
        mContext->Deactivate(mStartCut);
        mContext->SetZLayer(mStartCut, mDepthOffLayer);

        mView->Redraw();

        mContext->SetSelectionModeActive(mModel,
                                         AIS_Shape::SelectionMode(TopAbs_FACE),
                                         Standard_True,
                                         AIS_SelectionModesConcurrency_Single);
        mContext->SetSelectionModeActive(mModel,
                                         AIS_Shape::SelectionMode(TopAbs_EDGE),
                                         Standard_False);
    }

    void redrawCutCurve(const Graphic3d_Vec2i &pos) {
        if (mStartCut) {
            if (mCutLine) {
                mContext->Remove(mCutLine, Standard_False);
            }
            if (mEndCut) {
                mContext->Remove(mEndCut, Standard_False);
                mEndCut.reset(nullptr);
            }

            mContext->MainSelector()->Pick(pos.x(), pos.y(), mView);
            auto owner = mContext->MainSelector()->Picked(1);
            if (owner && owner->Selectable() == mModel) {
                auto point = mContext->MainSelector()->PickedPoint(1);
                point.Transform(mContext->Location(mModel).Transformation().Inverted());
                TopoDS_Face face;
                if (findFaceByPoint(point, face) && face == mStartCutFace) {
                    mEndCut = new AIS_Point(new Geom_CartesianPoint(point));
                    mModel->AddChild(mEndCut);
                    mContext->Display(mEndCut, Standard_False);
                    mContext->Deactivate(mEndCut);
                    mContext->SetZLayer(mEndCut, mDepthOffLayer);

                    auto surf = BRep_Tool::Surface(mStartCutFace);
                    Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(surf);
                    auto startUV = surfAnalis->ValueOfUV(mStartCut->Component()->Pnt(), Precision::Confusion());
                    auto endUV = surfAnalis->ValueOfUV(point, Precision::Confusion());
                    Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(startUV, endUV);
                    Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_HCurve(curve),
                                                         new BRepAdaptor_HSurface(mStartCutFace));
                    BRepBuilderAPI_MakeEdge builder(curve, surf);
                    mCutLine = new AIS_Shape(builder.Edge());
                    mModel->AddChild(mCutLine);
                    mContext->Display(mCutLine, Standard_False);
                    mContext->Deactivate(mCutLine);
                    mContext->SetZLayer(mCutLine, mDepthOffLayer);
                }
            }
            mView->Redraw();
        }
    }

    void endCut(const Graphic3d_Vec2i &pos) {
        if (!mStartCut) {
            return;
        }

        if (mEndCut) {
            if (mCurve) {
                mContext->Remove(mCurve, Standard_False);
            }

            mCurve = new InteractiveCurve(mStartCutFace,
                                          mStartCut->Component()->Pnt(),
                                          mEndCut->Component()->Pnt());
            mModel->AddChild(mCurve);
            mContext->Display(mCurve, Standard_False);
            mContext->SetZLayer(mCurve, mDepthOffLayer);
            mCurve->addObserver(&mCurveObserver);
        }

        mContext->Remove(mStartCut, Standard_False);
        if (mEndCut) {
            mContext->Remove(mEndCut, Standard_False);
        }
        if (mCutLine) {
            mContext->Remove(mCutLine, Standard_False);
        }
        mStartCut.reset(nullptr);
        mEndCut.reset(nullptr);
        mCutLine.reset(nullptr);
        mView->Redraw();
    }

    Handle(V3d_Viewer) mViewer;
    Handle(V3d_View) mView;
    Handle(AspectWindow) mAspect;
    Handle(AIS_InteractiveContext) mContext;

    Graphic3d_ZLayerId mDepthOffLayer = Graphic3d_ZLayerId_UNKNOWN;

    Handle(AIS_Shape) mModel;
    std::vector <Handle(AIS_InteractiveObject)> mEdgeFaceLines;
    std::string mPreviosEdge;

    TopoDS_Face mStartCutFace;
    Handle(AIS_Point) mStartCut, mEndCut;
    Handle(AIS_Shape) mCutLine;

    Handle(InteractiveCurve) mCurve;
    CurveObserver mCurveObserver;

    NCollection_DataMap <TopoDS_Face, QByteArray> mSerializedNormals;
    NCollection_DataMap <TopoDS_Face, QByteArray> mSerializedCurves;
};

ViewPort::ViewPort(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ViewPortPrivate)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);

    d_ptr->init(this);

    const char *modelPath = "Models/tube_with_cuts.step";
    StepLoader loader;
    auto shape = loader.load(modelPath);
    auto obj = new AIS_Shape(shape);
    d_ptr->mModel = obj;
    d_ptr->mContext->Display(obj, Standard_False);
    auto coords = new Geom_Axis2Placement(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.));
    auto trihedron = new AIS_Trihedron(coords);
    trihedron->SetSize(15.);
    d_ptr->mContext->Display(trihedron, Standard_False);
    d_ptr->mContext->Deactivate(trihedron);
    gp_Trsf transform;
    transform.SetTranslationPart(gp_Vec(10, 20, 30));
    gp_Quaternion quat;
    quat.SetEulerAngles(gp_Extrinsic_XYZ, 10., 10., 10.);
    transform.SetRotationPart(quat);
    d_ptr->mContext->SetLocation(obj, transform);
    d_ptr->mContext->SetLocation(trihedron, transform);
    d_ptr->mContext->SetDisplayMode(obj, AIS_Shaded, Standard_True);

    d_ptr->mContext->SetSelectionModeActive(obj,
                                            AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                            Standard_False);
    d_ptr->mContext->SetSelectionModeActive(obj,
                                            AIS_Shape::SelectionMode(TopAbs_EDGE),
                                            Standard_True,
                                            AIS_SelectionModesConcurrency_Single);
    d_ptr->mCurveObserver.mViewport = this;
}

ViewPort::~ViewPort()
{
    delete d_ptr;
}

void ViewPort::fitInView()
{
    d_ptr->mView->FitAll();
    d_ptr->mView->ZFitAll();
    d_ptr->mView->Redraw();
}

void ViewPort::debugCurve() const
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

void ViewPort::slNormalV1Test()
{
    auto action = qobject_cast <QAction *> (sender());
    auto edges = d_ptr->selectedEdges();
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

QPaintEngine *ViewPort::paintEngine() const
{
    return nullptr;
}

void ViewPort::paintEvent(QPaintEvent *)
{
    d_ptr->mView->InvalidateImmediate();
    d_ptr->FlushViewEvents(d_ptr->mContext, d_ptr->mView, Standard_True);
}

void ViewPort::resizeEvent(QResizeEvent *)
{
    d_ptr->mView->MustBeResized();
}

//! Map Qt buttons bitmask to virtual keys.
inline static Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons theButtons)
{
    Aspect_VKeyMouse aButtons = Aspect_VKeyMouse_NONE;
    if ((theButtons & Qt::LeftButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_LeftButton;
    }
    if ((theButtons & Qt::MiddleButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_MiddleButton;
    }
    if ((theButtons & Qt::RightButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_RightButton;
    }
    return aButtons;
}

//! Map Qt mouse modifiers bitmask to virtual keys.
inline static Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers theModifiers)
{
    Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
    if ((theModifiers & Qt::ShiftModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_SHIFT;
    }
    if ((theModifiers & Qt::ControlModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_CTRL;
    }
    if ((theModifiers & Qt::AltModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_ALT;
    }
    return aFlags;
}

void ViewPort::mousePressEvent(QMouseEvent *event)
{
    const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false)) {
        update();
    }

    d_ptr->endCut(aPnt);
}

void ViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aPnt(pos.x(), pos.y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false)) {
        update();
    }

    if (event->button() == Qt::RightButton) {
        d_ptr->mContext->MainSelector()->Pick(aPnt.x(), aPnt.y(), d_ptr->mView);
        if (d_ptr->mContext->MainSelector()->NbPicked()) {
            auto owner = d_ptr->mContext->MainSelector()->Picked(1);
            if (owner) {
                auto point = d_ptr->mContext->MainSelector()->PickedPoint(1);
                point.Transform(d_ptr->mContext->Location(d_ptr->mModel).Transformation().Inverted());
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
                    auto entity = d_ptr->mContext->MainSelector()->OnePicked();
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
                        d_ptr->mContext->Remove(d_ptr->mCurve, Standard_True);
                        d_ptr->mCurve.reset(nullptr);
                    });
                    if (!menu.isEmpty()) {
                        menu.exec(event->globalPos());
                    }
                    return;
                }

                auto normal = Handle(InteractiveFaceNormal)::DownCast(owner->Selectable());
                if (normal) {
                    QMenu menu;
                    menu.addAction(tr("Remove %1").arg(normal->getLabel().ToCString()), this, [this, normal](){
                        d_ptr->mContext->Remove(normal, Standard_True);
                    });
                    menu.addAction(tr("Save %1").arg(normal->getLabel().ToCString()), this, [this, normal](){
                        QJsonDocument doc(InteractiveFaceNormalSerializer::serialize(normal));
                        d_ptr->mSerializedNormals.UnBind(normal->face());
                        d_ptr->mSerializedNormals.Bind(normal->face(), doc.toJson());
                    });
                    menu.exec(event->globalPos());
                    return;
                }
            }
        }

        // Detect d_ptr->mModel under cursor
        d_ptr->mContext->SetSelectionModeActive(d_ptr->mModel,
                                                AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                                Standard_True,
                                                AIS_SelectionModesConcurrency_Single,
                                                Standard_False);
        d_ptr->mContext->MainSelector()->Pick(aPnt.x(), aPnt.y(), d_ptr->mView);
        auto owner = d_ptr->mContext->MainSelector()->Picked(1);
        if (owner && owner->Selectable() == d_ptr->mModel) {
            auto point = d_ptr->mContext->MainSelector()->PickedPoint(1);
            point.Transform(d_ptr->mContext->Location(d_ptr->mModel).Transformation().Inverted());
            QMenu menu;
            menu.addAction(tr("Add normal"), this, [this, point](){
                d_ptr->addNormal(point);
            });
            menu.addAction(tr("Add face normal"), this, [this, point](){
                d_ptr->addFaceNormal(point);
            });
            menu.addAction(tr("Add cut line"), this, [this, point]{
                d_ptr->addStartCut(point);
            });

            TopoDS_Face face;
            if (d_ptr->findFaceByPoint(point, face)) {
                auto normalValue = d_ptr->mSerializedNormals.Seek(face);
                if (normalValue) {
                    menu.addSeparator();
                    menu.addAction(tr("Load point"), this, [this, normalValue]{
                        auto normal = InteractiveFaceNormalSerializer::deserialize(QJsonDocument::fromJson(*normalValue).object());
                        if (normal) {
                            d_ptr->mModel->AddChild(normal);
                            d_ptr->mContext->Display(normal, Standard_True);
                            d_ptr->mContext->SetZLayer(normal, d_ptr->mDepthOffLayer);
                            d_ptr->mView->Redraw();
                        }
                    });
                }

                auto curveValue = d_ptr->mSerializedCurves.Seek(face);
                if (curveValue) {
                    menu.addSeparator();
                    menu.addAction(tr("Load curve"), this, [this, curveValue]{
                        auto curve = InteractiveCurve::fromJson(QJsonDocument::fromJson(*curveValue).object());
                        if (curve) {
                            if (d_ptr->mCurve) {
                                d_ptr->mContext->Remove(d_ptr->mCurve, Standard_False);
                            }

                            d_ptr->mCurve = curve;
                            d_ptr->mModel->AddChild(d_ptr->mCurve);
                            d_ptr->mContext->Display(d_ptr->mCurve, Standard_False);
                            d_ptr->mContext->SetZLayer(d_ptr->mCurve, d_ptr->mDepthOffLayer);
                            d_ptr->mCurve->addObserver(&d_ptr->mCurveObserver);

                            d_ptr->mContext->Remove(d_ptr->mStartCut, Standard_False);
                            if (d_ptr->mEndCut) {
                                d_ptr->mContext->Remove(d_ptr->mEndCut, Standard_False);
                            }
                            if (d_ptr->mCutLine) {
                                d_ptr->mContext->Remove(d_ptr->mCutLine, Standard_False);
                            }
                            d_ptr->mStartCut.reset(nullptr);
                            d_ptr->mEndCut.reset(nullptr);
                            d_ptr->mCutLine.reset(nullptr);
                            d_ptr->mView->Redraw();
                        }
                    });
                }
            }

            menu.exec(event->globalPos());
        }
        d_ptr->mContext->SetSelectionModeActive(d_ptr->mModel,
                                                AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                                Standard_False);
    }
}

void ViewPort::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aNewPos(pos.x(), pos.y());
    if (d_ptr->UpdateMousePosition(aNewPos,
                                   qtMouseButtons2VKeys(event->buttons()),
                                   qtMouseModifiers2VKeys(event->modifiers()),
                                   false)) {
        update();
    }

    d_ptr->redrawCutCurve(aNewPos);
}

void ViewPort::wheelEvent(QWheelEvent *event)
{
    const QPoint pos = event->position().toPoint();
    const Graphic3d_Vec2i aPos(pos.x(), pos.y());
    if (d_ptr->UpdateZoom(Aspect_ScrollDelta(aPos, event->angleDelta().y() / 8))) {
        update();
    }
}
