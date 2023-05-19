#include "interactivecylinder.h"

#include <BRepPrimAPI_MakeCylinder.hxx>

namespace ExamplesBase {

class InteractiveCylinderPrivate
{
    friend class InteractiveCylinder;

    TopoDS_Shape cylinder() const {
        return BRepPrimAPI_MakeCylinder(radius, lenght);
    }

    Standard_Real radius = 100.;
    Standard_Real lenght = 200.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveCylinder, InteractiveObject)

InteractiveCylinder::InteractiveCylinder()
    : InteractiveObject()
    , d(new InteractiveCylinderPrivate)
{
    updateShape(d->cylinder());
}

InteractiveCylinder::~InteractiveCylinder()
{
    delete d;
}

Standard_Real InteractiveCylinder::getRadius() const
{
    return d->radius;
}

Standard_Real InteractiveCylinder::getLenght() const
{
    return d->lenght;
}

void InteractiveCylinder::setRadius(Standard_Real radius)
{
    if (radius > 0.) {
        d->radius = radius;
        updateShape(d->cylinder());
    }
}

void InteractiveCylinder::setLenght(Standard_Real lenght)
{
    if (lenght > 0.) {
        d->lenght = lenght;
        updateShape(d->cylinder());
    }
}

}
