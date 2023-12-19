/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVESHAPE_H
#define INTERACTIVESHAPE_H

#include "interactiveobject.h"

namespace ExamplesBase {

class InteractiveShapePrivate;

class InteractiveShape : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveShape, InteractiveObject)

public:
    InteractiveShape();
    ~InteractiveShape();

    bool isValid() const;
    QString modelPath() const;
    void setModelPath(const QString &path);

private:
    InteractiveShapePrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveShape, InteractiveObject)

}

#endif // INTERACTIVESHAPE_H
