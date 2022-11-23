#ifndef INTERACTIVEPRIMITIVESERIALIZER_H
#define INTERACTIVEPRIMITIVESERIALIZER_H

#include <string>

#include <Standard_Handle.hxx>

class InteractivePrimitive;

class InteractivePrimitiveSerializer
{
public:
    static std::string serialize(const Handle(InteractivePrimitive) &primitive);
    static Handle(InteractivePrimitive) deserialize(const std::string &data);

private:
    InteractivePrimitiveSerializer() = delete;
    InteractivePrimitiveSerializer(const InteractivePrimitiveSerializer &) = delete;
    InteractivePrimitiveSerializer& operator=(const InteractivePrimitiveSerializer &) = delete;
};

#endif // INTERACTIVEPRIMITIVESERIALIZER_H
