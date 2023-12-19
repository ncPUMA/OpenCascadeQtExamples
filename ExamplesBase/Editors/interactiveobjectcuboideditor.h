/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTCUBOIDEDITOR_H
#define INTERACTIVEOBJECTCUBOIDEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveCuboid;
class InteractiveObjectCuboidEditorPrivate;

class InteractiveObjectCuboidEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectCuboidEditor, InteractiveObjectEditor)

public:
    InteractiveObjectCuboidEditor(const Handle(InteractiveCuboid) &cuboid);
    ~InteractiveObjectCuboidEditor();

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectCuboidEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectCuboidEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTCUBOIDEDITOR_H
