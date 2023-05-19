#include "point.h"

#include <Geom_CartesianPoint.hxx>

using namespace ExamplesBase;

IMPLEMENT_STANDARD_RTTIEXT(Point, AIS_Point)

Point::Point()
    : AIS_Point(new Geom_CartesianPoint(gp_Pnt()))
{

}

void Point::ComputeSelection(const Handle(SelectMgr_Selection) &, const Standard_Integer)
{

}
