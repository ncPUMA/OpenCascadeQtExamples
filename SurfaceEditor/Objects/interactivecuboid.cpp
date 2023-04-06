#include "interactivecuboid.h"

#include <BRepPrimAPI_MakeBox.hxx>

class InteractiveCuboidPrivate
{
    friend class InteractiveCuboid;

    TopoDS_Shape cuboid() const {
        return BRepPrimAPI_MakeBox(width, height, depth);
    }

    Standard_Real width = 100.;
    Standard_Real height = 150.;
    Standard_Real depth = 200.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveCuboid, InteractiveObject)

InteractiveCuboid::InteractiveCuboid()
    : InteractiveObject()
    , d(new InteractiveCuboidPrivate)
{
    updateShape(d->cuboid());
}

InteractiveCuboid::~InteractiveCuboid()
{
    delete d;
}

Standard_Real InteractiveCuboid::getWidth() const
{
    return d->width;
}

Standard_Real InteractiveCuboid::getHeight() const
{
    return d->height;
}

Standard_Real InteractiveCuboid::getDepth() const
{
    return d->depth;
}

void InteractiveCuboid::setWidth(Standard_Real width)
{
    if (width > 0.) {
        d->width = width;
        updateShape(d->cuboid());
    }
}

void InteractiveCuboid::setHeight(Standard_Real height)
{
    if (height > 0.) {
        d->height = height;
        updateShape(d->cuboid());
    }
}

void InteractiveCuboid::setDepth(Standard_Real depth)
{
    if (depth > 0.) {
        d->depth = depth;
        updateShape(d->cuboid());
    }
}
