#include "viewport.h"

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
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
#include <BOPTools_AlgoTools3D.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Axis1Placement.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Curve.hxx>
#include <gp_Quaternion.hxx>
#include <Graphic3d_Vec2.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "aspectwindow.h"
#include "brepserializer.h"
#include "InteractiveObjects/interactivenormal.h"
#include "ModelLoader/steploader.h"
#include "normaldetector.h"

static constexpr size_t sPerfPointsCount = 1000000ul;

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

    void OnSelectionChanged (const Handle(AIS_InteractiveContext)& theCtx,
                             const Handle(V3d_View)& theView) final {
        Q_ASSERT(theCtx == mContext && theView == mView);

        // remove subshapes from model and context
        auto children = mModel->Children();
        for (const auto &ch : children) {
            auto o = Handle(AIS_InteractiveObject)::DownCast(ch);
            if (o && !Handle(InteractiveNormal)::DownCast(o)) {
                mContext->Remove(o, Standard_False);
                mModel->RemoveChild(o);
            }
        }

        // deserialize and draw previos edges
        for (const auto &str : qAsConst(mPreviosEdges)) {
            auto shape = BrepSerializer::deserialize(str.toStdString());
            if (shape.ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(shape);
                auto shapeObj = new AIS_Shape(edge);
                mModel->AddChild(shapeObj);
                mContext->Display(shapeObj, Standard_False);
            }
        }
        mPreviosEdges.clear();

        // store selected edges and draw points on them
        auto edges = selectedEdges();
        for (const auto &edge : edges) {
            std::string serializedEdge = BrepSerializer::serialize(edge);
            mPreviosEdges << QString::fromStdString(serializedEdge);

            // curve
            TopLoc_Location loc;
            Standard_Real F = 0.;
            Standard_Real L = 0.;
            auto curve = BRep_Tool::Curve(edge, loc, F, L);
            qDebug() << "Curve:" << F << L << curve->FirstParameter() << curve->LastParameter();

            // points on curve
            Standard_Real pos[3] = {
                F, // start
                (L - F) / 2. + F, // mid
                L //end
            };

            TopoDS_Face face;
            bool hasFace = findFaceByEdge(edge, face);
            for (auto vl : pos) {
                gp_Pnt p = curve->Value(vl);
                if (hasFace) {
                    // normal
                    auto aisFace = new AIS_Shape(face);
                    mModel->AddChild(aisFace);
                    mContext->Display(aisFace, Standard_False);

                    const gp_Dir normal = NormalDetector::getNormal(face, p);
                    const gp_Pnt normalEnd = p.Translated(normal.XYZ() * 5);
                    qDebug() << "Point    :" << p.X() << p.Y() << p.Z();
                    qDebug() << "Normal   :" << normal.X() << normal.Y() << normal.Z();
                    qDebug() << "NormalEnd:" << normalEnd.X() << normalEnd.Y() << normalEnd.Z();
                    auto lineN = new AIS_Line(new Geom_CartesianPoint(p), new Geom_CartesianPoint(normalEnd));
                    lineN->SetWidth(2.);
                    mModel->AddChild(lineN);
                    mContext->Display(lineN, Standard_False);
                    mContext->SetZLayer(lineN, mDepthOffLayer);
                    gp_Dir axis(0., 0., 1.);
                    if (normal.IsParallel(axis, Precision::Confusion())) {
                        axis = gp_Dir(0., 1., 0.);
                    }
                    const gp_Dir normalRotation = normal.Crossed(axis);
                    const gp_Pnt normalRotationEnd = p.Translated(normalRotation.XYZ() * 5);
                    auto lineV = new AIS_Line(new Geom_CartesianPoint(p), new Geom_CartesianPoint(normalRotationEnd));
                    lineV->SetColor(Quantity_NOC_BLUE);
                    lineV->SetWidth(2.);
                    mModel->AddChild(lineV);
                    mContext->Display(lineV, Standard_False);
                    mContext->SetZLayer(lineV, mDepthOffLayer);
                } else {
                    // point
                    auto ap = new AIS_Point(new Geom_CartesianPoint(p));
                    mModel->AddChild(ap);
                    mContext->Display(ap, Standard_False);
                }
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
        for (TopExp_Explorer anExp(mModel->Shape(), TopAbs_FACE); anExp.More(); anExp.Next()) {
            auto &curFace = TopoDS::Face(anExp.Current());
            BRepClass3d_SolidClassifier classifier(curFace);
            classifier.Perform(localPnt, 1.);
            const TopAbs_State classifierState = classifier.State();
            if (classifierState == TopAbs_ON) {
                face = curFace;
                return true;
            }
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

    Handle(V3d_Viewer) mViewer;
    Handle(V3d_View) mView;
    Handle(AspectWindow) mAspect;
    Handle(AIS_InteractiveContext) mContext;

    Graphic3d_ZLayerId mDepthOffLayer = Graphic3d_ZLayerId_UNKNOWN;

    Handle(AIS_Shape) mModel;

    QStringList mPreviosEdges;
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

    const char *modelPath = "Models/45deg AdjMirr Adapter Left Rev1.STEP";
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
//    transform.SetTranslationPart(gp_Vec(10, 20, 30));
    gp_Quaternion quat;
    quat.SetEulerAngles(gp_Extrinsic_XYZ, 10., 10., 10.);
//    transform.SetRotationPart(quat);
    d_ptr->mContext->SetLocation(obj, transform);
    d_ptr->mContext->SetLocation(trihedron, transform);
    d_ptr->mContext->SetDisplayMode(obj, AIS_Shaded, Standard_True);

    d_ptr->mContext->SetSelectionModeActive(obj,
                                            AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                            Standard_False);
    d_ptr->mContext->SetSelectionModeActive(obj,
                                            AIS_Shape::SelectionMode(TopAbs_EDGE),
                                            Standard_True);
    d_ptr->mContext->SetSelectionSensitivity(obj,
                                             AIS_Shape::SelectionMode(TopAbs_EDGE),
                                             30);
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
        // Detect d_ptr->mModel under cursor
        d_ptr->mContext->SetSelectionModeActive(d_ptr->mModel,
                                                AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                                Standard_True,
                                                AIS_SelectionModesConcurrency_Single,
                                                Standard_True);
        d_ptr->mContext->MainSelector()->Pick(aPnt.x(), aPnt.y(), d_ptr->mView);
        auto owner = d_ptr->mContext->MainSelector()->Picked(1);
        if (owner && owner->Selectable() == d_ptr->mModel) {
            auto point = d_ptr->mContext->MainSelector()->PickedPoint(1);
            point.Transform(d_ptr->mContext->Location(d_ptr->mModel).Transformation().Inverted());
            QMenu menu;
            menu.addAction(tr("Add normal"), this, [this, point](){
                d_ptr->addNormal(point);
            });
            menu.exec(event->globalPos());
        }
        d_ptr->mContext->SetSelectionModeActive(d_ptr->mModel,
                                                AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                                Standard_False);
        d_ptr->mContext->SetSelectionModeActive(d_ptr->mModel,
                                                AIS_Shape::SelectionMode(TopAbs_EDGE),
                                                Standard_True);
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
}

void ViewPort::wheelEvent(QWheelEvent *event)
{
    const QPoint pos = event->position().toPoint();
    const Graphic3d_Vec2i aPos(pos.x(), pos.y());
    if (d_ptr->UpdateZoom(Aspect_ScrollDelta(aPos, event->angleDelta().y() / 8))) {
        update();
    }
}
