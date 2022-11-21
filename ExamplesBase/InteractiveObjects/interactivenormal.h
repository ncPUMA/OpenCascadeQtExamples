#ifndef EB_INTERACTIVENORMAL_H
#define EB_INTERACTIVENORMAL_H

#include "../ExamplesBase_global.h"

#include <AIS_InteractiveObject.hxx>

namespace ExamplesBase {

class InteractiveNormalPrivate;

//! Create a selectable and movable normal presentation
class InteractiveNormal : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveNormal, AIS_InteractiveObject)
public:

    //! constract object with empty normal, i.e. pos is (0., 0., 0.)
    //! and empty rotation equal OZ direction
    InteractiveNormal();

    //! constract object
    //! @param pos - local position
    //! @param normal - local direction normal, i.e. XOY direction. Direction by OZ is 0.
    InteractiveNormal(const gp_Pnt &pos, const gp_Dir &normal);
    ~InteractiveNormal();

    //! set normal in local coordinates
    void setNormal(const gp_Trsf &normal);

    //! returns normal in local coordinates
    gp_Trsf getNormal() const;

    //! add text label to corner point
    void setLabel(const TCollection_AsciiString &txt);

    //! map local trsf to world coordinates
    gp_Trsf mapToGlobal(const gp_Trsf &localTrsf) const;
    //! map global trsf to local coordinates
    gp_Trsf mapToLocal(const gp_Trsf &globalTrsf) const;

protected:
    void HilightSelected(const Handle(PrsMgr_PresentationManager) &thePM,
                         const SelectMgr_SequenceOfOwner &) Standard_OVERRIDE;
    void ClearSelected() Standard_OVERRIDE;
    void HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
                               const Handle(Prs3d_Drawer) &theStyle,
                               const Handle(SelectMgr_EntityOwner) &theOwner) Standard_OVERRIDE;

    void ComputeSelection(const Handle(SelectMgr_Selection) &theSelection,
                          const Standard_Integer theMode) Standard_OVERRIDE;

    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                     const Handle(V3d_View) &view,
                                     const Handle(SelectMgr_EntityOwner) &owner,
                                     const Graphic3d_Vec2i &from,
                                     const Graphic3d_Vec2i &to,
                                     const AIS_DragAction action) Standard_OVERRIDE;

    void Compute(const Handle(PrsMgr_PresentationManager) &,
                 const Handle(Prs3d_Presentation) &thePrs,
                 const Standard_Integer theMode) Standard_OVERRIDE;

private:
    InteractiveNormalPrivate *const d;
};

DEFINE_STANDARD_HANDLE(InteractiveNormal, AIS_InteractiveObject)

}

#endif // EB_INTERACTIVENORMAL_H
