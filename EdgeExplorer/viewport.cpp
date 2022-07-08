#include "viewport.h"

#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_ViewController.hxx>
#include <AIS_ViewCube.hxx>
#include <Geom_Axis2Placement.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "aspectwindow.h"
#include "ModelLoader/steploader.h"

class Aspect : public Aspect_Window
{

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
        mViewer->AddZLayer(mDeptOffLayer);
        Graphic3d_ZLayerSettings zSettings = mViewer->ZLayerSettings(mDeptOffLayer);
        zSettings.SetEnableDepthTest(Standard_False);
        mViewer->SetZLayerSettings(mDeptOffLayer, zSettings);

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
        Geom_Axis2Placement coords(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.));
        Handle(AIS_Trihedron) trihedron = new AIS_Trihedron(new Geom_Axis2Placement(coords));
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

    Handle(V3d_Viewer) mViewer;
    Handle(V3d_View) mView;
    Handle(AspectWindow) mAspect;
    Handle(AIS_InteractiveContext) mContext;

    Graphic3d_ZLayerId mDeptOffLayer = Graphic3d_ZLayerId_UNKNOWN;
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
    const TopoDS_Shape shape = loader.load(modelPath);
    Handle(AIS_Shape) obj = new AIS_Shape(shape);
    d_ptr->mContext->Display(obj, Standard_False);
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
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false))
        update();
}

void ViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aPnt(pos.x(), pos.y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false))
        update();
}

void ViewPort::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aNewPos(pos.x(), pos.y());
    if (d_ptr->UpdateMousePosition(aNewPos,
                                   qtMouseButtons2VKeys(event->buttons()),
                                   qtMouseModifiers2VKeys(event->modifiers()),
                                   false))
    {
        update();
    }
}

void ViewPort::wheelEvent(QWheelEvent *event)
{
    const QPoint pos = event->position().toPoint();
    const Graphic3d_Vec2i aPos(pos.x(), pos.y());
    if (d_ptr->UpdateZoom(Aspect_ScrollDelta(aPos, event->angleDelta().y() / 8)))
        update();
}
