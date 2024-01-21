/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UTILITY_H
#define UTILITY_H

#include "ExamplesBase_global.h"

#include <vector>

class gp_Dir;
class gp_Pnt;
class TopoDS_Edge;
class TopoDS_Face;
class TopoDS_Shape;

namespace ExamplesBase {

    EXAMPLESBASE_EXPORT TopoDS_Face findFaceByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt);
    EXAMPLESBASE_EXPORT TopoDS_Edge findEdgeByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt);

    EXAMPLESBASE_EXPORT gp_Dir getNormal(const TopoDS_Face &face, const gp_Pnt &point);
    EXAMPLESBASE_EXPORT std::vector <gp_Dir> getNormals(const TopoDS_Face &face, const std::vector <gp_Pnt> &points);

}

#endif // UTILITY_H
