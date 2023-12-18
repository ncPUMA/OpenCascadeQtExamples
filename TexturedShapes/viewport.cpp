#include "viewport.h"

#include <QDebug>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Graphic3d_Texture2Dmanual.hxx>
#include <Prs3d_ShadingAspect.hxx>

class ViewportPrivate
{
    friend class Viewport;

    Viewport *q_ptr;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    auto sphereShape = BRepPrimAPI_MakeSphere(50.);
    Handle(AIS_Shape) sphereObj = new AIS_Shape(sphereShape);
    sphereObj->Attributes()->SetupOwnShadingAspect();
    Handle(Graphic3d_Texture2Dmanual) texture = new Graphic3d_Texture2Dmanual("../Textures/deathstar.png");
    texture->DisableModulate();
    auto shadingAspect = sphereObj->Attributes()->ShadingAspect()->Aspect();
    auto material = shadingAspect->FrontMaterial();
    material.SetColor(Quantity_NOC_WHITESMOKE);
    material.SetTransparency(.001);
    shadingAspect->SetFrontMaterial(material);
    shadingAspect->SetTextureMapOn(true);
    shadingAspect->SetTextureMap(texture);
    sphereObj->SetDisplayMode(AIS_Shaded);
    context()->Display(sphereObj,Standard_False);
    context()->Deactivate(sphereObj);
}

Viewport::~Viewport()
{
    delete d_ptr;
}
