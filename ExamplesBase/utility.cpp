/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "utility.h"

#include <map>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepTools.hxx>
#include <GeomLProp_SLProps.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

namespace ExamplesBase
{

TopoDS_Face findFaceByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt)
{
    BRepBuilderAPI_MakeVertex builder(localPnt);
    std::map <Standard_Real, TopoDS_Face> faces;
    for (TopExp_Explorer anExp(shape, TopAbs_FACE); anExp.More(); anExp.Next()) {
        auto &curFace = TopoDS::Face(anExp.Current());
        BRepExtrema_DistShapeShape extrema(curFace, builder.Vertex());
        if (extrema.IsDone()) {
            faces[extrema.Value()] = curFace;
        }
    }
    TopoDS_Face res;
    if (!faces.empty()) {
        res = faces.cbegin()->second;
    }
    return res;
}

TopoDS_Edge findEdgeByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt)
{
    BRepBuilderAPI_MakeVertex builder(localPnt);
    std::map <Standard_Real, TopoDS_Edge> edges;
    for (TopExp_Explorer anExp(shape, TopAbs_EDGE); anExp.More(); anExp.Next()) {
        auto &curEdge = TopoDS::Edge(anExp.Current());
        BRepExtrema_DistShapeShape extrema(curEdge, builder.Vertex());
        if (extrema.IsDone()) {
            edges[extrema.Value()] = curEdge;
        }
    }
    TopoDS_Edge res;
    if (!edges.empty()) {
        res = edges.cbegin()->second;
    }
    return res;
}

gp_Dir getNormal(const TopoDS_Face &face, const gp_Pnt &point, gp_Dir *D1U)
{
    auto aSurf = BRep_Tool::Surface(face);
    Standard_Real u1, u2, v1, v2;
    BRepTools::UVBounds(face, u1, u2, v1, v2);

    Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
    const gp_Pnt2d pUV = surfAnalis->ValueOfUV(point, Precision::Confusion());

    GeomLProp_SLProps props(aSurf, pUV.X(), pUV.Y(), 1, 0.01);
    gp_Dir normal = props.Normal();
    if (D1U) {
        *D1U = props.D1U();
    }
    if (face.Orientation() == TopAbs_REVERSED || face.Orientation() == TopAbs_INTERNAL) {
        normal.Reverse();
        if (D1U) {
            D1U->Reverse();
        }
    }
    return normal;
}

std::vector<gp_Dir> getNormals(const TopoDS_Face &face, const std::vector<gp_Pnt> &points)
{
    std::vector <gp_Dir> normals;
    normals.reserve(points.size());

    auto aSurf = BRep_Tool::Surface(face);
    Standard_Real u1, u2, v1, v2;
    BRepTools::UVBounds(face, u1, u2, v1, v2);

    Handle(ShapeAnalysis_Surface)  surfAnalis = new ShapeAnalysis_Surface(aSurf);
    for (const auto &point : points) {
        const gp_Pnt2d pUV = surfAnalis->ValueOfUV(point, Precision::Confusion());
        GeomLProp_SLProps props(aSurf, pUV.X(), pUV.Y(), 1, 0.01);
        gp_Dir normal = props.Normal();
        if (face.Orientation() == TopAbs_REVERSED || face.Orientation() == TopAbs_INTERNAL) {
            normal.Reverse();
        }
        normals.push_back(normal);
    }
    return normals;
}

}
