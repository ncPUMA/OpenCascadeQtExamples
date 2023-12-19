/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVECUBOID_H
#define INTERACTIVECUBOID_H

#include "interactiveobject.h"

namespace ExamplesBase {

class InteractiveCuboidPrivate;

class InteractiveCuboid : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveCuboid, InteractiveObject)

public:
    InteractiveCuboid();
    ~InteractiveCuboid();

    Standard_Real getWidth() const;
    Standard_Real getHeight() const;
    Standard_Real getDepth() const;

    void setWidth(Standard_Real width);
    void setHeight(Standard_Real height);
    void setDepth(Standard_Real depth);

private:
    InteractiveCuboidPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveCuboid, InteractiveObject)

}

#endif // INTERACTIVECUBOID_H
