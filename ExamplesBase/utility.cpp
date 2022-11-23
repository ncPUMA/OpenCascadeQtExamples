#include "utility.h"

#include <map>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
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

}
