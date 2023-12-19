/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "breploader.h"

#include <TopoDS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

namespace ExamplesBase {

BrepLoader::BrepLoader()
    : AbstractModelLoader()
{

}

TopoDS_Shape BrepLoader::load(const char *fName)
{
    TopoDS_Shape result;
    BRep_Builder aBuilder;
    BRepTools::Read(result, fName , aBuilder);
    return result;
}

}
