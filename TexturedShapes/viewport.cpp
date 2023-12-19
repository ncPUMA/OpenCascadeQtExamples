/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewport.h"

#include <QDebug>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Geom_Plane.hxx>
#include <Graphic3d_Texture2Dmanual.hxx>
#include <Graphic3d_Texture2Dplane.hxx>
#include <Prs3d_ShadingAspect.hxx>

class ViewportPrivate
{
    friend class Viewport;

    void addDeathStar() {
        auto sphereShape = BRepPrimAPI_MakeSphere(50.);
        Handle(AIS_Shape) sphereObj = new AIS_Shape(sphereShape);
        sphereObj->Attributes()->SetupOwnShadingAspect();
        Handle(Graphic3d_Texture2Dmanual) texture = new Graphic3d_Texture2Dmanual("../Textures/deathstar.png");
        texture->DisableModulate();
        auto shadingAspect = sphereObj->Attributes()->ShadingAspect()->Aspect();
        auto material = shadingAspect->FrontMaterial();
        material.SetColor(Quantity_NOC_WHITE);
        material.SetTransparency(.001);
        shadingAspect->SetFrontMaterial(material);
        shadingAspect->SetTextureMapOn(true);
        shadingAspect->SetTextureMap(texture);
        sphereObj->SetDisplayMode(AIS_Shaded);
        auto ctx = q_ptr->context();
        ctx->Display(sphereObj,Standard_False);
        ctx->Deactivate(sphereObj);
        gp_Trsf transform;
        transform.SetTranslation(gp_Pnt(), gp_Pnt(200., 0., 0.));
        ctx->SetLocation(sphereObj, transform);
    }

    void addCube() {
        auto boxShape = BRepPrimAPI_MakeBox(50., 50., 50.);
        Handle(AIS_Shape) boxObj = new AIS_Shape(boxShape);
        boxObj->Attributes()->SetupOwnShadingAspect();
        auto shadingAspect = boxObj->Attributes()->ShadingAspect()->Aspect();
        auto material = shadingAspect->FrontMaterial();
        material.SetColor(Quantity_NOC_WHITE);
        shadingAspect->SetFrontMaterial(material);

        Handle(Graphic3d_Texture2Dplane) texture1 = new Graphic3d_Texture2Dplane("../Textures/brick-texture-23887.png");
        texture1->DisableRepeat();
        texture1->DisableModulate();
//        texture1->SetScaleS(2.);
//        texture1->SetTranslateS(.2);
//        texture1->SetTranslateT(1.2);
        texture1->SetRotation(15.);

        Handle(Graphic3d_Texture2Dplane) texture2 = new Graphic3d_Texture2Dplane("../Textures/brick-texture-23888.png");
        texture2->DisableRepeat();
//        texture2->SetScaleS(2.);
        texture2->SetTranslateS(.2);
//        texture2->SetTranslateT(.5);
//        texture2->SetRotation(45.);

        Handle(Graphic3d_TextureSet) textureSet = new Graphic3d_TextureSet(2);
        textureSet->SetValue(0, texture1);
        textureSet->SetValue(1, texture2);
        shadingAspect->SetTextureMapOn(true);
        shadingAspect->SetTextureSet(textureSet);

        boxObj->SetDisplayMode(AIS_Shaded);
        auto ctx = q_ptr->context();
        ctx->Display(boxObj,Standard_False);
        ctx->Deactivate(boxObj);

        gp_Trsf transform;
        transform.SetTranslation(gp_Pnt(), gp_Pnt(0., 200., 0.));
        ctx->SetLocation(boxObj, transform);
    }

    void addPlane() {
        gp_Ax3 axes;
        Handle(Geom_Plane) plane = new Geom_Plane(axes);
        BRepBuilderAPI_MakeFace faceMaker(plane, 0., 50., 0., 50., Precision::Confusion());
        Handle(AIS_Shape) planeObj = new AIS_Shape(faceMaker.Face());
        planeObj->Attributes()->SetupOwnShadingAspect();
        auto shadingAspect = planeObj->Attributes()->ShadingAspect()->Aspect();
        auto material = shadingAspect->FrontMaterial();
        material.SetColor(Quantity_NOC_WHITE);
        shadingAspect->SetFrontMaterial(material);

        Handle(Graphic3d_Texture2Dplane) texture = new Graphic3d_Texture2Dplane("../Textures/brick-texture-23888.png");
        texture->DisableRepeat();
        texture->DisableModulate();

        shadingAspect->SetTextureMapOn(true);
        shadingAspect->SetTextureMap(texture);

        planeObj->SetDisplayMode(AIS_Shaded);
        auto ctx = q_ptr->context();
        ctx->Display(planeObj,Standard_False);
        ctx->Deactivate(planeObj);
    }

    Viewport *q_ptr;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    d_ptr->addDeathStar();
    d_ptr->addCube();
    d_ptr->addPlane();
}

Viewport::~Viewport()
{
    delete d_ptr;
}
