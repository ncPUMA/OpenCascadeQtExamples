#include "interactivesphere.h"

#include <BRepPrimAPI_MakeSphere.hxx>

namespace ExamplesBase {

class InteractiveSpherePrivate
{
    friend class InteractiveSphere;

    TopoDS_Shape sphere() const {
        return BRepPrimAPI_MakeSphere(radius);
    }

    Standard_Real radius = 100.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSphere, InteractiveObject)

InteractiveSphere::InteractiveSphere()
    : InteractiveObject()
    , d(new InteractiveSpherePrivate)
{
    updateShape(d->sphere());
}

InteractiveSphere::~InteractiveSphere()
{
    delete d;
}

Standard_Real InteractiveSphere::getRadius() const
{
    return d->radius;
}

void InteractiveSphere::setRadius(Standard_Real radius)
{
    if (radius > 0.) {
        d->radius = radius;
        updateShape(d->sphere());
    }
}

}
