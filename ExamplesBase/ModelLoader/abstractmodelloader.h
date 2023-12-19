/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_ABSTRACTMODELLOADER_H
#define EB_ABSTRACTMODELLOADER_H

#include "../ExamplesBase_global.h"

#include <TopoDS_Shape.hxx>

namespace ExamplesBase {

class EXAMPLESBASE_EXPORT AbstractModelLoader
{
public:
    AbstractModelLoader() = default;
    virtual ~AbstractModelLoader() = default;

    virtual TopoDS_Shape load(const char *fName) = 0;

private:
    AbstractModelLoader(const AbstractModelLoader &) = delete;
    AbstractModelLoader& operator=(const AbstractModelLoader &) = delete;
};

}

#endif // EB_ABSTRACTMODELLOADER_H
