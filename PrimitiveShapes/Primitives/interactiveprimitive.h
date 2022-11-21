#ifndef INTERACTIVEPRIMITIVE_H
#define INTERACTIVEPRIMITIVE_H

#include <AIS_Shape.hxx>

class InteractivePrimitiveObserver
{
public:
    virtual ~InteractivePrimitiveObserver() = default;
    virtual void primitiveChanged() = 0;
};

class InteractivePrimitivePrivate;
class InteractiveDimentionLenght;

class InteractivePrimitive : public AIS_Shape
{
    DEFINE_STANDARD_RTTIEXT(InteractivePrimitive, AIS_Shape)
public:
    InteractivePrimitive();
    ~InteractivePrimitive();

    void setManipulatorVisible(Standard_Boolean visible);
    virtual void setAdvancedManipulatorsVisible(Standard_Boolean visible);

    virtual void handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value);

    void addObserver(InteractivePrimitiveObserver *observer);
    void removeObserver(InteractivePrimitiveObserver *observer);

protected:
    void notify();

    void SetContext(const Handle(AIS_InteractiveContext) &context) Standard_OVERRIDE;

private:
    InteractivePrimitivePrivate *d;
    friend class InteractivePrimitivePrivate;
};

namespace opencascade {
inline uint qHash(const Handle(InteractivePrimitive) &obj, uint seed = 0) {
    return qHash(obj.get(), seed);
}

}

DEFINE_STANDARD_HANDLE(InteractivePrimitive, AIS_Shape)

#endif // INTERACTIVEPRIMITIVE_H
