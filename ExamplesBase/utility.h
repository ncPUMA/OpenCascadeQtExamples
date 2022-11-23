#ifndef UTILITY_H
#define UTILITY_H

#include "ExamplesBase_global.h"

class gp_Pnt;
class TopoDS_Edge;
class TopoDS_Face;
class TopoDS_Shape;

namespace ExamplesBase {

    TopoDS_Face EXAMPLESBASE_EXPORT findFaceByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt);
    TopoDS_Edge EXAMPLESBASE_EXPORT findEdgeByPoint(const TopoDS_Shape &shape, const gp_Pnt &localPnt);

}

#endif // UTILITY_H
