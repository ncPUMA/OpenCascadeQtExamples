/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "stlloader.h"

#include <BRep_Builder.hxx>
#include <RWStl.hxx>

namespace ExamplesBase {

StlLoader::StlLoader()
    : AbstractModelLoader()
{
}

TopoDS_Shape StlLoader::load(const char *fName)
{
    TopoDS_Shape result;
    Handle(Poly_Triangulation) mesh = RWStl::ReadFile(fName);
    if (!mesh.IsNull()) {
        BRep_Builder builder;
        TopoDS_Face face;        
        builder.MakeFace(face, mesh);
        result = face;
    }
    return result;
}

}
