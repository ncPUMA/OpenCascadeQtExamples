/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTSPHEREEDITOR_H
#define INTERACTIVEOBJECTSPHEREEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveSphere;
class InteractiveObjectSphereEditorPrivate;

class InteractiveObjectSphereEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSphereEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSphereEditor(const Handle(InteractiveSphere) &sphere);
    ~InteractiveObjectSphereEditor();

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSphereEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSphereEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTSPHEREEDITOR_H
