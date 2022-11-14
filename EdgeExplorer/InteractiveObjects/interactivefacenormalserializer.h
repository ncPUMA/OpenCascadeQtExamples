#ifndef INTERACTIVEFACENORMALSERIALIZER_H
#define INTERACTIVEFACENORMALSERIALIZER_H

#include <QJsonObject>

#include <Standard_Handle.hxx>

class InteractiveFaceNormal;

class InteractiveFaceNormalSerializer
{
public:
    static QJsonObject serialize(const Handle(InteractiveFaceNormal) &normal);
    static Handle(InteractiveFaceNormal) deserialize(const QJsonObject &obj);
};

#endif // INTERACTIVEFACENORMALSERIALIZER_H
