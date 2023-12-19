/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_OBJLOADER_H
#define EB_OBJLOADER_H

#include "abstractmodelloader.h"

namespace ExamplesBase {

class ObjLoader : public AbstractModelLoader
{
public:
    ObjLoader();

    TopoDS_Shape load(const char *fName) final;
};

}
#endif // EB_OBJLOADER_H
