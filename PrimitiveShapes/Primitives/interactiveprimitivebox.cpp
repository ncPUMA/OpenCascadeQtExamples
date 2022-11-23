#include "interactiveprimitivebox.h"

#include <BRepPrimAPI_MakeBox.hxx>

#include <ExamplesBase/utility.h>

#include "interactivedimentionlenght.h"

class InteractivePrimitiveBoxPrivate
{
    friend class InteractivePrimitiveBox;

    TopoDS_Shape makeShape() const {
        return BRepPrimAPI_MakeBox(mWidth, mHeight, mDepth);
    }

    std::vector <Handle(InteractiveDimentionLenght)> dimentions() const {
        return { mXDimension, mYDimension, mZDimension };
    }

    Bnd_Box boundingBox() const {
        return Bnd_Box(gp_Pnt(), gp_Pnt(mWidth, mHeight, mDepth));
    }

    void updateDimensions() {
        auto shape = makeShape();
        mXDimension->SetMeasuredGeometry(ExamplesBase::findFaceByPoint(shape, gp_Pnt(0., mHeight / 2., mDepth / 2.)),
                                         ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth, mHeight / 2., mDepth / 2.)));
        mYDimension->SetMeasuredGeometry(ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., 0., mDepth / 2.)),
                                         ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight, mDepth / 2.)));
        mZDimension->SetMeasuredGeometry(ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight / 2., 0.)),
                                         ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight / 2., mDepth)));
    }

    void createDimensions() {
        auto shape = makeShape();
        mXDimension = new InteractiveDimentionLenght(ExamplesBase::findFaceByPoint(shape, gp_Pnt(0., mHeight / 2., mDepth / 2.)),
                                                     ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth, mHeight / 2., mDepth / 2.)));
        mYDimension = new InteractiveDimentionLenght(ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., 0., mDepth / 2.)),
                                                     ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight, mDepth / 2.)));
        mYDimension->SetFlyout(-15.);
        mZDimension = new InteractiveDimentionLenght(ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight / 2., 0.)),
                                                     ExamplesBase::findFaceByPoint(shape, gp_Pnt(mWidth / 2., mHeight / 2., mDepth)));
        mZDimension->SetFlyout(-15.);
    }

    InteractivePrimitiveBox *q;
    Standard_Real mWidth = 100.;
    Standard_Real mHeight = 200.;
    Standard_Real mDepth = 300.;
    Standard_Real mMinLenght = 1.;
    Handle(InteractiveDimentionLenght) mXDimension, mYDimension, mZDimension;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractivePrimitiveBox, InteractivePrimitive)

InteractivePrimitiveBox::InteractivePrimitiveBox()
    : InteractivePrimitive()
    , d(new InteractivePrimitiveBoxPrivate)
{
    d->q = this;

    d->createDimensions();
}

InteractivePrimitiveBox::~InteractivePrimitiveBox()
{
    delete d;
}

Standard_Real InteractivePrimitiveBox::getWidth() const
{
    return d->mWidth;
}

Standard_Real InteractivePrimitiveBox::getHeight() const
{
    return d->mHeight;
}

Standard_Real InteractivePrimitiveBox::getDepth() const
{
    return d->mDepth;
}

void InteractivePrimitiveBox::setWidth(Standard_Real width)
{
    d->mWidth = width < d->mMinLenght ? d->mMinLenght : width;
    updateGeometry();
}

void InteractivePrimitiveBox::setHeight(Standard_Real height)
{
    d->mHeight = height < d->mMinLenght ? d->mMinLenght : height;
    updateGeometry();
}

void InteractivePrimitiveBox::setDepth(Standard_Real depth)
{
    d->mDepth = depth < d->mMinLenght ? d->mMinLenght : depth;
    updateGeometry();
}

void InteractivePrimitiveBox::handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value)
{
    if (dimension == d->mXDimension) {
        setWidth(value);
    } else if (dimension == d->mYDimension) {
        setHeight(value);
    } else if (dimension == d->mZDimension) {
        setDepth(value);
    }
}

TopoDS_Shape InteractivePrimitiveBox::createShape() const
{
    return d->makeShape();
}

Bnd_Box InteractivePrimitiveBox::createBoundingBox() const
{
    return d->boundingBox();
}

void InteractivePrimitiveBox::updateDimensions()
{
    d->updateDimensions();
}

std::vector<Handle(InteractiveDimentionLenght)> InteractivePrimitiveBox::dimentions() const
{
    return d->dimentions();
}
