/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLOREDSHAPE_H
#define COLOREDSHAPE_H

#include <AIS_Shape.hxx>

class ColoredShapePrivate;
class TopoDS_Face;

class ColoredShape : public AIS_Shape
{
    DEFINE_STANDARD_RTTIEXT(ColoredShape, AIS_Shape)
public:
    ColoredShape(const TopoDS_Shape& shape);
    ~ColoredShape();

    void setFaceColor(const TopoDS_Face &face, const Quantity_Color &color);
    void resetColors();

protected:
    void Compute(const Handle(PrsMgr_PresentationManager3d) &prsMgr,
                 const Handle(Prs3d_Presentation) &prs,
                 const Standard_Integer mode) override;

private:
    ColoredShapePrivate *d;
};

DEFINE_STANDARD_HANDLE(ColoredShape, AIS_Shape)

#endif // COLOREDSHAPE_H
