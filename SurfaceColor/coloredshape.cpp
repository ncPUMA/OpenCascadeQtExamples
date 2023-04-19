#include "coloredshape.h"

#include <algorithm>

#include <AIS_DisplayMode.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <StdPrs_ToolTriangulatedShape.hxx>
#include <StdPrs_WFShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

class ColoredShapePrivate
{
    friend class ColoredShape;

    void setFaceColor(const TopoDS_Face &face, const Quantity_Color &color) {
        auto it = std::find_if(coloredFaces.cbegin(), coloredFaces.cend(), [&face](const ColorOfFace &colorOfFace) {
            return colorOfFace.face == face;
        });
        if (it != coloredFaces.cend()) {
            coloredFaces.erase(it);
        }
        coloredFaces.push_back({face, color});
    }

    bool faceColor(const TopoDS_Face &face, Quantity_Color &color) {
        auto it = std::find_if(coloredFaces.cbegin(), coloredFaces.cend(), [&face](const ColorOfFace &colorOfFace) {
            return colorOfFace.face == face;
        });
        if (it != coloredFaces.cend()) {
            color = it->color;
            return true;
        }
        return false;
    }

    struct ColorOfFace
    {
        TopoDS_Face face;
        Quantity_Color color;
    };
    std::vector <ColorOfFace> coloredFaces;
};

IMPLEMENT_STANDARD_RTTIEXT(ColoredShape, AIS_Shape)

ColoredShape::ColoredShape(const TopoDS_Shape &shape)
    : AIS_Shape(shape)
    , d(new ColoredShapePrivate)
{

}

ColoredShape::~ColoredShape()
{
    delete d;
}

void ColoredShape::setFaceColor(const TopoDS_Face &face, const Quantity_Color &color)
{
    d->setFaceColor(face, color);
}

void ColoredShape::resetColors()
{
    d->coloredFaces.clear();
}

void ColoredShape::Compute(const Handle(PrsMgr_PresentationManager3d) &prsMgr,
                           const Handle(Prs3d_Presentation) &prs,
                           const Standard_Integer mode) {
    if (mode != AIS_Shaded) {
        AIS_Shape::Compute(prsMgr, prs, mode);
        return;
    }

    StdPrs_ToolTriangulatedShape::ClearOnOwnDeflectionChange (myshape, myDrawer, Standard_True);
    if ((Standard_Integer) myshape.ShapeType() > 4) {
        StdPrs_WFShape::Add(prs, myshape, myDrawer);
    } else {
        if (IsInfinite()) {
            StdPrs_WFShape::Add(prs, myshape, myDrawer);
        } else {
            for (TopExp_Explorer anExp(myshape, TopAbs_EDGE); anExp.More(); anExp.Next()) {
                auto &curEdge = TopoDS::Edge(anExp.Current());
                StdPrs_ShadedShape::Add(prs, curEdge, myDrawer);
            }

            for (TopExp_Explorer anExp(myshape, TopAbs_FACE); anExp.More(); anExp.Next()) {
                auto &curFace = TopoDS::Face(anExp.Current());
                Quantity_Color faceColor;
                if (d->faceColor(curFace, faceColor)) {
                    auto triangles = StdPrs_ShadedShape::FillTriangles(curFace);
                    if (!triangles.IsNull()) {
                        Handle(Graphic3d_Group) aGroup = prs->NewGroup();
                        aGroup->SetClosed(Standard_False);
                        Handle(Prs3d_ShadingAspect) aspect = new Prs3d_ShadingAspect();
                        aspect->SetColor(faceColor);
                        aGroup->SetGroupPrimitivesAspect(aspect->Aspect());
                        aGroup->AddPrimitiveArray(triangles);
                    }
                } else {
                    StdPrs_ShadedShape::Add(prs, curFace, myDrawer);
                }
            }
        }
    }
    Standard_Real aTransparency = Transparency() ;
    if (aTransparency > 0.0)
    {
        SetTransparency (aTransparency);
    }
}
