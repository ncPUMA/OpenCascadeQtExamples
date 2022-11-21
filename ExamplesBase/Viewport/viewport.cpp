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
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "aspectwindow.h"

#include "../ExamplesBase/ModelLoader/abstractmodelloader.h"
#include "../ExamplesBase/ModelLoader/modelloaderfactorymethod.h"

namespace ExamplesBase {

class ViewportPrivate : public AIS_ViewController
{
    friend class Viewport;

    void init(QWidget *Viewport) {
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
        mAspect = new AspectWindow(Viewport);
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
        if (q_ptr->selectionChanged()) {
            mView->Redraw();
        }
    }

    Viewport *q_ptr;
    Handle(V3d_Viewer) mViewer;
    Handle(V3d_View) mView;
    Handle(AspectWindow) mAspect;
    Handle(AIS_InteractiveContext) mContext;

    Graphic3d_ZLayerId mDepthOffLayer = Graphic3d_ZLayerId_UNKNOWN;
};

Viewport::Viewport(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ViewportPrivate)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);

    d_ptr->q_ptr = this;
    d_ptr->init(this);
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool ExamplesBase::Viewport::loadModel(const QString &path, Handle(AIS_Shape) &shape, Handle(AIS_Trihedron) &trihedron) const
{
    ExamplesBase::ModelLoaderFactoryMethod factory;
    auto &loader = factory.loaderByFName(path);
    auto topo_shape = loader.load(path.toLatin1().constData());
    if (topo_shape.IsNull()) {
        return false;
    }

    shape = new AIS_Shape(topo_shape);
    d_ptr->mContext->Display(shape, Standard_False);
    auto coords = new Geom_Axis2Placement(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.));
    trihedron = new AIS_Trihedron(coords);
    trihedron->SetSize(15.);
    d_ptr->mContext->Display(trihedron, Standard_False);
    d_ptr->mContext->Deactivate(trihedron);
    d_ptr->mContext->SetDisplayMode(shape, AIS_Shaded, Standard_True);
    return true;
}

void Viewport::fitInView()
{
    d_ptr->mView->FitAll();
    d_ptr->mView->ZFitAll();
    d_ptr->mView->Redraw();
}

Handle(AIS_InteractiveContext) ExamplesBase::Viewport::context() const
{
    return d_ptr->mContext;
}

Handle(V3d_View) ExamplesBase::Viewport::view() const
{
    return d_ptr->mView;
}

Standard_Integer ExamplesBase::Viewport::depthOffLayer() const
{
    return d_ptr->mDepthOffLayer;
}

bool ExamplesBase::Viewport::selectionChanged()
{
    return false;
}

bool ExamplesBase::Viewport::mousePressed(QMouseEvent *)
{
    return false;
}

bool ExamplesBase::Viewport::mouseReleased(QMouseEvent *)
{
    return false;
}

bool ExamplesBase::Viewport::mouseMoved(QMouseEvent *)
{
    return false;
}

QPaintEngine *Viewport::paintEngine() const
{
    return nullptr;
}

void Viewport::paintEvent(QPaintEvent *)
{
    d_ptr->mView->InvalidateImmediate();
    d_ptr->FlushViewEvents(d_ptr->mContext, d_ptr->mView, Standard_True);
}

void Viewport::resizeEvent(QResizeEvent *)
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

void Viewport::mousePressEvent(QMouseEvent *event)
{
    const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false)) {
        update();
    }

    if (mousePressed(event)) {
        d_ptr->mView->Redraw();
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aPnt(pos.x(), pos.y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false)) {
        update();
    }

    if (mouseReleased(event)) {
        d_ptr->mView->Redraw();
    }
}

void Viewport::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const Graphic3d_Vec2i aNewPos(pos.x(), pos.y());
    if (d_ptr->UpdateMousePosition(aNewPos,
                                   qtMouseButtons2VKeys(event->buttons()),
                                   qtMouseModifiers2VKeys(event->modifiers()),
                                   false)) {
        update();
    }

    if (mouseMoved(event)) {
        d_ptr->mView->Redraw();
    }
}

void Viewport::wheelEvent(QWheelEvent *event)
{
    const QPoint pos = event->position().toPoint();
    const Graphic3d_Vec2i aPos(pos.x(), pos.y());
    if (d_ptr->UpdateZoom(Aspect_ScrollDelta(aPos, event->angleDelta().y() / 8))) {
        update();
    }
}

}
