/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEPRIMITIVECONE_H
#define INTERACTIVEPRIMITIVECONE_H

#include "interactiveprimitive.h"

class InteractivePrimitiveConePrivate;

class InteractivePrimitiveCone : public InteractivePrimitive
{
    DEFINE_STANDARD_RTTIEXT(InteractivePrimitiveCone, InteractivePrimitive)
public:
    InteractivePrimitiveCone();
    ~InteractivePrimitiveCone();

    Standard_Real getRadiusNear() const;
    Standard_Real getRadiusFar() const;
    Standard_Real getHeight() const;

    void setRadiusNear(Standard_Real radius);
    void setRadiusFar(Standard_Real radius);
    void setHeight(Standard_Real height);

    void handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value) Standard_OVERRIDE;

protected:
    TopoDS_Shape createShape() const Standard_OVERRIDE;
    Bnd_Box createBoundingBox() const Standard_OVERRIDE;
    void updateDimensions() Standard_OVERRIDE;
    std::vector<Handle(InteractiveDimentionLenght)> dimentions() const Standard_OVERRIDE;

private:
    InteractivePrimitiveConePrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractivePrimitiveCone, InteractivePrimitive)

#endif // INTERACTIVEPRIMITIVECONE_H
