/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveshape.h"

#include <AIS_InteractiveContext.hxx>

#include "../ModelLoader/abstractmodelloader.h"
#include "../ModelLoader/modelloaderfactorymethod.h"

namespace ExamplesBase {

class InteractiveShapePrivate
{
    friend class InteractiveShape;

    QString path;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveShape, InteractiveObject)

InteractiveShape::InteractiveShape()
    : InteractiveObject()
    , d(new InteractiveShapePrivate)
{

}

InteractiveShape::~InteractiveShape()
{
    delete d;
}

bool InteractiveShape::isValid() const
{
    return !Shape().IsNull();
}

QString InteractiveShape::modelPath() const
{
    return d->path;
}

void InteractiveShape::setModelPath(const QString &path)
{
    d->path = path;
    ExamplesBase::ModelLoaderFactoryMethod factory;
    auto &loader = factory.loaderByFName(path);
    auto shape = loader.load(path.toLatin1());
    SetShape(shape);
    if (GetContext()) {
        GetContext()->Redisplay(this, Standard_True);
    }
}

}
