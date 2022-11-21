#include "objloader.h"

#include <RWObj.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Poly_Triangulation.hxx>

namespace ExamplesBase {

ObjLoader::ObjLoader()
    : AbstractModelLoader()
{
}

TopoDS_Shape ObjLoader::load(const char *fName)
{
    TopoDS_Shape result;
    Handle(Poly_Triangulation) mesh = RWObj::ReadFile(fName);
    if (!mesh.IsNull()) {
        BRep_Builder builder;
        TopoDS_Face face;
        builder.MakeFace(face, mesh);
        result = face;
    }
    return result;
}

}
