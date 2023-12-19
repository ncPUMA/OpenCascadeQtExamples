/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectcylindereditor.h"

#include "../Objects/interactivecylinder.h"

#include "dimentionlenghteditor.h"

namespace ExamplesBase {

class InteractiveObjectCylinderEditorPrivate
{
    friend class InteractiveObjectCylinderEditor;

    void update() {
        gp_Pnt p0;
        gp_Pnt pR(cylinder->getRadius(), 0., 0.);
        gp_Pnt pL(cylinder->getRadius(), 0., cylinder->getLenght());
        editorRadius->update(p0, pR, gp_Pln(p0, gp_Dir(0., 1., 0.)));
        editorLenght->update(pR, pL, gp_Pln(p0, gp_Dir(0., 1., 0.)));
    }

    Handle(InteractiveCylinder) cylinder;

    Handle(DimentionLenghtEditor) editorRadius;
    Handle(DimentionLenghtEditor) editorLenght;
};

}

using namespace ExamplesBase;

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectCylinderEditor, InteractiveObjectEditor)

InteractiveObjectCylinderEditor::InteractiveObjectCylinderEditor(const Handle(InteractiveCylinder) &cylinder)
    : InteractiveObjectEditor(cylinder)
    , d(new InteractiveObjectCylinderEditorPrivate)
{
    d->cylinder = cylinder;

    d->editorRadius = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
//    d->editorWidth->SetFlyout(- d->editorWidth->GetFlyout());
    AddChild(d->editorRadius);
    d->editorLenght = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorLenght);
    d->update();
}

InteractiveObjectCylinderEditor::~InteractiveObjectCylinderEditor()
{
    delete d;
}

void InteractiveObjectCylinderEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (!dimentionEditor) {
        return;
    }

    Standard_Real value = dimentionEditor->dimention();
    if (dimentionEditor == d->editorRadius) {
        d->cylinder->setRadius(value);
    } else if (dimentionEditor == d->editorLenght) {
        d->cylinder->setLenght(value);
    }
}

void InteractiveObjectCylinderEditor::updateEditors()
{
    d->update();
}
