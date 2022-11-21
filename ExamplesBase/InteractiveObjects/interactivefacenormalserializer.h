#ifndef EB_INTERACTIVEFACENORMALSERIALIZER_H
#define EB_INTERACTIVEFACENORMALSERIALIZER_H

#include <QJsonObject>

#include <Standard_Handle.hxx>

namespace ExamplesBase {

class InteractiveFaceNormal;

class InteractiveFaceNormalSerializer
{
public:
    static QJsonObject serialize(const Handle(InteractiveFaceNormal) &normal);
    static Handle(InteractiveFaceNormal) deserialize(const QJsonObject &obj);
};

}
#endif // EB_INTERACTIVEFACENORMALSERIALIZER_H
