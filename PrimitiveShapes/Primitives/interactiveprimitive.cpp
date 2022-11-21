#include "interactiveprimitive.h"

#include <algorithm>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>

class InteractivePrimitivePrivate
{
    friend class InteractivePrimitive;

    InteractivePrimitivePrivate() {
        mManip = new AIS_Manipulator;
        mManip->SetPart(0, AIS_MM_Scaling, Standard_False);
        mManip->SetPart(1, AIS_MM_Scaling, Standard_False);
        mManip->SetPart(2, AIS_MM_Scaling, Standard_False);
        mManip->SetModeActivationOnDetection(Standard_True);
    }

    Handle(AIS_Manipulator) mManip;
    std::vector <InteractivePrimitiveObserver *> mObservers;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractivePrimitive, AIS_Shape)

InteractivePrimitive::InteractivePrimitive()
    : AIS_Shape(TopoDS_Shape())
    , d(new InteractivePrimitivePrivate)
{

}

InteractivePrimitive::~InteractivePrimitive()
{
    delete d;
}

void InteractivePrimitive::setManipulatorVisible(Standard_Boolean visible)
{
    auto ctx = GetContext();
    if (!ctx) {
        return;
    }

    if (visible && !ctx->IsDisplayed(d->mManip)) {
        gp_Trsf combined;
        if (CombinedParentTransformation()) {
            combined = CombinedParentTransformation()->Trsf();
        }
        auto ax = gp_Ax2().Transformed(Transformation());
        auto bndBox = BoundingBox().Transformed(Transformation());
        ax.SetLocation(bndBox.CornerMin().XYZ() + (bndBox.CornerMax().XYZ() - bndBox.CornerMin().XYZ()) / 2.);
        d->mManip->SetPosition(ax);

        AIS_Manipulator::OptionsForAttach options;
        options.SetAdjustPosition(Standard_False);
        options.SetAdjustSize(Standard_False);
        options.SetEnableModes(Standard_True);
        d->mManip->Attach(this, options);
        ctx->Display(d->mManip, Standard_False);
        return;
    }

    if (!visible && ctx->IsDisplayed(d->mManip)) {
        d->mManip->Detach();
        ctx->Remove(d->mManip, Standard_False);
    }
}

void InteractivePrimitive::setAdvancedManipulatorsVisible(Standard_Boolean)
{

}

void InteractivePrimitive::handleDimentionLenght(InteractiveDimentionLenght *, Standard_Real)
{

}

void InteractivePrimitive::addObserver(InteractivePrimitiveObserver *observer)
{
    d->mObservers.push_back(observer);
}

void InteractivePrimitive::removeObserver(InteractivePrimitiveObserver *observer)
{
    d->mObservers.erase(std::remove(d->mObservers.begin(), d->mObservers.end(), observer), d->mObservers.end());
}

void InteractivePrimitive::notify()
{
    for (auto &o : d->mObservers) {
        o->primitiveChanged();
    }
}

void InteractivePrimitive::SetContext(const Handle(AIS_InteractiveContext) &context) {
    auto ctx = GetContext();
    if (!context && ctx && d->mManip && ctx->IsDisplayed(d->mManip)) {
        d->mManip->Detach();
        ctx->Remove(d->mManip, Standard_False);
    }
    AIS_Shape::SetContext(context);
}
