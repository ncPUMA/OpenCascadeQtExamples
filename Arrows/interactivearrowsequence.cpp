/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivearrowsequence.h"

#include <Graphic3d_ArrayOfSegments.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DatumAspect.hxx>

class InteractiveArrowSequencePrivate
{
    friend class InteractiveArrowSequence;

    std::vector<std::pair<gp_Pnt, gp_Pnt> > points;
    Handle(Graphic3d_AspectFillArea3d) groupAspect;
    Standard_Real arrowRadius = 1.;
    Standard_Real arrowLenght = 5.;
    Standard_Real arrowPos = 1.;
    InteractiveArrowSequence::ArrowMode arrowMode = InteractiveArrowSequence::AM_DrawAll;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveArrowSequence, AIS_InteractiveObject)

InteractiveArrowSequence::InteractiveArrowSequence()
    : AIS_InteractiveObject()
    , d(new InteractiveArrowSequencePrivate)
{
    d->groupAspect = new Graphic3d_AspectFillArea3d(Aspect_IS_SOLID,
                                                    Quantity_NOC_WHITE,
                                                    Quantity_NOC_BLACK,
                                                    Aspect_TOL_SOLID,
                                                    1.5,
                                                    Graphic3d_NameOfMaterial_DEFAULT,
                                                    Graphic3d_NameOfMaterial_DEFAULT);
    d->groupAspect->SetLineWidth(1.5);
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
    d->groupAspect->SetLineType(type);
}

void InteractiveArrowSequence::setLineWidh(Standard_Real width)
{
    d->groupAspect->SetLineWidth(width);
}

void InteractiveArrowSequence::setArrowRadius(Standard_Real radius)
{
    d->arrowRadius = radius;
}

void InteractiveArrowSequence::setArrowLenght(Standard_Real lenght)
{
    d->arrowLenght = lenght;
}

void InteractiveArrowSequence::setArrowPosition(Standard_Real pos)
{
    if (0. <= pos && pos <= 1.) {
        d->arrowPos = pos;
    }
}

void InteractiveArrowSequence::setArrowMode(InteractiveArrowSequence::ArrowMode mode)
{
    d->arrowMode = mode;
}

void InteractiveArrowSequence::Compute(const Handle(PrsMgr_PresentationManager) &prsMgr,
                                       const Handle(Prs3d_Presentation) &prs,
                                       const Standard_Integer mode)
{
    if (mode != DM_Default) {
        return;
    }

    auto groupAspect = d->groupAspect;
    groupAspect->SetColor(myDrawer->Color());
    auto material = groupAspect->FrontMaterial();
    material.SetColor(myDrawer->Color());
    material.SetTransparency(myDrawer->Transparency());
    groupAspect->SetFrontMaterial(material);

    Handle(Graphic3d_Group) lineGroup = prs->NewGroup();
    lineGroup->SetGroupPrimitivesAspect(groupAspect);
    Handle(Graphic3d_ArrayOfPrimitives) line =
            new Graphic3d_ArrayOfSegments(static_cast<Standard_Integer>(d->points.size()) * 2.);
    for (const auto &pair : d->points) {
        line->AddVertex(pair.first);
        line->AddVertex(pair.second);
    }
    lineGroup->AddPrimitiveArray(line);

    auto datumAspect = myDrawer->DatumAspect();
    auto nbOfFacettes = static_cast<Standard_Integer>(datumAspect->Attribute(Prs3d_DP_ShadingNumberOfFacettes));
    Handle(Graphic3d_Group) arrowGroup = prs->NewGroup();
    arrowGroup->SetGroupPrimitivesAspect(groupAspect);
    auto it = d->points.cend();
    auto end = d->points.cend();
    switch (d->arrowMode) {
        case AM_Disable:
            break;
        case AM_DrawAll:
            it = d->points.cbegin();
            break;
        case AM_DrawLast:
            it = d->points.cbegin();
            if (d->points.size() > 1) {
                std::advance(it, d->points.size() - 1);
            }
            break;
        case AM_DrawFirst:
            it = d->points.cbegin();
            end = d->points.cbegin();
            if (d->points.size() > 0) {
                ++end;
            }
            break;
    }
    for (; it != end; ++it) {
        const auto &pair = *it;
        auto axisLenght = pair.first.Distance(pair.second);
        Handle(Graphic3d_ArrayOfPrimitives) arrow =
                Prs3d_Arrow::DrawShaded(gp_Ax1(pair.first, gp_Vec(pair.first, pair.second)),
                                        0.0,
                                        axisLenght * d->arrowPos,
                                        d->arrowRadius,
                                        d->arrowLenght,
                                        nbOfFacettes);
        arrowGroup->AddPrimitiveArray(arrow);
    }
}
