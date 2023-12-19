/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveprimitiveserializer.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <gp_Quaternion.hxx>

#include "Primitives/interactiveprimitivebox.h"
#include "Primitives/interactiveprimitivecone.h"
#include "Primitives/interactiveprimitivecylinder.h"

template <typename T>
inline QJsonObject serialize(const Handle(InteractivePrimitive) &primitive) = delete;

template <typename T>
inline Handle(InteractivePrimitive) deserialize(const QJsonObject &object) = delete;

// InteractivePrimitiveBox
template <>
inline QJsonObject serialize<InteractivePrimitiveBox>(const Handle(InteractivePrimitive) &primitive)
{
    QJsonObject ret;
    auto box = Handle(InteractivePrimitiveBox)::DownCast(primitive);
    ret["Width"] = box->getWidth();
    ret["Height"] = box->getHeight();
    ret["Depth"] = box->getDepth();
    return ret;
}

template <>
inline Handle(InteractivePrimitive) deserialize<InteractivePrimitiveBox>(const QJsonObject &obj)
{
    Handle(InteractivePrimitiveBox) ret = new InteractivePrimitiveBox();
    ret->setWidth(obj["Width"].toDouble());
    ret->setHeight(obj["Height"].toDouble());
    ret->setDepth(obj["Depth"].toDouble());
    return ret;
}

// InteractivePrimitiveCylinder
template <>
inline QJsonObject serialize<InteractivePrimitiveCylinder>(const Handle(InteractivePrimitive) &primitive)
{
    QJsonObject ret;
    auto box = Handle(InteractivePrimitiveCylinder)::DownCast(primitive);
    ret["Radius"] = box->getRadius();
    ret["Height"] = box->getHeight();
    return ret;
}

template <>
inline Handle(InteractivePrimitive) deserialize<InteractivePrimitiveCylinder>(const QJsonObject &obj)
{
    Handle(InteractivePrimitiveCylinder) ret = new InteractivePrimitiveCylinder();
    ret->setRadius(obj["Radius"].toDouble());
    ret->setHeight(obj["Height"].toDouble());
    return ret;
}

// InteractivePrimitiveCone
template <>
inline QJsonObject serialize<InteractivePrimitiveCone>(const Handle(InteractivePrimitive) &primitive)
{
    QJsonObject ret;
    auto box = Handle(InteractivePrimitiveCone)::DownCast(primitive);
    ret["RadiusNear"] = box->getRadiusNear();
    ret["RadiusFar"] = box->getRadiusFar();
    ret["Height"] = box->getHeight();
    return ret;
}

template <>
inline Handle(InteractivePrimitive) deserialize<InteractivePrimitiveCone>(const QJsonObject &obj)
{
    Handle(InteractivePrimitiveCone) ret = new InteractivePrimitiveCone();
    ret->setRadiusNear(obj["RadiusNear"].toDouble());
    ret->setRadiusFar(obj["RadiusFar"].toDouble());
    ret->setHeight(obj["Height"].toDouble());
    return ret;
}

std::map<QString, std::function<QJsonObject(const Handle(InteractivePrimitive) &primitive)>> sSerializers = {
    { QStringLiteral("InteractivePrimitiveBox"), serialize<InteractivePrimitiveBox> },
    { QStringLiteral("InteractivePrimitiveCone"), serialize<InteractivePrimitiveCone> },
    { QStringLiteral("InteractivePrimitiveCylinder"), serialize<InteractivePrimitiveCylinder> },
};

std::map<QString, std::function<Handle(InteractivePrimitive)(const QJsonObject &obj)>> sDeserializers = {
    { QStringLiteral("InteractivePrimitiveBox"), deserialize<InteractivePrimitiveBox> },
    { QStringLiteral("InteractivePrimitiveCone"), deserialize<InteractivePrimitiveCone> },
    { QStringLiteral("InteractivePrimitiveCylinder"), deserialize<InteractivePrimitiveCylinder> },
};

std::string InteractivePrimitiveSerializer::serialize(const Handle(InteractivePrimitive) &primitive)
{
    if (!primitive) {
        return std::string();
    }

    auto it = sSerializers.find(QLatin1String(primitive->DynamicType()->Name()));
    if (it == sSerializers.cend()) {
        return std::string();
    }

    auto obj = it->second(primitive);
    obj[QStringLiteral("type")] = QString::fromLatin1(primitive->DynamicType()->Name());
    auto trsf = primitive->LocalTransformation();
    auto pos = trsf.TranslationPart();
    obj[QStringLiteral("X")] = pos.X();
    obj[QStringLiteral("Y")] = pos.Y();
    obj[QStringLiteral("Z")] = pos.Z();
    auto quat = trsf.GetRotation();
    Standard_Real alpha, beta, gamma;
    quat.GetEulerAngles(gp_Extrinsic_XYZ, alpha, beta, gamma);
    obj[QStringLiteral("Alpha")] = alpha;
    obj[QStringLiteral("Beta")] = beta;
    obj[QStringLiteral("Gamma")] = gamma;
    return QJsonDocument(obj).toJson().toStdString();
}

Handle(InteractivePrimitive) InteractivePrimitiveSerializer::deserialize(const std::string &data)
{
    auto obj = QJsonDocument::fromJson(QByteArray::fromStdString(data)).object();
    auto it = sDeserializers.find(obj.value(QStringLiteral("type")).toString());
    if (it == sDeserializers.cend()) {
        return nullptr;
    }

    auto primitive = it->second(obj);
    if (!primitive) {
        return nullptr;
    }

    gp_XYZ pos(obj.value(QStringLiteral("X")).toDouble(),
               obj.value(QStringLiteral("Y")).toDouble(),
               obj.value(QStringLiteral("Z")).toDouble());
    gp_Quaternion quat;
    quat.SetEulerAngles(gp_Extrinsic_XYZ,
                        obj.value(QStringLiteral("Alpha")).toDouble(),
                        obj.value(QStringLiteral("Beta")).toDouble(),
                        obj.value(QStringLiteral("Gamma")).toDouble());
    gp_Trsf trsf;
    trsf.SetTranslationPart(pos);
    trsf.SetRotationPart(quat);
    primitive->SetLocalTransformation(trsf);
    return primitive;
}
