#include "normaldetector.h"

#include <QDebug>

#include <BRepAdaptor_Surface.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomLProp_SLProps.hxx>
#include <ShapeAnalysis_Surface.hxx>

//static const gp_Pnt ProjectPoint(const gp_Pnt &p, const TopoDS_Face &face)
//{
//    gp_Pnt resultpoint;
//    Handle_Geom_Surface surface = BRep_Tool::Surface(face);
//    GeomAPI_ProjectPointOnSurf Proj(p, surface);
//    if (Proj.NbPoints() > 0) {
//        resultpoint = Proj.NearestPoint();
//    }
//    return resultpoint;
//}

//static double map(double value, double low, double high, double tlow, double thigh)
//{
//    double mapped = (((value - low) / (high - low)) * (thigh - tlow)) + tlow;
//    return mapped;
//}

//static gp_Pnt2d Get2dPntonSurfacefromPoint(const TopoDS_Face &face, const gp_Pnt &point)
//{
//    Handle(Geom_Surface) aSurf = BRep_Tool::Surface(face);

//    Standard_Real u1, u2, v1, v2;
//    BRepTools::UVBounds(face, u1, u2, v1, v2);

//    Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
//    gp_Pnt2d pUV = surfAnalis->ValueOfUV(point, Precision::Confusion());
//    double newx = map(pUV.X(), u1, u2, 0, 1);
//    double newy = map(pUV.Y(), v1, v2, 0, 1);
//    pUV.SetX(newx);
//    pUV.SetY(newy);
//    return pUV;
//}

//gp_Vec NormalDetector::getNormal(const TopoDS_Face &face, const gp_Pnt &point)
//{
//    // Get 2d UV data
//    gp_Pnt2d pUV = Get2dPntonSurfacefromPoint(face, point);

//    BRepAdaptor_Surface surface(face);
//    gp_Vec ut, vt;
//    gp_Pnt pt;
//    surface.D1(pUV.X(), pUV.Y(), pt, ut, vt);
//    gp_Vec V = ut.Crossed(vt);

//    Standard_Real mod = V.Magnitude();
//    if (mod < Precision::Confusion()) {
//        qDebug() << "Vector has no Magnitude" ;
//    }
//    // consider the face orientation
//    if (face.Orientation() == TopAbs_REVERSED || face.Orientation() == TopAbs_INTERNAL) {
//        V = -V;
//    }
//    return V;
//}

gp_Dir NormalDetector::getNormal(const TopoDS_Face &face, const gp_Pnt &point)
{
    auto aSurf = BRep_Tool::Surface(face);
    Standard_Real u1, u2, v1, v2;
    BRepTools::UVBounds(face, u1, u2, v1, v2);

    auto surfAnalis = new ShapeAnalysis_Surface(aSurf);
    const gp_Pnt2d pUV = surfAnalis->ValueOfUV(point, Precision::Confusion());

    GeomLProp_SLProps props(aSurf, pUV.X(), pUV.Y(), 1, 0.01);
    gp_Dir normal = props.Normal();
    if (face.Orientation() == TopAbs_REVERSED || face.Orientation() == TopAbs_INTERNAL) {
        normal.Reverse();
    }
    return normal;
}

std::vector<gp_Dir> NormalDetector::getNormals(const TopoDS_Face &face, const std::vector<gp_Pnt> &points)
{
    std::vector <gp_Dir> normals;
    normals.reserve(points.size());

    auto aSurf = BRep_Tool::Surface(face);
    Standard_Real u1, u2, v1, v2;
    BRepTools::UVBounds(face, u1, u2, v1, v2);

    auto surfAnalis = new ShapeAnalysis_Surface(aSurf);
    for (const auto &point : points) {
        const gp_Pnt2d pUV = surfAnalis->ValueOfUV(point, Precision::Confusion());
        GeomLProp_SLProps props(aSurf, pUV.X(), pUV.Y(), 1, 0.01);
        gp_Dir normal = props.Normal();
        if (face.Orientation() == TopAbs_REVERSED || face.Orientation() == TopAbs_INTERNAL) {
            normal.Reverse();
        }
        normals.push_back(normal);
    }
    return normals;
}
