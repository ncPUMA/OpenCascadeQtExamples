/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTITEMMODELCREATOR_H
#define INTERACTIVEOBJECTITEMMODELCREATOR_H

#include <Standard_Handle.hxx>

namespace ExamplesBase {

class InteractiveObjectItemModel;
class InteractiveObject;

class InteractiveObjectItemModelCreator
{
public:
    InteractiveObjectItemModelCreator();

    InteractiveObjectItemModel *createModel(const Handle(InteractiveObject) &interactive);
};

}

#endif // INTERACTIVEOBJECTITEMMODELCREATOR_H
