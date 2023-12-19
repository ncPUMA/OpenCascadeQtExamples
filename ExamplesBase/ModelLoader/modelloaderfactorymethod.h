/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_MODELLOADERFACTORYMETHOD_H
#define EB_MODELLOADERFACTORYMETHOD_H

#include <QString>

namespace ExamplesBase {

class ModelLoaderFactoryMethodPrivate;
class AbstractModelLoader;

class ModelLoaderFactoryMethod
{
public:
    ModelLoaderFactoryMethod();
    virtual ~ModelLoaderFactoryMethod();

    QString supportedFilters() const;
    AbstractModelLoader& loader(const QString &filter);
    AbstractModelLoader& loaderByFName(const QString &fName);

private:
    ModelLoaderFactoryMethodPrivate * const d_ptr;

private:
    ModelLoaderFactoryMethod(const ModelLoaderFactoryMethod &) = delete;
    ModelLoaderFactoryMethod& operator=(const ModelLoaderFactoryMethod &) = delete;
};

}

#endif // EB_CMODELLOADERFACTORYMETHOD_H
