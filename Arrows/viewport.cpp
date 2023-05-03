#include "viewport.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <Geom_CartesianPoint.hxx>

#include <ExamplesBase/ModelLoader/steploader.h>

#include "interactivearrowsequence.h"

class ViewportPrivate
{
    friend class Viewport;

    void drawDemo(const Handle(AIS_InteractiveContext) &ctx) {
        std::vector<std::pair<gp_Pnt, gp_Pnt> > seq1 = {
            { gp_Pnt(0., -100., 0.), gp_Pnt(0., 0., 0.) },
            { gp_Pnt(0., 0., 0.), gp_Pnt(0., 200., 0.) },
            { gp_Pnt(0., 200., 0.), gp_Pnt(0., 200., 150.) },
        };

        Handle(InteractiveArrowSequence) seqObj1 = new InteractiveArrowSequence;
        seqObj1->setPoints(seq1);
        seqObj1->SetColor(Quantity_NOC_GREEN);
        seqObj1->SetTransparency(.5);
        seqObj1->setArrowPosition(.5);
        seqObj1->setArrowMode(InteractiveArrowSequence::AM_DrawFirst);
        ctx->Display(seqObj1, Standard_True);
    }

    void removeArrows(const Handle(AIS_InteractiveContext) &ctx) {
        AIS_ListOfInteractive objects;
        ctx->DisplayedObjects(objects);
        for(const auto &obj : objects) {
            if (Handle(InteractiveArrowSequence)::DownCast(obj)) {
                ctx->Remove(obj, Standard_True);
            }
        }
    }

    Standard_Real randomValue() const {
        return static_cast<Standard_Real>(rand() % 10000) / 10.;
    }

    void randomFill(std::vector<std::pair<gp_Pnt, gp_Pnt> > &points, size_t size) const {
        points.resize(size);
        for (size_t i = 0; i < size; ++i) {
            points[i] = std::make_pair(gp_Pnt(randomValue(), randomValue(), randomValue()),
                                       gp_Pnt(randomValue(), randomValue(), randomValue()));
        }
    }

    void drawWithArrows(const Handle(AIS_InteractiveContext) &ctx, size_t size) {
        QElapsedTimer tm;
        tm.start();
        std::vector<std::pair<gp_Pnt, gp_Pnt> > points;
        randomFill(points, size);
        qDebug() << "Random vector generated" << QString::number(tm.restart() / 1000., 'f') << "s.";

        Handle(InteractiveArrowSequence) obj = new InteractiveArrowSequence;
        obj->setPoints(points);
        obj->SetColor(Quantity_NOC_GREEN);
        obj->SetTransparency(.5);
        qDebug() << "Object created" << QString::number(tm.restart() / 1000., 'f') << "s.";
        ctx->Display(obj, Standard_False);
        qDebug() << "Object displayed" << QString::number(tm.restart() / 1000., 'f') << "s.";
    }

    void drawWithoutArrows(const Handle(AIS_InteractiveContext) &ctx, size_t size) {
        tm.start();
        std::vector<std::pair<gp_Pnt, gp_Pnt> > points;
        randomFill(points, size);
        qDebug() << "Random vector generated" << QString::number(tm.restart() / 1000., 'f') << "s.";

        Handle(InteractiveArrowSequence) obj = new InteractiveArrowSequence;
        obj->setPoints(points);
        obj->SetColor(Quantity_NOC_GREEN);
        obj->SetTransparency(.5);
        obj->setArrowMode(InteractiveArrowSequence::AM_Disable);
        qDebug() << "Object created" << QString::number(tm.restart() / 1000., 'f') << "s.";
        ctx->Display(obj, Standard_False);
        qDebug() << "Object displayed" << QString::number(tm.restart() / 1000., 'f') << "s.";
    }

    QElapsedTimer tm;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    std::srand(std::time(nullptr));

    const char *path = "../Models/sample_telescope_system_full.step";
    ExamplesBase::StepLoader loader;
    Handle(AIS_Shape) scene = new AIS_Shape(loader.load(path));
    scene->SetDisplayMode(AIS_Shaded);
    context()->Display(scene, Standard_True);
    context()->Deactivate(scene);

    d_ptr->drawDemo(context());
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QMenu topMenu;
        topMenu.addAction(tr("Draw demo"), this, [this]() {
            d_ptr->removeArrows(context());
            d_ptr->drawDemo(context());
        });
        topMenu.addSeparator();
        topMenu.addAction(tr("Draw 1M lines with arrows as one object"), this, [this]() {
            d_ptr->removeArrows(context());
            d_ptr->drawWithArrows(context(), 100000);
            d_ptr->tm.restart();
            QMetaObject::invokeMethod(this, [this](){
                qDebug() << "Render finish" << QString::number(d_ptr->tm.restart() / 1000., 'f') << "s.";
                fitInView();
            }, Qt::QueuedConnection);
        });
        topMenu.addAction(tr("Draw 1M lines without arrows as one object"), this, [this]() {
            d_ptr->removeArrows(context());
            d_ptr->drawWithoutArrows(context(), 1000000);
            d_ptr->tm.restart();
            QMetaObject::invokeMethod(this, [this](){
                qDebug() << "Render finish" << QString::number(d_ptr->tm.restart() / 1000., 'f') << "s.";
                fitInView();
            }, Qt::QueuedConnection);
        });
        return topMenu.exec(event->globalPos()) != nullptr;
    }
    return false;
}
