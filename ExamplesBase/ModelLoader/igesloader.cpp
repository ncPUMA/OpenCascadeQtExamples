/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "igesloader.h"

#include <IGESControl_Reader.hxx>

namespace ExamplesBase {

IgesLoader::IgesLoader()
    : AbstractModelLoader()
{

}

TopoDS_Shape IgesLoader::load(const char *fName)
{
    TopoDS_Shape result;
    IGESControl_Reader aReader;
    int status = aReader.ReadFile (fName);
    if (status == IFSelect_RetDone) {
        aReader.TransferRoots();
        result = aReader.OneShape();
    }
    return result;
}

}
