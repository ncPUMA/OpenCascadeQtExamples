/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "modelloaderfactorymethod.h"

#include <vector>
#include <map>

#include <QCoreApplication>
#include <QStringList>

#include "breploader.h"
#include "steploader.h"
#include "igesloader.h"
#include "stlloader.h"
#include "objloader.h"

namespace ExamplesBase {

class EmptyModelLoader : public AbstractModelLoader
{
public:
    EmptyModelLoader() : AbstractModelLoader() { }

    TopoDS_Shape load(const char *) final {
        return TopoDS_Shape();
    }
};



class ModelLoaderFactoryMethodPrivate
{
    friend class ModelLoaderFactoryMethod;

    ModelLoaderFactoryMethodPrivate() {
        appendLoader(QT_TRANSLATE_NOOP("ModelLoaderFactoryMethod", "STEP (*.step *.stp)"), new StepLoader());
        appendLoader(QT_TRANSLATE_NOOP("ModelLoaderFactoryMethod", "BREP (*.brep)"), new BrepLoader());
        appendLoader(QT_TRANSLATE_NOOP("ModelLoaderFactoryMethod", "IGES (*.iges)"), new IgesLoader());
        appendLoader(QT_TRANSLATE_NOOP("ModelLoaderFactoryMethod", "STL (*.stl)")  , new StlLoader());
        appendLoader(QT_TRANSLATE_NOOP("ModelLoaderFactoryMethod", "OBJ (*.obj)")  , new ObjLoader());
    }

    ~ModelLoaderFactoryMethodPrivate() {
        for(const auto &pair : loaders)
            delete pair.second;
    }

    AbstractModelLoader& loader(const QString &filter) {
        AbstractModelLoader * result = &emptyLoader;
        for(const auto &pair : loaders) {
            if (pair.first == filter) {
                result = pair.second;
                break;
            }
        }
        return *result;
    }

    AbstractModelLoader& loaderByFName(const QString &fName) {
        AbstractModelLoader * result = &emptyLoader;
        const QStringList ls = fName.split('.');
        if (ls.size() > 1) {
            const QString &ext = ls.last();
            for(const auto &pair : loaders) {
                if (pair.first.contains(ext, Qt::CaseInsensitive)) {
                    result = pair.second;
                    break;
                }
            }
        }
        return *result;
    }

    void appendLoader(const char *format, AbstractModelLoader * const loader) {
        loaders.push_back(std::make_pair(QCoreApplication::translate("ModelLoaderFactoryMethod", format), loader));
    }

    std::vector <std::pair <QString, AbstractModelLoader *> > loaders;
    EmptyModelLoader emptyLoader;
};



ModelLoaderFactoryMethod::ModelLoaderFactoryMethod() :
    d_ptr(new ModelLoaderFactoryMethodPrivate())
{

}

ModelLoaderFactoryMethod::~ModelLoaderFactoryMethod()
{
    delete d_ptr;
}

QString ModelLoaderFactoryMethod::supportedFilters() const
{
    QStringList ls;
    for(const auto &pair : d_ptr->loaders)
        ls << pair.first;
    return ls.join(";;");
}

AbstractModelLoader &ModelLoaderFactoryMethod::loader(const QString &filter)
{
    return d_ptr->loader(filter);
}

AbstractModelLoader &ModelLoaderFactoryMethod::loaderByFName(const QString &fName)
{
    return d_ptr->loaderByFName(fName);
}

}
