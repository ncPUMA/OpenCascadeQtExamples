#ifndef INTERACTIVEARROWSEQUENCE_H
#define INTERACTIVEARROWSEQUENCE_H

#include <utility>
#include <vector>

#include <AIS_InteractiveObject.hxx>

class gp_Pnt;
class InteractiveArrowSequencePrivate;

class InteractiveArrowSequence : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveArrowSequence, AIS_InteractiveObject)
public:
    enum DisplayMode
    {
        DM_Default = 0,
    };

    enum ArrowMode
    {
        AM_Disable,
        AM_DrawAll,
        AM_DrawLast,
        AM_DrawFirst,
    };

    InteractiveArrowSequence();
    ~InteractiveArrowSequence();

    Standard_Boolean AcceptDisplayMode(const Standard_Integer mode) const Standard_OVERRIDE;

    void setPoints(const std::vector<std::pair<gp_Pnt, gp_Pnt> > &points);
    void setLineType(Aspect_TypeOfLine type);
    void setLineWidh(Standard_Real width);
    void setArrowRadius(Standard_Real radius);
    void setArrowLenght(Standard_Real lenght);
    //! Sets the arrow position on line (from 0. to 1.) in percent
    void setArrowPosition(Standard_Real pos);
    void setArrowMode(InteractiveArrowSequence::ArrowMode mode);

    void ComputeSelection(const Handle(SelectMgr_Selection) &,
                          const Standard_Integer) Standard_OVERRIDE { }

protected:
    void Compute(const Handle(PrsMgr_PresentationManager) &prsMgr,
                 const Handle(Prs3d_Presentation) &prs,
                 const Standard_Integer mode) Standard_OVERRIDE;

private:
    InteractiveArrowSequencePrivate *d;

    InteractiveArrowSequence(const InteractiveArrowSequence &) = delete;
    InteractiveArrowSequence& operator=(const InteractiveArrowSequence &) = delete;
};

DEFINE_STANDARD_HANDLE(InteractiveArrowSequence, AIS_InteractiveObject)

#endif // INTERACTIVEARROWSEQUENCE_H
