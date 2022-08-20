#ifndef NORMALDETECTOR_H
#define NORMALDETECTOR_H

#include <vector>

#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>

class NormalDetector
{
public:
    static gp_Dir getNormal(const TopoDS_Face &face, const gp_Pnt &point);
    static std::vector <gp_Dir> getNormals(const TopoDS_Face &face, const std::vector <gp_Pnt> &points);
};

#endif // NORMALDETECTOR_H
