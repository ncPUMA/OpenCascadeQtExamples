#include "viewport.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Point.hxx>
#include <AIS_Shape.hxx>
#include <Geom_CartesianPoint.hxx>

#include <ExamplesBase/ModelLoader/steploader.h>

#include "interactivearrowsequence.h"

class ViewportPrivate
{
    friend class Viewport;

};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    const char *path = "../Models/sample_telescope_system_full.step";
    ExamplesBase::StepLoader loader;
    Handle(AIS_Shape) scene = new AIS_Shape(loader.load(path));
    scene->SetDisplayMode(AIS_Shaded);
    context()->Display(scene, Standard_True);
    context()->Deactivate(scene);

    std::vector<std::pair<gp_Pnt, gp_Pnt> > seq1 = {
        { gp_Pnt(0., -100., 0.), gp_Pnt(0., 0., 0.) },
        { gp_Pnt(0., 0., 0.), gp_Pnt(0., 200., 0.) },
        { gp_Pnt(0., 200., 0.), gp_Pnt(0., 200., 150.) },
    };

    Handle(InteractiveArrowSequence) seqObj1 = new InteractiveArrowSequence;
    seqObj1->setPoints(seq1);
    seqObj1->SetColor(Quantity_NOC_GREEN);
    seqObj1->SetTransparency(.5);
    context()->Display(seqObj1, Standard_True);
}

Viewport::~Viewport()
{
    delete d_ptr;
}
