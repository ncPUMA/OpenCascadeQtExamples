#include "interactivearrowsequence.h"

#include <Graphic3d_ArrayOfSegments.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DatumAspect.hxx>

class InteractiveArrowSequencePrivate
{
    friend class InteractiveArrowSequence;

    std::vector<std::pair<gp_Pnt, gp_Pnt> > points;
    Aspect_TypeOfLine lineType = Aspect_TOL_SOLID;
    Standard_Real lineWidth = 1.5;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveArrowSequence, AIS_InteractiveObject)

InteractiveArrowSequence::InteractiveArrowSequence()
    : AIS_InteractiveObject()
    , d(new InteractiveArrowSequencePrivate)
{
    SetDisplayMode(DM_Default);
}

InteractiveArrowSequence::~InteractiveArrowSequence()
{
    delete d;
}

Standard_Boolean InteractiveArrowSequence::AcceptDisplayMode(const Standard_Integer mode) const
{
    return mode == DM_Default;
}

void InteractiveArrowSequence::setPoints(const std::vector<std::pair<gp_Pnt, gp_Pnt> > &points)
{
    d->points = points;
}

void InteractiveArrowSequence::setLineType(Aspect_TypeOfLine type)
{
    d->lineType = type;
}

void InteractiveArrowSequence::setLineWidh(Standard_Real width)
{
    d->lineWidth = width;
}

void InteractiveArrowSequence::Compute(const Handle(PrsMgr_PresentationManager) &prsMgr,
                                       const Handle(Prs3d_Presentation) &prs,
                                       const Standard_Integer mode)
{
    if (mode != DM_Default) {
        return;
    }

    Handle(Graphic3d_AspectFillArea3d) groupAspect = new Graphic3d_AspectFillArea3d(*myDrawer->ShadingAspect()->Aspect());
    groupAspect->SetColor(myDrawer->Color());
    auto material = groupAspect->FrontMaterial();
    material.SetColor(myDrawer->Color());
    groupAspect->SetFrontMaterial(material);
    groupAspect->SetLineType(d->lineType);
    groupAspect->SetLineWidth(d->lineWidth);

    Handle(Graphic3d_Group) lineGroup = prs->NewGroup();
    lineGroup->SetGroupPrimitivesAspect(groupAspect);
    for (const auto &pair : d->points) {
        Handle(Graphic3d_ArrayOfPrimitives) line = new Graphic3d_ArrayOfSegments(2);
        line->AddVertex(pair.first);
        line->AddVertex(pair.second);
        lineGroup->AddPrimitiveArray(line);
    }

    auto datumAspect = myDrawer->DatumAspect();
    auto nbOfFacettes = static_cast<Standard_Integer>(datumAspect->Attribute(Prs3d_DP_ShadingNumberOfFacettes));
    Handle(Graphic3d_Group) arrowGroup = prs->NewGroup();
    arrowGroup->SetGroupPrimitivesAspect(groupAspect);
    for (const auto &pair : d->points) {
        auto axisLenght = pair.first.Distance(pair.second);
        Handle(Graphic3d_ArrayOfPrimitives) arrow =
                Prs3d_Arrow::DrawShaded(gp_Ax1(pair.first, gp_Vec(pair.first, pair.second)),
                                        0.0,
                                        axisLenght,
                                        1.,
                                        5.,
                                        nbOfFacettes);
        arrowGroup->AddPrimitiveArray(arrow);
    }
}
