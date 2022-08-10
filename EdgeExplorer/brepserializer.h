#ifndef BREPSERIALIZER_H
#define BREPSERIALIZER_H

#include <string>

#include <TopoDS_Shape.hxx>

class BrepSerializer
{
public:
    static std::string serialize(const TopoDS_Shape &shape);
    static TopoDS_Shape deserialize(const std::string &data);
};

#endif // BREPSERIALIZER_H
