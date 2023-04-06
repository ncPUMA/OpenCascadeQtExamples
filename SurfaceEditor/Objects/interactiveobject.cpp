#include "interactiveobject.h"

#include <QString>

#include <AIS_InteractiveContext.hxx>

class InteractiveObjectPrivate
{
    friend class InteractiveObject;

    QString name;
    std::vector<InteractiveObjectObserver *> observers;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObject, AIS_Shape)

InteractiveObject::InteractiveObject()
    : AIS_Shape(TopoDS_Shape())
    , d(new InteractiveObjectPrivate)
{

}

InteractiveObject::~InteractiveObject()
{
    delete d;
}

QString InteractiveObject::name() const
{
    return d->name;
}

void InteractiveObject::setName(const QString &name)
{
    d->name = name;
    notify();
}

gp_Trsf InteractiveObject::getContextTransform() const
{
    gp_Trsf trsf;
    auto ctx = GetContext();
    if (ctx) {
        trsf = ctx->Location(this).Transformation();
        auto parentTransform = CombinedParentTransformation();
        if (parentTransform) {
            trsf *= parentTransform->Transformation().Inverted();
        }
    }
    return trsf;
}

void InteractiveObject::setContextTransform(const gp_Trsf &trsf)
{
    auto ctx = GetContext();
    if (ctx) {
        gp_Trsf invertedParentTransform;
        auto parentTransform = CombinedParentTransformation();
        if (parentTransform) {
            invertedParentTransform = parentTransform->Transformation().Inverted();
        }
        ctx->SetLocation(this, trsf * invertedParentTransform);
        ctx->Redisplay(this, Standard_True, Standard_True);
        notify();
    }
}

void InteractiveObject::addObserver(InteractiveObjectObserver &observer)
{
    d->observers.push_back(&observer);
}

void InteractiveObject::notify()
{
    for (auto &observer : d->observers) {
        observer->changed();
    }
}

void InteractiveObject::updateShape(const TopoDS_Shape &shape)
{
    SetShape(shape);
    auto ctx = GetContext();
    if (ctx) {
        ctx->Redisplay(this, Standard_True, Standard_True);
    }
    notify();
}
