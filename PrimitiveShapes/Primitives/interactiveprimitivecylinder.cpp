#include "interactiveprimitivecylinder.h"

#include <BRepPrimAPI_MakeCylinder.hxx>

#include "interactivedimentionlenght.h"

class InteractivePrimitiveCylinderPrivate
{
    friend class InteractivePrimitiveCylinder;

    TopoDS_Shape makeShape() const {
        return BRepPrimAPI_MakeCylinder(mRadius, mHeight);
    }


    std::vector <Handle(InteractiveDimentionLenght)> dimentions() const {
        return { mRadiusDimension, mHeightDimension };
    }

    Bnd_Box boundingBox() const {
        return Bnd_Box(gp_Pnt(-mRadius, -mRadius, 0.), gp_Pnt(mRadius, mRadius, mHeight));
    }

    void updateDimensions() {
        auto shape = makeShape();
        mRadiusDimension->SetMeasuredGeometry(gp_Pnt(0., 0., 0.),
                                              gp_Pnt(mRadius, 0., 0.),
                                              gp_Pln(gp_Pnt(0., 0., 0.), gp_Dir(0., 1., 0.)));
        mHeightDimension->SetMeasuredGeometry(gp_Pnt(mRadius, 0., 0.),
                                              gp_Pnt(mRadius, 0., mHeight),
                                              gp_Pln(gp_Pnt(mRadius, 0., 0.), gp_Dir(0., 1., 0.)));
    }

    void createDimensions() {
        auto shape = makeShape();
        mRadiusDimension = new InteractiveDimentionLenght(gp_Pnt(0., 0., 0.),
                                                          gp_Pnt(mRadius, 0., 0.),
                                                          gp_Pln(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.)));
        mHeightDimension = new InteractiveDimentionLenght(gp_Pnt(mRadius, 0., 0.),
                                                          gp_Pnt(mRadius, 0., mHeight),
                                                          gp_Pln(gp_Pnt(mRadius, 0., 0.), gp_Dir(0., 1., 0.)));
    }

    InteractivePrimitiveCylinder *q;
    Standard_Real mRadius = 100.;
    Standard_Real mHeight = 300.;
    Standard_Real mMinLenght = 1.;
    Handle(InteractiveDimentionLenght) mRadiusDimension, mHeightDimension;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractivePrimitiveCylinder, InteractivePrimitive)

InteractivePrimitiveCylinder::InteractivePrimitiveCylinder()
    : InteractivePrimitive()
    , d(new InteractivePrimitiveCylinderPrivate)
{
    d->q = this;

    d->createDimensions();
}

InteractivePrimitiveCylinder::~InteractivePrimitiveCylinder()
{
    delete d;
}

Standard_Real InteractivePrimitiveCylinder::getRadius() const
{
    return d->mRadius;
}

Standard_Real InteractivePrimitiveCylinder::getHeight() const
{
    return d->mHeight;
}

void InteractivePrimitiveCylinder::setRadius(Standard_Real width)
{
    d->mRadius = width < d->mMinLenght ? d->mMinLenght : width;
    updateGeometry();
}

void InteractivePrimitiveCylinder::setHeight(Standard_Real height)
{
    d->mHeight = height < d->mMinLenght ? d->mMinLenght : height;
    updateGeometry();
}

void InteractivePrimitiveCylinder::handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value)
{
    if (dimension == d->mRadiusDimension) {
        setRadius(value);
    } else if (dimension == d->mHeightDimension) {
        setHeight(value);
    }
}

TopoDS_Shape InteractivePrimitiveCylinder::createShape() const
{
    return d->makeShape();
}

Bnd_Box InteractivePrimitiveCylinder::createBoundingBox() const
{
    return d->boundingBox();
}

void InteractivePrimitiveCylinder::updateDimensions()
{
    d->updateDimensions();
}

std::vector<Handle(InteractiveDimentionLenght)> InteractivePrimitiveCylinder::dimentions() const
{
    return d->dimentions();
}
