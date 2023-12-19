/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "steploader.h"

#include <STEPControl_Reader.hxx>

namespace ExamplesBase {

StepLoader::StepLoader()
    : AbstractModelLoader()
{

}

TopoDS_Shape StepLoader::load(const char *fName)
{
    TopoDS_Shape result;
    STEPControl_Reader aReader;
    const IFSelect_ReturnStatus aStatus = aReader.ReadFile(fName);
    if (aStatus == IFSelect_RetDone) {
        bool anIsFailsOnly = false;
        aReader.PrintCheckLoad(anIsFailsOnly, IFSelect_ItemsByEntity);

        int aRootsNumber = aReader.NbRootsForTransfer();
        aReader.PrintCheckTransfer(anIsFailsOnly, IFSelect_ItemsByEntity);
        for (Standard_Integer i = 1; i <= aRootsNumber; i++) {
            aReader.TransferRoot(i);
        }
        result = aReader.OneShape();
    }
    return result;
}

}
