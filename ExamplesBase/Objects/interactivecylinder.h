/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVECYLINDER_H
#define INTERACTIVECYLINDER_H

#include "interactiveobject.h"

namespace ExamplesBase {

class InteractiveCylinderPrivate;

class InteractiveCylinder : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveCylinder, InteractiveObject)

public:
    InteractiveCylinder();
    ~InteractiveCylinder();

    Standard_Real getRadius() const;
    Standard_Real getLenght() const;

    void setRadius(Standard_Real radius);
    void setLenght(Standard_Real lenght);

private:
    InteractiveCylinderPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveCylinder, InteractiveObject)

}

#endif // INTERACTIVECYLINDER_H
