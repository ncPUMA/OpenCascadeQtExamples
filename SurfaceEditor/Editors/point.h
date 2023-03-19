#ifndef POINT_H
#define POINT_H

#include <AIS_Point.hxx>

class Point : public AIS_Point
{
    DEFINE_STANDARD_RTTIEXT(Point, AIS_Point)

public:
    Point();

protected:
    void ComputeSelection(const Handle(SelectMgr_Selection) &, const Standard_Integer) Standard_OVERRIDE;
};

DEFINE_STANDARD_HANDLE(Point, AIS_Point)

#endif // POINT_H
