#ifndef NORMALDETECTOR_H
#define NORMALDETECTOR_H

#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>

class NormalDetector
{
public:
    static gp_Dir getNormal(const TopoDS_Face &face, const gp_Pnt &point);
};

#endif // NORMALDETECTOR_H
