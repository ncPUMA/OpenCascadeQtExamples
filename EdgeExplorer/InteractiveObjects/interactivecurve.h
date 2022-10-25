#ifndef INTERACTIVECURVE_H
#define INTERACTIVECURVE_H

#include <AIS_InteractiveObject.hxx>

class InteractiveCurvePrivate;
class TopoDS_Face;

class InteractiveCurve : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveCurve, AIS_InteractiveObject)
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

    size_t curveCount() const;

    bool isCurvePicked(const Handle(SelectMgr_EntityOwner) &entity, size_t &index) const;
    bool isPointPicked(const Handle(SelectMgr_EntityOwner) &entity,
                       size_t &curveIndex, size_t &pointIndex) const;

    void addPoint(size_t curveIndex, const gp_Pnt &pnt);
    void removePoint(size_t curveIndex);

    void addArcOfCircle(size_t curveIndex, const gp_Pnt &pnt);

protected:
    void Compute(const Handle(PrsMgr_PresentationManager) &,
                 const Handle(Prs3d_Presentation) &presentation,
                 const Standard_Integer theMode) Standard_OVERRIDE;

private:
    InteractiveCurvePrivate *const d;
};

DEFINE_STANDARD_HANDLE(InteractiveCurve, AIS_InteractiveObject)

#endif // INTERACTIVECURVE_H
