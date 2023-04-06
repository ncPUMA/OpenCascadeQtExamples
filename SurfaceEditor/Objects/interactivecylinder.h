#ifndef INTERACTIVECYLINDER_H
#define INTERACTIVECYLINDER_H

#include "interactiveobject.h"

class InteractiveCylinderPrivate;

class InteractiveCylinder : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveCylinder, InteractiveObject)

public:
    InteractiveCylinder();
    ~InteractiveCylinder();

    Standard_Real getRadius() const;
    Standard_Real getLenght() const;

    void setRadius(Standard_Real radius);
    void setLenght(Standard_Real lenght);

private:
    InteractiveCylinderPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveCylinder, InteractiveObject)

#endif // INTERACTIVECYLINDER_H
