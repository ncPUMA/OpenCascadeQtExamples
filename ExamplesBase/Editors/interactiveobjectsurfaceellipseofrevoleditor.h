/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTSURFACEELLIPSEOFREVOLEDITOR_H
#define INTERACTIVEOBJECTSURFACEELLIPSEOFREVOLEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveSurfaceEllipseOfRevol;
class InteractiveObjectSurfaceEllipseOfRevolEditorPrivate;

class InteractiveObjectSurfaceEllipseOfRevolEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfaceEllipseOfRevolEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfaceEllipseOfRevolEditor(const Handle(InteractiveSurfaceEllipseOfRevol) &surface);
    ~InteractiveObjectSurfaceEllipseOfRevolEditor();

    void SetContext(const Handle(AIS_InteractiveContext) &ctx) Standard_OVERRIDE;

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfaceEllipseOfRevolEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfaceEllipseOfRevolEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTSURFACEELLIPSEOFREVOLEDITOR_H
