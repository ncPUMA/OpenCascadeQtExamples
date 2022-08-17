#include "viewport.h"

#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

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
#include "ModelLoader/steploader.h"
#include "normaldetector.h"

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
            if (o) {
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
        for(mContext->InitSelected(); mContext->MoreSelected(); mContext->NextSelected()) {
            auto edgeOwner = Handle(StdSelect_BRepOwner)::DownCast(mContext->SelectedOwner());
            if (edgeOwner && edgeOwner->Shape().ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(edgeOwner->Shape());
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

                for (auto vl : pos) {
                    gp_Pnt p = curve->Value(vl);
                    auto ap = new AIS_Point(new Geom_CartesianPoint(p));
                    mModel->AddChild(ap);
                    mContext->Display(ap, Standard_False);
                }

                // faces and normals
                for (TopExp_Explorer faceExplorer(mModel->Shape(), TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
                    auto &face = TopoDS::Face(faceExplorer.Current());
                    if (!face.IsNull()) {
                        for (TopExp_Explorer edgeExplorer(face, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
                            const auto &edgeOnFace = TopoDS::Edge(edgeExplorer.Current());
                            if (!edgeOnFace.IsNull() && edgeOnFace.IsEqual(edge)) {
                                auto aisFace = new AIS_Shape(face);
                                mModel->AddChild(aisFace);
                                mContext->Display(aisFace, Standard_False);

                                const gp_Dir normal = NormalDetector::getNormal(face, mLastPicked);
                                const gp_Pnt normalEnd = mLastPicked.Translated(normal.XYZ() * 5);
                                qDebug() << "Pick     :" << mLastPicked.X() << mLastPicked.Y() << mLastPicked.Z();
                                qDebug() << "Normal   :" << normal.X() << normal.Y() << normal.Z();
                                qDebug() << "NormalEnd:" << normalEnd.X() << normalEnd.Y() << normalEnd.Z();
                                auto lineN = new AIS_Line(new Geom_CartesianPoint(mLastPicked), new Geom_CartesianPoint(normalEnd));
                                lineN->SetWidth(2.);
                                mModel->AddChild(lineN);
                                mContext->Display(lineN, Standard_False);
                                mContext->SetZLayer(lineN, mDepthOffLayer);
                                gp_Dir axis(0., 0., 1.);
                                if (normal.IsParallel(axis, Precision::Confusion())) {
                                    axis = gp_Dir(0., 1., 0.);
                                }
                                const gp_Dir normalRotation = normal.Crossed(axis);
                                const gp_Pnt normalRotationEnd = mLastPicked.Translated(normalRotation.XYZ() * 5);
                                auto lineV = new AIS_Line(new Geom_CartesianPoint(mLastPicked), new Geom_CartesianPoint(normalRotationEnd));
                                lineV->SetColor(Quantity_NOC_BLUE);
                                lineV->SetWidth(2.);
                                mModel->AddChild(lineV);
                                mContext->Display(lineV, Standard_False);
                                mContext->SetZLayer(lineV, mDepthOffLayer);
                                break;
                            }
                        }
                    }
                }
            }
        }

        mView->Redraw();
    }

    void handleDynamicHighlight(const Handle(AIS_InteractiveContext)& theCtx,
                                const Handle(V3d_View)& theView) final {
        AIS_ViewController::handleDynamicHighlight(theCtx, theView);

        Handle(StdSelect_ViewerSelector3d) selector = mContext->MainSelector();
        if (selector->NbPicked() > 0) {
            mLastPicked = selector->PickedPoint(1);
            mLastPicked.Transform(mContext->Location(mModel).Transformation().Inverted());
        }
    }

    Handle(V3d_Viewer) mViewer;
    Handle(V3d_View) mView;
    Handle(AspectWindow) mAspect;
    Handle(AIS_InteractiveContext) mContext;

    Graphic3d_ZLayerId mDepthOffLayer = Graphic3d_ZLayerId_UNKNOWN;

    Handle(AIS_Shape) mModel;

    QStringList mPreviosEdges;
    gp_Pnt mLastPicked;
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
