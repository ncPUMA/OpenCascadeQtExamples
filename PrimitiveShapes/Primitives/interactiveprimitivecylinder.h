#ifndef INTERACTIVEPRIMITIVECYLINDER_H
#define INTERACTIVEPRIMITIVECYLINDER_H

#include "interactiveprimitive.h"

class InteractivePrimitiveCylinderPrivate;

class InteractivePrimitiveCylinder : public InteractivePrimitive
{
    DEFINE_STANDARD_RTTIEXT(InteractivePrimitiveCylinder, InteractivePrimitive)
public:
    InteractivePrimitiveCylinder();
    ~InteractivePrimitiveCylinder();

    Standard_Real getRadius() const;
    Standard_Real getHeight() const;

    void setRadius(Standard_Real width);
    void setHeight(Standard_Real height);

    void handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value) Standard_OVERRIDE;

protected:
    TopoDS_Shape createShape() const Standard_OVERRIDE;
    Bnd_Box createBoundingBox() const Standard_OVERRIDE;
    void updateDimensions() Standard_OVERRIDE;
    std::vector<Handle(InteractiveDimentionLenght)> dimentions() const Standard_OVERRIDE;

private:
    InteractivePrimitiveCylinderPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractivePrimitiveCylinder, InteractivePrimitive)

#endif // INTERACTIVEPRIMITIVECYLINDER_H
