/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_BREPLOADER_H
#define EB_BREPLOADER_H

#include "abstractmodelloader.h"

namespace ExamplesBase {

class BrepLoader : public AbstractModelLoader
{
public:
    BrepLoader();

    TopoDS_Shape load(const char *fName) final;
};

}

#endif // EB_BREPLOADER_H
