#include "brepserializer.h"

#include <sstream>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

std::string BrepSerializer::serialize(const TopoDS_Shape &shape)
{
    std::ostringstream ss;
    BRepTools::Write(shape, ss);
    return ss.str();
}

TopoDS_Shape BrepSerializer::deserialize(const std::string &data)
{
    TopoDS_Shape shape;
    BRep_Builder builder;
    std::istringstream ss(data);
    BRepTools::Read(shape, ss, builder);
    return shape;
}
