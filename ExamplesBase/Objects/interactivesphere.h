#ifndef INTERACTIVESPHERE_H
#define INTERACTIVESPHERE_H

#include "interactiveobject.h"

namespace ExamplesBase {

class InteractiveSpherePrivate;

class InteractiveSphere : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSphere, InteractiveObject)

public:
    InteractiveSphere();
    ~InteractiveSphere();

    Standard_Real getRadius() const;
    void setRadius(Standard_Real radius);

private:
    InteractiveSpherePrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSphere, InteractiveObject)

}

#endif // INTERACTIVESPHERE_H