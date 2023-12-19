/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    void setAdvancedManipulatorsVisible(Standard_Boolean visible);
    virtual void handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value);

    void addObserver(InteractivePrimitiveObserver *observer);
    void removeObserver(InteractivePrimitiveObserver *observer);

    const Bnd_Box& BoundingBox() Standard_OVERRIDE;

protected:
    void setBoundingBox(const Bnd_Box &box);

    void SetContext(const Handle(AIS_InteractiveContext) &context) Standard_OVERRIDE;
    void updateGeometry();

    virtual TopoDS_Shape createShape() const = 0;
    virtual Bnd_Box createBoundingBox() const = 0;
    virtual void updateDimensions() = 0;
    virtual std::vector<Handle(InteractiveDimentionLenght)> dimentions() const = 0;

private:
    InteractivePrimitivePrivate *d;
    friend class InteractivePrimitivePrivate;

private:
    InteractivePrimitive(const InteractivePrimitive &) = delete;
    InteractivePrimitive& operator=(InteractivePrimitive &) = delete;
    void SetShape(const TopoDS_Shape) = delete;
};

namespace opencascade {
inline uint qHash(const Handle(InteractivePrimitive) &obj, uint seed = 0) {
    return qHash(obj.get(), seed);
}

}

DEFINE_STANDARD_HANDLE(InteractivePrimitive, AIS_Shape)

#endif // INTERACTIVEPRIMITIVE_H
