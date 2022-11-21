#ifndef INTERACTIVEPRIMITIVEBOX_H
#define INTERACTIVEPRIMITIVEBOX_H

#include "interactiveprimitive.h"

class InteractivePrimitiveBoxPrivate;

class InteractivePrimitiveBox : public InteractivePrimitive
{
    DEFINE_STANDARD_RTTIEXT(InteractivePrimitiveBox, InteractivePrimitive)
public:
    InteractivePrimitiveBox();
    ~InteractivePrimitiveBox();

    Standard_Real getWidth() const;
    Standard_Real getHeight() const;
    Standard_Real getDepth() const;

    void setWidth(Standard_Real width);
    void setHeight(Standard_Real height);
    void setDepth(Standard_Real depth);

    void setAdvancedManipulatorsVisible(Standard_Boolean visible) Standard_OVERRIDE;

    void handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value) Standard_OVERRIDE;

    const Bnd_Box& BoundingBox() Standard_OVERRIDE;

private:
    InteractivePrimitiveBoxPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractivePrimitiveBox, InteractivePrimitive)

#endif // INTERACTIVEPRIMITIVEBOX_H
