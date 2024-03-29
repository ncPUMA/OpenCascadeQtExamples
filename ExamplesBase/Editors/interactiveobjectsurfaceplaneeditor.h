/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTSURFACEPLANEEDITOR_H
#define INTERACTIVEOBJECTSURFACEPLANEEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveSurfacePlane;
class InteractiveObjectSurfacePlaneEditorPrivate;

class InteractiveObjectSurfacePlaneEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfacePlaneEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfacePlaneEditor(const Handle(InteractiveSurfacePlane) &surface);
    ~InteractiveObjectSurfacePlaneEditor();

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfacePlaneEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfacePlaneEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTSURFACEPLANEEDITOR_H
