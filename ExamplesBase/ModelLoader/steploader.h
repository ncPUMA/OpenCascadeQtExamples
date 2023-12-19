/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_STEPLOADER_H
#define EB_STEPLOADER_H

#include "abstractmodelloader.h"

namespace ExamplesBase {

class StepLoader : public AbstractModelLoader
{
public:
    StepLoader();

    TopoDS_Shape load(const char *fName) final;
};

}
#endif // EB_STEPLOADER_H
