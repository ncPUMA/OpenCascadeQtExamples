/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveprimitivecone.h"

#include <BRepPrimAPI_MakeCone.hxx>

#include "interactivedimentionlenght.h"

class InteractivePrimitiveConePrivate
{
    friend class InteractivePrimitiveCone;

    TopoDS_Shape makeShape() const {
        return BRepPrimAPI_MakeCone(mRadiusNear, mRadiusFar, mHeight);
    }


    std::vector <Handle(InteractiveDimentionLenght)> dimentions() const {
        return { mRadiusNearDimension, mRadiusFarDimension, mHeightDimension };
    }

    Bnd_Box boundingBox() const {
        auto maxRadius = (mRadiusNear < mRadiusFar) ? mRadiusFar : mRadiusNear;
        return Bnd_Box(gp_Pnt(-maxRadius, -maxRadius, 0.), gp_Pnt(maxRadius, maxRadius, mHeight));
    }

    void updateDimensions() {
        mRadiusNearDimension->SetMeasuredGeometry(gp_Pnt(0., 0., 0.),
                                                  gp_Pnt(mRadiusNear, 0., 0.),
                                                  gp_Pln(gp_Pnt(0., 0., 0.), gp_Dir(0., 1., 0.)));
        mRadiusFarDimension->SetMeasuredGeometry(gp_Pnt(0., 0., mHeight),
                                                 gp_Pnt(mRadiusFar, 0., mHeight),
                                                 gp_Pln(gp_Pnt(0., 0., mHeight), gp_Dir(0., -1., 0.)));
        auto maxRadius = (mRadiusNear < mRadiusFar) ? mRadiusFar : mRadiusNear;
        mHeightDimension->SetMeasuredGeometry(gp_Pnt(maxRadius, 0., 0.),
                                              gp_Pnt(maxRadius, 0., mHeight),
                                              gp_Pln(gp_Pnt(maxRadius, 0., 0.), gp_Dir(0., 1., 0.)));
    }

    void createDimensions() {
        mRadiusNearDimension = new InteractiveDimentionLenght(gp_Pnt(0., 0., 0.),
                                                              gp_Pnt(mRadiusNear, 0., 0.),
                                                              gp_Pln(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.)));
        mRadiusFarDimension = new InteractiveDimentionLenght(gp_Pnt(0., 0., mHeight),
                                                             gp_Pnt(mRadiusFar, 0., mHeight),
                                                             gp_Pln(gp_Pnt(0., 0., mHeight), gp_Dir(0., -1., 0.)));
        auto maxRadius = (mRadiusNear < mRadiusFar) ? mRadiusFar : mRadiusNear;
        mHeightDimension = new InteractiveDimentionLenght(gp_Pnt(maxRadius, 0., 0.),
                                                          gp_Pnt(maxRadius, 0., mHeight),
                                                          gp_Pln(gp_Pnt(maxRadius, 0., 0.), gp_Dir(0., 1., 0.)));
    }

    InteractivePrimitiveCone *q;
    Standard_Real mRadiusNear = 100.;
    Standard_Real mRadiusFar = 200.;
    Standard_Real mHeight = 300.;
    Standard_Real mMinLenght = 1.;
    Handle(InteractiveDimentionLenght) mRadiusNearDimension, mRadiusFarDimension, mHeightDimension;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractivePrimitiveCone, InteractivePrimitive)

InteractivePrimitiveCone::InteractivePrimitiveCone()
    : InteractivePrimitive()
    , d(new InteractivePrimitiveConePrivate)
{
    d->q = this;

    d->createDimensions();
}

InteractivePrimitiveCone::~InteractivePrimitiveCone()
{
    delete d;
}

Standard_Real InteractivePrimitiveCone::getRadiusNear() const
{
    return d->mRadiusNear;
}

Standard_Real InteractivePrimitiveCone::getRadiusFar() const
{
    return d->mRadiusFar;
}

Standard_Real InteractivePrimitiveCone::getHeight() const
{
    return d->mHeight;
}

void InteractivePrimitiveCone::setRadiusNear(Standard_Real radius)
{
    d->mRadiusNear = (d->mRadiusFar == 0 && radius < d->mMinLenght) ? d->mMinLenght : radius;
    updateGeometry();
}

void InteractivePrimitiveCone::setRadiusFar(Standard_Real radius)
{
    d->mRadiusFar = (d->mRadiusNear == 0 && radius < d->mMinLenght) ? d->mMinLenght : radius;
    updateGeometry();
}

void InteractivePrimitiveCone::setHeight(Standard_Real height)
{
    d->mHeight = height < d->mMinLenght ? d->mMinLenght : height;
    updateGeometry();
}

void InteractivePrimitiveCone::handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value)
{
    if (dimension == d->mRadiusNearDimension) {
        setRadiusNear(value);
    } else if (dimension == d->mRadiusFarDimension) {
        setRadiusFar(value);
    } else if (dimension == d->mHeightDimension) {
        setHeight(value);
    }
}

TopoDS_Shape InteractivePrimitiveCone::createShape() const
{
    return d->makeShape();
}

Bnd_Box InteractivePrimitiveCone::createBoundingBox() const
{
    return d->boundingBox();
}

void InteractivePrimitiveCone::updateDimensions()
{
    d->updateDimensions();
}

std::vector<Handle(InteractiveDimentionLenght)> InteractivePrimitiveCone::dimentions() const
{
    return d->dimentions();
}
