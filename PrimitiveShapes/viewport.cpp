#include "viewport.h"

#include <QDebug>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>

#include "Primitives/interactiveprimitivebox.h"

class ViewportPrivate
{
    friend class Viewport;

    void addPrimitive(const Handle(InteractivePrimitive) &prim) {
        prim->SetZLayer(q_ptr->depthOffLayer());
        mModel->AddChild(prim);
        mPrimitives << prim;
        q_ptr->context()->Display(prim, Standard_False);
    }

    void removePrimitive(const Handle(InteractivePrimitive) &primitive) {
        auto it = std::find(mPrimitives.begin(), mPrimitives.end(), primitive);
        if (it != mPrimitives.cend()) {
            q_ptr->context()->Remove(*it, Standard_False);
            mPrimitives.erase(it);
        }
    }

    Viewport *q_ptr;
    Handle(AIS_Shape) mModel;

    QVector <Handle(InteractivePrimitive)> mPrimitives;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;
    auto ctx = context();
    Handle(AIS_Trihedron) trihedron;
    if (loadModel(QStringLiteral("../Models/tube_with_cuts.stl"), d_ptr->mModel, trihedron)) {
        gp_Trsf transform;
        transform.SetTranslationPart(gp_Vec(10, 20, 30));
        gp_Quaternion quat;
        quat.SetEulerAngles(gp_Extrinsic_XYZ, 10., 10., 10.);
        transform.SetRotationPart(quat);
        ctx->SetLocation(d_ptr->mModel, transform);
        ctx->SetLocation(trihedron, transform);

        ctx->SetSelectionModeActive(d_ptr->mModel,
                                    AIS_Shape::SelectionMode(TopAbs_SHAPE),
                                    Standard_False);
        ctx->SetSelectionModeActive(d_ptr->mModel,
                                    AIS_Shape::SelectionMode(TopAbs_EDGE),
                                    Standard_True,
                                    AIS_SelectionModesConcurrency_Single);
    }
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::selectionChanged()
{
    QSet <Handle(InteractivePrimitive)> selectedShapes;
    auto ctx = context();
    ctx->InitSelected();
    bool ret = false;
    while (ctx->MoreSelected())
    {
        auto primitive = Handle(InteractivePrimitive)::DownCast(ctx->SelectedInteractive());
        if (primitive) {
            ret = true;
            primitive->setManipulatorVisible(Standard_True);
            primitive->setAdvancedManipulatorsVisible(Standard_True);
        }
        selectedShapes << primitive;
        ctx->NextSelected ();
    }
    for (const auto &p : qAsConst(d_ptr->mPrimitives)) {
        if (!selectedShapes.contains(p)) {
            ret = true;
            p->setManipulatorVisible(Standard_False);
            p->setAdvancedManipulatorsVisible(Standard_False);
        }
    }
    return ret;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    bool ret = false;
    if (event->button() == Qt::RightButton) {
        auto ctx = context();
        QMenu menu;
        if (ctx->MainSelector()->NbPicked() == 0) {
            menu.addAction(tr("Make Box"), this, [this](){
                d_ptr->addPrimitive(new InteractivePrimitiveBox);
            });
        } else {
            ctx->InitDetected();
            QSet <Handle(InteractivePrimitive)> detected;
            while (ctx->MoreDetected()) {
                auto primitive = Handle(InteractivePrimitive)::DownCast(ctx->DetectedInteractive());
                if (primitive && !detected.contains(primitive)) {
                    detected << primitive;
                    menu.addAction(tr("Remove"), this, [this, primitive](){
                        d_ptr->removePrimitive(primitive);
                    });
                }
                ctx->NextDetected();
            }
        }
        if (!menu.isEmpty()) {
            ret = menu.exec(event->globalPos()) != nullptr;
        }
    }
    return ret;
}
