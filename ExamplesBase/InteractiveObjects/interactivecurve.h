#ifndef EB_INTERACTIVECURVE_H
#define EB_INTERACTIVECURVE_H

#include "../ExamplesBase_global.h"

#include <AIS_InteractiveObject.hxx>

class TopoDS_Face;
class QJsonObject;

namespace ExamplesBase {

class InteractiveCurvePrivate;

class EXAMPLESBASE_EXPORT InteractiveCurve : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveCurve, AIS_InteractiveObject)
public:
    class EXAMPLESBASE_EXPORT Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void handleChanged() = 0;
    };

public:
    InteractiveCurve(const TopoDS_Face &face, const gp_Pnt &startOnFace, const gp_Pnt &endOnFace);
    ~InteractiveCurve();

    void SetZLayer(const Graphic3d_ZLayerId layerId) Standard_OVERRIDE;
    void SetContext(const Handle(AIS_InteractiveContext) &context) Standard_OVERRIDE;

    void HilightSelected(const Handle(PrsMgr_PresentationManager) &,
                         const SelectMgr_SequenceOfOwner &) Standard_OVERRIDE;
    void ClearSelected() Standard_OVERRIDE;
    void ComputeSelection(const Handle(SelectMgr_Selection) &selection,
                          const Standard_Integer mode) Standard_OVERRIDE;
    void HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
                               const Handle(Prs3d_Drawer) &theStyle,
                               const Handle(SelectMgr_EntityOwner) &theOwner) Standard_OVERRIDE;

    TopoDS_Face face() const;

    size_t curvesCount() const;
    size_t normalsCount() const;
    size_t curveNormalsCount(size_t curveIndex) const;
    bool getNormal(size_t index, gp_Pnt &pnt, gp_Quaternion &rotation) const;
    bool getNormalOnCurve(size_t curveIndex, size_t index, gp_Pnt &pnt, gp_Quaternion &rotation) const;
    bool getMinMaxUParameter(size_t curveIndex, Standard_Real &first, Standard_Real &last) const;
    bool getUParameter(size_t curveIndex, const gp_Pnt &pnt, gp_Pnt &projection, Standard_Real &U) const;
    bool getPointOnCurve(size_t curveIndex, Standard_Real U, gp_Pnt &point, gp_Quaternion &rotation) const;

    bool isCurvePicked(const Handle(SelectMgr_EntityOwner) &entity, size_t &index) const;
    bool isPointPicked(const Handle(SelectMgr_EntityOwner) &entity,
                       size_t &curveIndex, size_t &pointIndex) const;

    void addCurve(size_t curveIndex, const gp_Pnt &pnt);
    void removeCurve(size_t curveIndex);

    void addArcOfCircle(size_t curveIndex, const gp_Pnt &pnt);
    void addArcOfEllipse(size_t curveIndex, const gp_Pnt &pnt);

    QJsonObject toJson() const;
    static Handle(InteractiveCurve) fromJson(const QJsonObject &obj);

    void addObserver(Observer *observer);
    void removeObserver(Observer *observer);
    void notify();

protected:
    void Compute(const Handle(PrsMgr_PresentationManager) &,
                 const Handle(Prs3d_Presentation) &presentation,
                 const Standard_Integer theMode) Standard_OVERRIDE;

    InteractiveCurve();

private:
    InteractiveCurvePrivate *const d;
};

DEFINE_STANDARD_HANDLE(InteractiveCurve, AIS_InteractiveObject)

}

#endif // EB_INTERACTIVECURVE_H
