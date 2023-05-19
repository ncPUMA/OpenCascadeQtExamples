#ifndef INTERACTIVEOBJECT_H
#define INTERACTIVEOBJECT_H

#include "AIS_Shape.hxx"

class QString;

namespace ExamplesBase {

class InteractiveObjectPrivate;

class InteractiveObjectObserver
{
public:
    InteractiveObjectObserver() = default;
    virtual ~InteractiveObjectObserver() = default;
    virtual void changed() = 0;

private:
    InteractiveObjectObserver(const InteractiveObjectObserver &) = delete;
    InteractiveObjectObserver& operator=(const InteractiveObjectObserver &) = delete;
};

class InteractiveObject : public AIS_Shape
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObject, AIS_Shape)

public:
    InteractiveObject();
    ~InteractiveObject();

    QString name() const;
    void setName(const QString &name);

    gp_Trsf getContextTransform() const;
    void setContextTransform(const gp_Trsf &trsf);

    void addObserver(InteractiveObjectObserver &observer);

protected:
    void notify();
    void updateShape(const TopoDS_Shape &shape);

private:
    InteractiveObjectPrivate *d;

private:
    InteractiveObject(const InteractiveObject &) = delete;
    InteractiveObject& operator=(InteractiveObject &) = delete;
};

DEFINE_STANDARD_HANDLE(InteractiveObject, AIS_Shape)

}

#endif // INTERACTIVEOBJECT_H
