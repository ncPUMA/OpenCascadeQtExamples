#include "interactivecurve.h"

#include <list>
#include <memory>

#include <QJsonObject>
#include <QJsonArray>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRep_Tool.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>
#include <GCE2d_MakeArcOfEllipse.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GeomAdaptor.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include <gp_Quaternion.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Select3D_SensitiveCurve.hxx>
#include <Standard_Version.hxx>
#include <StdPrs_Curve.hxx>
#include <TopoDS_Face.hxx>

#if OCC_VERSION_HEX > 0x070500
#include <BRepAdaptor_Surface.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#else
#include <BRepAdaptor_HSurface.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#endif

#include "interactivefacenormal.h"
#include "interactivefacenormalserializer.h"

namespace ExamplesBase {

class Point : public InteractiveFaceNormal
{
public:
    static Handle(Point) createPoint(Handle(InteractiveCurve) curve, const TopoDS_Face &face, const gp_Pnt &pnt) {
        Handle(Point) point = new Point(curve, face, pnt);
        curve->AddChild(point);
        auto context = curve->GetContext();
        if (context) {
            context->Display(point, Standard_False);
            context->SetSelectionModeActive(point, 2, Standard_True);
        }
        return point;
    }

    static Handle(Point) createPoint(Handle(InteractiveCurve) curve, const QJsonObject &obj) {
        auto normal = InteractiveFaceNormalSerializer::deserialize(obj);
        if (!normal) {
            return nullptr;
        }
        Handle(Point) point = new Point(curve, normal->face(), normal->get2dPnt(), normal->getRotation());
        point->setLabel(normal->getLabel());
        curve->AddChild(point);
        return point;
    }

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
            mCurve->notify();
            return true;
        }
        return false;
    }

private:
    Point(Handle(InteractiveCurve) curve, const TopoDS_Face &face, const gp_Pnt &pnt)
        : InteractiveFaceNormal(face, pnt)
        , mCurve(curve) { }

    Point(Handle(InteractiveCurve) curve, const TopoDS_Face &face, const gp_Pnt2d &uv, const gp_Quaternion &rotation)
        : InteractiveFaceNormal(face, uv, rotation)
        , mCurve(curve) { }

    Handle(InteractiveCurve) mCurve;
};

class InteractiveCurvePrivate
{
    friend class InteractiveCurve;

    struct Curve
    {
        enum CurveTypes
        {
            CT_Line,
            CT_ArcOfCircle,
        };

        Curve(const Quantity_Color &color, Standard_Real width) {
            drawer = new Prs3d_Drawer();
            aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, width);
            aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
            aspect->SetInteriorColor(Quantity_ColorRGBA(aspect->Color(), 1.));
            drawer->SetLineAspect(new Prs3d_LineAspect(aspect));
        }

        virtual ~Curve() = default;

        virtual CurveTypes type() const = 0;

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

        virtual std::vector <Handle(Point)> getPoints() const  = 0;

        virtual Adaptor3d_CurveOnSurface getCurve(const TopoDS_Face &face,
                                                  Handle(Point) &start) const  = 0;

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

        Handle(Graphic3d_AspectLine3d) aspect;
        Handle(Prs3d_Drawer) drawer;
        Handle(SelectMgr_EntityOwner) owner;
    };

    struct Line :public Curve
    {
        Line(const Quantity_Color &color, Standard_Real width, const Handle(Point) &point)
            : Curve(color, width) {
            mLastPoint = point;
        }

        CurveTypes type() const override { return CT_Line; }

        std::vector <Handle(Point)> getPoints() const override {
            std::vector <Handle(Point)> res;
            res.push_back(mLastPoint);
            return res;
        }

        Adaptor3d_CurveOnSurface getCurve(const TopoDS_Face &face,
                                          Handle(Point) &start) const override {
            auto first = start->get2dPnt();
            auto last = mLastPoint->get2dPnt();
            Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(first, last);
#if OCC_VERSION_HEX > 0x070500
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_Curve(curve), new BRepAdaptor_Surface(face));
#else
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(face));
#endif
        }

        Handle(Point) mLastPoint;
    };

    struct ArcOfCircle : public Curve
    {
        ArcOfCircle(const Quantity_Color &color, Standard_Real width,
                    const Handle(Point) &midPoint, const Handle(Point) &lastPoint)
            : Curve(color, width)
            , mMidlePoint(midPoint)
            , mLastPoint(lastPoint) { }

        CurveTypes type() const override { return CT_ArcOfCircle; }

        std::vector <Handle(Point)> getPoints() const override {
            std::vector <Handle(Point)> res;
            res.push_back(mMidlePoint);
            res.push_back(mLastPoint);
            return res;
        }

        Adaptor3d_CurveOnSurface getCurve(const TopoDS_Face &face,
                                          Handle(Point) &start) const override {
            auto first = start->get2dPnt();
            auto midle = mMidlePoint->get2dPnt();
            auto last = mLastPoint->get2dPnt();
            Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeArcOfCircle(first, midle, last);
#if OCC_VERSION_HEX > 0x070500
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_Curve(curve), new BRepAdaptor_Surface(face));
#else
            return Adaptor3d_CurveOnSurface(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(face));
#endif
        }

        Handle(Point) mMidlePoint;
        Handle(Point) mLastPoint;
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
            lastPoint = (*it++)->getPoints().back();
        }
    }

    void compute(const Handle(Prs3d_Presentation) &presentation, const Quantity_Color &selectionColor) {
        auto lastPoint = mFirstPoint;
        auto it = mCurves.cbegin();
        while (it != mCurves.cend()) {
            (*it)->aspect->SetColor(((*it)->owner && (*it)->owner->IsSelected()) ? selectionColor : mCurveColor);
            (*it)->Draw(presentation, mFace, lastPoint);
            lastPoint = (*it++)->getPoints().back();
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
            lastPoint = curve->getPoints().back();
        }
    }

    void clearSelected() {
        for (auto &curve : mCurves) {
            if (curve->owner) {
                curve->owner->SetSelected(Standard_False);
            }
        }
    }

    void addCurve(size_t index, const gp_Pnt &pnt) {
        auto it = mCurves.begin();
        while (it != mCurves.end()) {
            if (index > 0) {
                --index;
                ++it;
                continue;
            }

            auto curve = std::make_shared<Line>(mCurveColor, mCurveWidth, Point::createPoint(q, mFace, pnt));
            curve->SetZLayer(mFirstPoint->ZLayer());
            mCurves.insert(it, curve);
            break;
        }
    }

    void removeCurve(size_t index) {
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
            lastPnt = (*it)->getPoints().back();
            if (index > 0) {
                --index;
                ++it;
                continue;
            }

            it = mCurves.erase(it);

            auto arc = std::make_shared<ArcOfCircle>(mCurveColor, mCurveWidth,
                                                     Point::createPoint(q, mFace, pnt), lastPnt);
            arc->SetZLayer(mFirstPoint->ZLayer());
            mCurves.insert(it, arc);
            break;
        }
    }

    void updatePointsLabel() {
        mFirstPoint->setLabel("T1");
        Standard_Integer index = 1;
        for (auto &curve : mCurves) {
            for (auto &p : curve->getPoints()) {
                p->setLabel(TCollection_AsciiString("T").Cat(++index));
            }
        }
    }

    void notify() {
        for (auto o : mObservers) {
            o->handleChanged();
        }
    }

    InteractiveCurve *q;
    TopoDS_Face mFace;
    Handle(Point) mFirstPoint;
    std::list < std::shared_ptr<Curve> > mCurves;

    Standard_Real mCurveWidth = 2.;
    Quantity_Color mCurveColor = Quantity_NOC_RED;

    std::vector <InteractiveCurve::Observer *> mObservers;
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
    d->mFirstPoint = Point::createPoint(this, face, startOnFace);
    auto curve = std::make_shared <InteractiveCurvePrivate::Line>
            (d->mCurveColor, d->mCurveWidth, Point::createPoint(this, d->mFace, endOnFace));
    d->mCurves.push_back(curve);
    d->updatePointsLabel();
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
    if (GetContext()) {
        GetContext()->RecomputePrsOnly(this, Standard_True);
    }
}

TopoDS_Face InteractiveCurve::face() const
{
    return d->mFace;
}

size_t InteractiveCurve::curvesCount() const
{
    return d->mCurves.size();
}

size_t InteractiveCurve::normalsCount() const
{
    size_t res = 1;
    for (const auto &curve : d->mCurves) {
        res += curve->getPoints().size();
    }
    return res;
}

size_t InteractiveCurve::curveNormalsCount(size_t curveIndex) const
{
    if (curveIndex < d->mCurves.size()) {
        auto it = d->mCurves.cbegin();
        std::advance(it, curveIndex);
        return (*it)->getPoints().size();
    }
    return 0;
}

bool InteractiveCurve::getNormal(size_t index, gp_Pnt &pnt, gp_Quaternion &rotation) const
{
    auto point = d->mFirstPoint;
    auto itCurves = d->mCurves.cbegin();
    auto points = (*itCurves)->getPoints();
    auto itPoints = points.cbegin();
    while (index > 0) {
        ++itPoints;
        if (itPoints != points.cend()) {
            point = *itPoints;
        } else {
            ++itCurves;
            if (++itCurves == d->mCurves.cend()) {
                return false;
            }
            points = (*itCurves)->getPoints();
            itPoints = points.cbegin();
            if (itPoints == points.cend()) {
                return false;
            }
            point = *itPoints;
        }
    }

    pnt = point->getPnt();
    rotation = point->getRotation();
    return true;
}

bool InteractiveCurve::getNormalOnCurve(size_t curveIndex, size_t index, gp_Pnt &pnt, gp_Quaternion &rotation) const
{
    if (curveIndex < d->mCurves.size()) {
        auto it = d->mCurves.cbegin();
        std::advance(it, curveIndex);
        auto points = (*it)->getPoints();
        if (index < points.size()) {
            auto point = points[index];
            pnt = point->getPnt();
            rotation = point->getRotation();
            return true;
        }
        return false;
    }
    return false;
}

bool InteractiveCurve::getMinMaxUParameter(size_t curveIndex, Standard_Real &first, Standard_Real &last) const
{
    if (curveIndex < d->mCurves.size()) {
        auto it = d->mCurves.cbegin();
        auto firstPnt = d->mFirstPoint;
        if (curveIndex > 0) {
            std::advance(it, curveIndex - 1);
            firstPnt = (*it)->getPoints().back();
            ++it;
        }
        auto curve = (*it)->getCurve(d->mFace, firstPnt);
        first = curve.FirstParameter();
        last = curve.LastParameter();
        return true;
    }
    return false;
}

bool InteractiveCurve::getPointOnCurve(size_t curveIndex, Standard_Real U, gp_Pnt &point) const
{
    if (curveIndex < d->mCurves.size()) {
        auto it = d->mCurves.cbegin();
        auto firstPnt = d->mFirstPoint;
        if (curveIndex > 0) {
            std::advance(it, curveIndex - 1);
            firstPnt = (*it)->getPoints().back();
            ++it;
        }
        auto curve = (*it)->getCurve(d->mFace, firstPnt);
        if (curve.FirstParameter() <= U && U <= curve.LastParameter()) {
            point = curve.Value(U);
            return true;
        }
        return false;
    }
    return false;
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

void InteractiveCurve::addCurve(size_t curveIndex, const gp_Pnt &pnt)
{
    d->addCurve(curveIndex, pnt);
    d->updatePointsLabel();
    if (GetContext()) {
        GetContext()->Redisplay(this, Standard_True);
    }
    d->notify();
}

void InteractiveCurve::removeCurve(size_t curveIndex)
{
    if (curvesCount() == 1) {
        return;
    }

    d->removeCurve(curveIndex);
    d->updatePointsLabel();
    if (GetContext()) {
        GetContext()->Redisplay(this, Standard_True);
    }
    d->notify();
}

void InteractiveCurve::addArcOfCircle(size_t curveIndex, const gp_Pnt &pnt)
{
    d->addArcOfCircle(curveIndex, pnt);
    d->updatePointsLabel();
    if (GetContext()) {
        GetContext()->Redisplay(this, Standard_True);
    }
    d->notify();
}

QJsonObject InteractiveCurve::toJson() const
{
    QJsonArray arr;
    for (const auto &curve : d->mCurves) {
        QJsonObject obj;
        obj["type"] = curve->type();
        switch (curve->type()) {
            case InteractiveCurvePrivate::Curve::CT_Line: {
                auto line = std::static_pointer_cast <InteractiveCurvePrivate::Line> (curve);
                obj["lastPnt"] = InteractiveFaceNormalSerializer::serialize(line->mLastPoint);
                break;
            }
            case InteractiveCurvePrivate::Curve::CT_ArcOfCircle: {
                auto arcOfCircle = std::static_pointer_cast <InteractiveCurvePrivate::ArcOfCircle> (curve);
                obj["midlePnt"] = InteractiveFaceNormalSerializer::serialize(arcOfCircle->mMidlePoint);
                obj["lastPnt"] = InteractiveFaceNormalSerializer::serialize(arcOfCircle->mLastPoint);
                break;
            }
            default:
                return QJsonObject();
        }
        arr.append(obj);
    }

    QJsonObject obj;
    obj["fPnt"] = InteractiveFaceNormalSerializer::serialize(d->mFirstPoint);
    obj["curves"] = arr;
    return obj;
}

Handle(InteractiveCurve) InteractiveCurve::fromJson(const QJsonObject &obj)
{
    Handle(InteractiveCurve) res = new InteractiveCurve();
    res->d->mFirstPoint = Point::createPoint(res, obj["fPnt"].toObject());
    if (!res->d->mFirstPoint) {
        return nullptr;
    }

    res->d->mFace = res->d->mFirstPoint->face();
    auto array = obj["curves"].toArray();
    for (const auto &value : qAsConst(array)) {
        QJsonObject curveObj = value.toObject();
        auto type = static_cast <InteractiveCurvePrivate::Curve::CurveTypes> (curveObj["type"].toInt());
        std::shared_ptr <InteractiveCurvePrivate::Curve> curve;
        switch (type) {
            case InteractiveCurvePrivate::Curve::CT_Line: {
                auto lastPnt = Point::createPoint(res, curveObj["lastPnt"].toObject());
                if (lastPnt) {
                    curve = std::make_shared <InteractiveCurvePrivate::Line>
                            (res->d->mCurveColor, res->d->mCurveWidth, lastPnt);
                }
                break;
            }
            case InteractiveCurvePrivate::Curve::CT_ArcOfCircle: {
                auto midlePnt = Point::createPoint(res, curveObj["midlePnt"].toObject());
                auto lastPnt = Point::createPoint(res, curveObj["lastPnt"].toObject());
                if (midlePnt && lastPnt) {
                    curve = std::make_shared <InteractiveCurvePrivate::ArcOfCircle>
                            (res->d->mCurveColor, res->d->mCurveWidth, midlePnt, lastPnt);
                }
                break;
            }
            default:
                break;
        }
        if (!curve) {
            return nullptr;
        }
        res->d->mCurves.push_back(curve);
    }

    if (res->d->mCurves.empty()) {
        return nullptr;
    }

    return res;
}

void InteractiveCurve::addObserver(Observer *observer)
{
    d->mObservers.push_back(observer);
}

void InteractiveCurve::removeObserver(Observer *observer)
{
    std::remove(d->mObservers.begin(), d->mObservers.end(), observer);
}

void InteractiveCurve::notify()
{
    d->notify();
}

InteractiveCurve::InteractiveCurve()
    : AIS_InteractiveObject()
    , d(new InteractiveCurvePrivate)
{
    SetInfiniteState(Standard_False);
    SetMutable(Standard_True);
    SetAutoHilight(Standard_False);

    d->q = this;
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

}
