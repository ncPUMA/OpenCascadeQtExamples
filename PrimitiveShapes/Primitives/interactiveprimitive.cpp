/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveprimitive.h"

#include <algorithm>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>

#include "Primitives/interactivedimentionlenght.h"

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

    Bnd_Box mBox;
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

void InteractivePrimitive::setAdvancedManipulatorsVisible(Standard_Boolean visible)
{
    auto ctx = GetContext();
    if (!ctx) {
        return;
    }

    if (visible) {
        for (const auto &dimension : dimentions()) {
            if (!ctx->IsDisplayed(dimension)) {
                dimension->SetZLayer(ZLayer());
                AddChild(dimension);
                ctx->Display(dimension, Standard_False);
                ctx->SetSelectionSensitivity(dimension, PrsDim_DimensionSelectionMode_All, 10);
            }
        }
        return;
    }

    if (!visible) {
        for (const auto &dimension : dimentions()) {
            if (ctx->IsDisplayed(dimension)) {
                ctx->Remove(dimension, Standard_False);
                RemoveChild(dimension);
            }
        }
    }
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

const Bnd_Box &InteractivePrimitive::BoundingBox()
{
    return d->mBox;
}

void InteractivePrimitive::updateGeometry()
{
    AIS_Shape::SetShape(createShape());
    d->mBox = createBoundingBox();
    updateDimensions();
    auto ctx = GetContext();
    if (ctx) {
        ctx->Redisplay(this, Standard_False);
        for (const auto &dimension : dimentions()) {
            ctx->RecomputePrsOnly(dimension, Standard_False);
        }
    }

    for (auto &o : d->mObservers) {
        o->primitiveChanged();
    }
}

void InteractivePrimitive::SetContext(const Handle(AIS_InteractiveContext) &context) {
    if (context) {
        updateGeometry();
    }

    auto ctx = GetContext();
    if (!context && ctx && d->mManip && ctx->IsDisplayed(d->mManip)) {
        d->mManip->Detach();
        ctx->Remove(d->mManip, Standard_False);
    }
    AIS_Shape::SetContext(context);
}
