/*
    SPDX-FileCopyrightText: 2024 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivetexturedplane.h"

#include <AIS_DisplayMode.hxx>
#include <Graphic3d_Texture2Dplane.hxx>
#include <Prs3d_ShadingAspect.hxx>

namespace ExamplesBase {

class InteractiveTexturedPlanePrivate
{
    friend class InteractiveTexturedPlane;

    TCollection_AsciiString textureFileName;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveTexturedPlane, InteractiveSurfacePlane)

InteractiveTexturedPlane::InteractiveTexturedPlane()
    : InteractiveSurfacePlane()
    , d(new InteractiveTexturedPlanePrivate)
{
    SetDisplayMode(AIS_Shaded);

    Attributes()->SetupOwnShadingAspect();
    auto shadingAspect = Attributes()->ShadingAspect()->Aspect();
    auto material = shadingAspect->FrontMaterial();
    material.SetColor(Quantity_NOC_WHITE);
    material.SetTransparency(.001);
    shadingAspect->SetFrontMaterial(material);
    shadingAspect->SetTextureMapOn(true);

    setTextureFileName("../Textures/barcode2.png");
}

InteractiveTexturedPlane::~InteractiveTexturedPlane()
{
    delete d;
}

TCollection_AsciiString InteractiveTexturedPlane::textureFileName() const
{
    return d->textureFileName;
}

void InteractiveTexturedPlane::setTextureFileName(const TCollection_AsciiString &fname)
{
    d->textureFileName = fname;
    Handle(Graphic3d_Texture2Dplane) texture = new Graphic3d_Texture2Dplane(d->textureFileName);
    texture->DisableRepeat();
    texture->DisableModulate();

    auto shadingAspect = Attributes()->ShadingAspect()->Aspect();
    shadingAspect->SetTextureMap(texture);
}

}
