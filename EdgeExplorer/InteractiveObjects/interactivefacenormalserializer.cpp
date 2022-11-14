#include "interactivefacenormalserializer.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <gp_Quaternion.hxx>
#include <TopoDS_Face.hxx>

#include "interactivefacenormal.h"

QJsonObject InteractiveFaceNormalSerializer::serialize(const Handle(InteractiveFaceNormal) &normal)
{
    const gp_Quaternion quat = normal->getRotation();
    Standard_Real X, Y, Z;
    quat.GetEulerAngles(gp_Extrinsic_XYZ, X, Y, Z);
    QJsonObject obj;
    std::ostringstream ss;
    BRepTools::Write(normal->face(), ss);
    obj["face"] = QString::fromStdString(ss.str());
    const gp_Pnt2d uv = normal->get2dPnt();
    obj["U"] = static_cast <double> (uv.X());
    obj["V"] = static_cast <double> (uv.Y());
    obj["X"] = static_cast <double> (X);
    obj["Y"] = static_cast <double> (Y);
    obj["Z"] = static_cast <double> (Z);
    obj["Label"] = QString(normal->getLabel().ToCString());
    return obj;
}

Handle(InteractiveFaceNormal) InteractiveFaceNormalSerializer::deserialize(const QJsonObject &obj)
{
    const std::string faceStr = obj["face"].toString().toStdString();
    if (faceStr.empty()) {
        return nullptr;
    }

    TopoDS_Face face;
    BRep_Builder builder;
    std::istringstream ss(faceStr);
    BRepTools::Read(face, ss, builder);
    if (face.IsNull()) {
        return nullptr;
    }

    auto U = static_cast <Standard_Real> (obj["U"].toDouble());
    auto V = static_cast <Standard_Real> (obj["V"].toDouble());
    auto X = static_cast <Standard_Real> (obj["X"].toDouble());
    auto Y = static_cast <Standard_Real> (obj["Y"].toDouble());
    auto Z = static_cast <Standard_Real> (obj["Z"].toDouble());
    gp_Quaternion quat;
    quat.SetEulerAngles(gp_Extrinsic_XYZ, X, Y, Z);
    Handle(InteractiveFaceNormal) normal = new InteractiveFaceNormal(face, gp_Pnt2d(U, V), quat);
    normal->setLabel(obj["Label"].toString().toLocal8Bit().constData());
    return normal;
}
