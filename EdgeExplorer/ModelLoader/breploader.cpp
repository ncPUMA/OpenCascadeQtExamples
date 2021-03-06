#include "breploader.h"

#include <TopoDS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

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
