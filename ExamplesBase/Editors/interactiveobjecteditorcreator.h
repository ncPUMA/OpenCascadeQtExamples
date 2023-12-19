/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTEDITORCREATOR_H
#define INTERACTIVEOBJECTEDITORCREATOR_H

#include <Standard_Handle.hxx>

namespace ExamplesBase {

class InteractiveObject;
class InteractiveObjectEditor;

class InteractiveObjectEditorCreator
{
public:
    InteractiveObjectEditorCreator();

    Handle(InteractiveObjectEditor) create(const Handle(InteractiveObject) &object);
};

}

#endif // INTERACTIVEOBJECTEDITORCREATOR_H
