#include "interactivecurve.h"

#include <list>
#include <memory>

#include <QDebug>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GeomAdaptor.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Select3D_SensitiveCurve.hxx>
#include <StdPrs_Curve.hxx>
#include <TopoDS_Face.hxx>

#include "interactivefacenormal.h"

class Point : public InteractiveFaceNormal
{
public:
    Point(Handle(InteractiveCurve) curve, const TopoDS_Face &face, const gp_Pnt &pnt)
        : InteractiveFaceNormal(face, pnt)
        , mCurve(curve) { }

    ~Point() {
        mCurve.reset(nullptr);
    }

    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                     const Handle(V3d_View) &view,
                                     const Handle(SelectMgr_EntityOwner) &owner,
                                     const Graphic3d_Vec2i &from,
                                     const Graphic3d_Vec2i &to,
                                     const AIS_DragAction action) {
        if (InteractiveFaceNormal::ProcessDragging(context, view, owner, from, to, action)) {
            switch (action) {
                case AIS_DragAction_Start:
                    break;
                case AIS_DragAction_Update:
                    context->RecomputePrsOnly(mCurve, Standard_True);
                    break;
                case AIS_DragAction_Stop:
                case AIS_DragAction_Abort:
                    context->RecomputeSelectionOnly(mCurve);
                    break;
            }
            return true;
        }
        return false;
    }

private:
    Handle(InteractiveCurve) mCurve;
};

class InteractiveCurvePrivate
{
    friend class InteractiveCurve;

    struct Curve
    {
        Curve(const Quantity_Color &color, Standard_Real width) {
            drawer = new Prs3d_Drawer();
            aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, width);
            aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
            aspect->SetInteriorColor(Quantity_ColorRGBA(aspect->Color(), 1.));
            drawer->SetLineAspect(new Prs3d_LineAspect(aspect));
        }

        virtual ~Curve() = default;

        void SetZLayer(const Graphic3d_ZLayerId layerId) {
            for (auto &p : getPoints()) {
                p->SetZLayer(layerId);
            }
        }

        void SetContext(const Handle(AIS_InteractiveContext) &context) {
            for (auto &p : getPoints()) {
                context->Display(p, Standard_False);
                context->SetSelectionModeActive(p, 2, Standard_True);
            }
        }

        virtual std::vector <Handle(Point)> getPoints() const {
            std::vector <Handle(Point)> res;
            res.push_back(lastPoint);
            return res;
        }

        virtual Adaptor3d_CurveOnSurface getCurve(const TopoDS_Face &face,
                                                  Handle(Point) &start) const {
            auto first = start->get2dPnt();
            auto last = lastPoint->get2dPnt();
            Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(first, last);
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(face));
        }

        void Draw(const Handle(Prs3d_Presentation) &presentation,
                  const TopoDS_Face &face,
                  Handle(Point) &start) const {
            StdPrs_Curve::Add(presentation, getCurve(face, start), drawer);
        }

        bool isPointPicked(const Handle(SelectMgr_EntityOwner) &entity, size_t &index) const {
            index = 0;
            for (auto &p : getPoints()) {
                if (p->isPicked(entity)) {
                    return true;
                }
            }
            return false;
        }

        Handle(Point) lastPoint;
        Handle(Graphic3d_AspectLine3d) aspect;
        Handle(Prs3d_Drawer) drawer;
        Handle(SelectMgr_EntityOwner) owner;
    };

    struct ArcOfCircle : public Curve
    {
        ArcOfCircle(const Quantity_Color &color, Standard_Real width)
            : Curve(color, width) { }

        std::vector <Handle(Point)> getPoints() const override {
            std::vector <Handle(Point)> res;
            res.push_back(midlePoint);
            res.push_back(lastPoint);
            return res;
        }

        Adaptor3d_CurveOnSurface getCurve(const TopoDS_Face &face,
                                          Handle(Point) &start) const override {
            auto first = start->get2dPnt();
            auto midle = midlePoint->get2dPnt();
            auto last = lastPoint->get2dPnt();
            Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeArcOfCircle(first, midle, last);
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(face));
        }

        Handle(Point) midlePoint;
    };

    void computeSelection(const Handle(SelectMgr_Selection) &selection) {
        auto lastPoint = mFirstPoint;
        auto it = mCurves.begin();
        while (it != mCurves.end()) {
            (*it)->owner = new SelectMgr_EntityOwner(q);
            (*it)->owner->SetComesFromDecomposition(Standard_True);
            auto curveOnSurf = (*it)->getCurve(mFace, lastPoint);
            Handle(Geom_Curve) geomCurve;
            Standard_Real maxDeviation = 0.;
            Standard_Real averageDeviation = 0.;
            GeomLib::BuildCurve3d(Precision::Confusion(), curveOnSurf, curveOnSurf.FirstParameter(), curveOnSurf.LastParameter(),
                                  geomCurve, maxDeviation, averageDeviation);
            if (geomCurve) {
                Handle(Select3D_SensitiveCurve) sens =
                        new Select3D_SensitiveCurve((*it)->owner, geomCurve, 50);
                sens->SetSensitivityFactor(5);
                selection->Add(sens);
            }
            lastPoint = (*it++)->lastPoint;
        }
    }

    void compute(const Handle(Prs3d_Presentation) &presentation, const Quantity_Color &selectionColor) {
        auto lastPoint = mFirstPoint;
        auto it = mCurves.cbegin();
        while (it != mCurves.cend()) {
            (*it)->aspect->SetColor(((*it)->owner && (*it)->owner->IsSelected()) ? selectionColor : mCurveColor);
            (*it)->Draw(presentation, mFace, lastPoint);
            lastPoint = (*it++)->lastPoint;
        }
    }

    void drawHiglighted(const Handle(SelectMgr_EntityOwner) &owner,
                        const Handle(Prs3d_Presentation) &presentation,
                        const Quantity_Color &color) {
        auto lastPoint = mFirstPoint;
        auto it = mCurves.cbegin();
        while (it != mCurves.cend()) {
            auto curve = *it++;
            if (owner == curve->owner) {
                curve->drawer = new Prs3d_Drawer();
                curve->aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mCurveWidth);
                curve->aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
                curve->aspect->SetInteriorColor(Quantity_ColorRGBA(curve->aspect->Color(), 1.));
                curve->drawer->SetLineAspect(new Prs3d_LineAspect(curve->aspect));
                curve->Draw(presentation, mFace, lastPoint);
            }
            lastPoint = curve->lastPoint;
        }
    }

    void clearSelected() {
        for (auto &curve : mCurves) {
            if (curve->owner) {
                curve->owner->SetSelected(Standard_False);
            }
        }
    }

    void addPoint(size_t index, const gp_Pnt &pnt) {
        auto it = mCurves.begin();
        while (it != mCurves.end()) {
            if (index > 0) {
                --index;
                ++it;
                continue;
            }

            auto curve = std::make_shared<Curve>(mCurveColor, mCurveWidth);
            curve->lastPoint = new Point(q, mFace, pnt);
            q->AddChild(curve->lastPoint);
            q->GetContext()->Display(curve->lastPoint, Standard_False);
            q->GetContext()->SetSelectionModeActive(curve->lastPoint, 2, Standard_True);
            curve->SetZLayer(mFirstPoint->ZLayer());
            mCurves.insert(it, curve);
            break;
        }
    }

    void removePoint(size_t index) {
        auto it = mCurves.begin();
        while (it != mCurves.end()) {
            if (index > 0) {
                --index;
                ++it;
                continue;
            }

            for (auto &p : (*it)->getPoints()) {
                q->GetContext()->Remove(p, Standard_False);
            }
            it = mCurves.erase(it);
            break;
        }
    }

    void addArcOfCircle(size_t index, const gp_Pnt &pnt) {
        auto it = mCurves.begin();
        auto lastPnt = mFirstPoint;
        while (it != mCurves.end()) {
            lastPnt = (*it)->lastPoint;
            if (index > 0) {
                --index;
                ++it;
                continue;
            }

            for (auto &p : (*it)->getPoints()) {
                q->GetContext()->Remove(p, Standard_False);
            }
            it = mCurves.erase(it);

            auto arc = std::make_shared<ArcOfCircle>(mCurveColor, mCurveWidth);
            arc->midlePoint = new Point(q, mFace, pnt);
            q->AddChild(arc->midlePoint);
            q->GetContext()->Display(arc->midlePoint, Standard_False);
            q->GetContext()->SetSelectionModeActive(arc->midlePoint, 2, Standard_True);
            arc->lastPoint = lastPnt;
            q->AddChild(arc->lastPoint);
            q->GetContext()->Display(arc->lastPoint, Standard_False);
            q->GetContext()->SetSelectionModeActive(arc->lastPoint, 2, Standard_True);
            arc->SetZLayer(mFirstPoint->ZLayer());
            mCurves.insert(it, arc);
            break;
        }
    }

    InteractiveCurve *q;
    TopoDS_Face mFace;
    Handle(Point) mFirstPoint;
    std::list < std::shared_ptr<Curve> > mCurves;

    Standard_Real mCurveWidth = 2.;
    Quantity_Color mCurveColor = Quantity_NOC_RED;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveCurve, AIS_InteractiveObject)

InteractiveCurve::InteractiveCurve(const TopoDS_Face &face, const gp_Pnt &startOnFace, const gp_Pnt &endOnFace)
    : AIS_InteractiveObject()
    , d(new InteractiveCurvePrivate)
{
    SetInfiniteState(Standard_False);
    SetMutable(Standard_True);
    SetAutoHilight(Standard_False);

    d->q = this;
    d->mFace = face;
    d->mFirstPoint = new Point(this, face, startOnFace);
    AddChild(d->mFirstPoint);

    auto curve = std::make_shared<InteractiveCurvePrivate::Curve>(d->mCurveColor, d->mCurveWidth);
    curve->lastPoint = new Point(this, face, endOnFace);
    AddChild(curve->lastPoint);
    d->mCurves.push_back(curve);
}

InteractiveCurve::~InteractiveCurve()
{
    delete d;
}

void InteractiveCurve::SetZLayer(const Graphic3d_ZLayerId layerId)
{
    d->mFirstPoint->SetZLayer(layerId);
    for (auto &curve : d->mCurves) {
        curve->SetZLayer(layerId);
    }
//    AIS_InteractiveObject::SetZLayer(layerId);
}

void InteractiveCurve::SetContext(const Handle(AIS_InteractiveContext) &context)
{
    if (myCTXPtr == context.get()) {
        return;
    }

    myCTXPtr = context.get();
    if (!context.IsNull()) {
        myDrawer->Link (context->DefaultDrawer());
        context->Display(d->mFirstPoint, Standard_False);
        for (auto &curve : d->mCurves) {
            curve->SetContext(context);
        }
    }
}

void InteractiveCurve::HilightSelected(const Handle(PrsMgr_PresentationManager) &,
                                       const SelectMgr_SequenceOfOwner &)
{
    GetContext()->RecomputePrsOnly(this, Standard_True);
}

void InteractiveCurve::ClearSelected()
{
    d->clearSelected();
    GetContext()->RecomputePrsOnly(this, Standard_True);
}

size_t InteractiveCurve::InteractiveCurve::curveCount() const
{
    return d->mCurves.size();
}

bool InteractiveCurve::isCurvePicked(const opencascade::handle<SelectMgr_EntityOwner> &entity, size_t &index) const
{
    index = 0;
    for (const auto &curve : d->mCurves) {
        if (curve->owner == entity) {
            return true;
        }
        ++index;
    }
    return false;
}

bool InteractiveCurve::isPointPicked(const Handle(SelectMgr_EntityOwner) &entity,
                                     size_t &curveIndex, size_t &pointIndex) const
{
    curveIndex = 0;
    for (const auto &curve : d->mCurves) {
        if (curve->isPointPicked(entity, pointIndex)) {
            return true;
        }
        ++curveIndex;
    }
    return false;
}

void InteractiveCurve::addPoint(size_t curveIndex, const gp_Pnt &pnt)
{
    d->addPoint(curveIndex, pnt);
    GetContext()->Redisplay(this, Standard_True);
}

void InteractiveCurve::removePoint(size_t curveIndex)
{
    if (d->mCurves.size() < 2) {
        return;
    }

    d->removePoint(curveIndex);
    GetContext()->Redisplay(this, Standard_True);
}

void InteractiveCurve::addArcOfCircle(size_t curveIndex, const gp_Pnt &pnt)
{
    d->addArcOfCircle(curveIndex, pnt);
    GetContext()->Redisplay(this, Standard_True);
}

void InteractiveCurve::ComputeSelection(const Handle(SelectMgr_Selection) &selection,
                                        const Standard_Integer mode)
{
    if (mode == 2) {
        d->computeSelection(selection);
    }
}

void InteractiveCurve::HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
                                             const Handle(Prs3d_Drawer) &theStyle,
                                             const Handle(SelectMgr_EntityOwner) &theOwner)
{
    auto presentation = GetHilightPresentation(thePM);
    if (presentation.IsNull()) {
        return;
    }

    presentation->Clear();
    Quantity_Color highlightColor = Quantity_NOC_CYAN1;
    if (myDynHilightDrawer) {
        highlightColor = myDynHilightDrawer->Color();
    } else {
        auto ctx = GetContext();
        if (ctx) {
            highlightColor = ctx->HighlightStyle()->Color();
        }
    }
    d->drawHiglighted(theOwner, presentation, highlightColor);
    auto layer = theStyle->ZLayer() != Graphic3d_ZLayerId_UNKNOWN
            ? theStyle->ZLayer()
            : myDrawer->ZLayer();
    if (presentation->GetZLayer() != layer) {
        presentation->SetZLayer(layer);
    }
    presentation->Highlight(theStyle);
    thePM->AddToImmediateList(presentation);
}

void InteractiveCurve::Compute(const Handle(PrsMgr_PresentationManager) &,
                               const Handle(Prs3d_Presentation) &presentation,
                               const Standard_Integer theMode)
{
    if (theMode != 0) {
      return;
    }

    Quantity_Color selectionColor = Quantity_NOC_GRAY80;
    if (myHilightDrawer) {
        selectionColor = myHilightDrawer->Color();
    } else {
        auto ctx = GetContext();
        if (ctx) {
            selectionColor = ctx->SelectionStyle()->Color();
        }
    }

    d->compute(presentation, selectionColor);
}
