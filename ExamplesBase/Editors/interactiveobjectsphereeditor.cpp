/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectsphereeditor.h"

#include "../Objects/interactivesphere.h"

#include "dimentionlenghteditor.h"

namespace ExamplesBase {

class InteractiveObjectSphereEditorPrivate
{
    friend class InteractiveObjectSphereEditor;

    void update() {
        gp_Pnt p0;
        gp_Pnt pR(sphere->getRadius(), 0., 0.);
        editorRadius->update(p0, pR, gp_Pln(p0, gp_Dir(0., 1., 0.)));
    }

    Handle(InteractiveSphere) sphere;
    Handle(DimentionLenghtEditor) editorRadius;
};

}

using namespace ExamplesBase;

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSphereEditor, InteractiveObjectEditor)

InteractiveObjectSphereEditor::InteractiveObjectSphereEditor(const Handle(InteractiveSphere) &sphere)
    : InteractiveObjectEditor(sphere)
    , d(new InteractiveObjectSphereEditorPrivate)
{
    d->sphere = sphere;

    d->editorRadius = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorRadius);
    d->update();
}

InteractiveObjectSphereEditor::~InteractiveObjectSphereEditor()
{
    delete d;
}

void InteractiveObjectSphereEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (!dimentionEditor) {
        return;
    }

    Standard_Real value = dimentionEditor->dimention();
    if (dimentionEditor == d->editorRadius) {
        d->sphere->setRadius(value);
    }
}

void InteractiveObjectSphereEditor::updateEditors()
{
    d->update();
}

