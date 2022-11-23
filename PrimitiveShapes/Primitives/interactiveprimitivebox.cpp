#include "interactiveprimitivebox.h"

#include <map>

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "interactivedimentionlenght.h"

class InteractivePrimitiveBoxPrivate
{
    friend class InteractivePrimitiveBox;

    TopoDS_Shape makeShape() const {
        return BRepPrimAPI_MakeBox(mWidth, mHeight, mDepth);
    }

    TopoDS_Face findFaceByPoint(const gp_Pnt &localPnt) const {
        BRepBuilderAPI_MakeVertex builder(localPnt);
        std::map <Standard_Real, TopoDS_Face> faces;
        for (TopExp_Explorer anExp(q->Shape(), TopAbs_FACE); anExp.More(); anExp.Next()) {
            auto &curFace = TopoDS::Face(anExp.Current());
            BRepExtrema_DistShapeShape extrema(curFace, builder.Vertex());
            if (extrema.IsDone()) {
                faces[extrema.Value()] = curFace;
            }
        }
        TopoDS_Face res;
        if (!faces.empty()) {
            res = faces.cbegin()->second;
        }
        return res;
    }

    std::vector <Handle(InteractiveDimentionLenght)> dimentions() const {
        return { mXDimension, mYDimension, mZDimension };
    }

    Bnd_Box boundingBox() const {
        return Bnd_Box(gp_Pnt(), gp_Pnt(mWidth, mHeight, mDepth));
    }

    void updateGeometry() {
        q->AIS_Shape::SetShape(makeShape());

        auto faceX0 = findFaceByPoint(gp_Pnt(0., mHeight / 2., mDepth / 2.));
        auto faceX1 = findFaceByPoint(gp_Pnt(mWidth, mHeight / 2., mDepth / 2.));
        mXDimension->SetMeasuredGeometry(faceX0, faceX1);
        auto faceY0 = findFaceByPoint(gp_Pnt(mWidth / 2., 0., mDepth / 2.));
        auto faceY1 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight, mDepth / 2.));
        mYDimension->SetMeasuredGeometry(faceY0, faceY1);
        auto faceZ0 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight / 2., 0.));
        auto faceZ1 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight / 2., mDepth));
        mZDimension->SetMeasuredGeometry(faceZ0, faceZ1);

        mBox = boundingBox();

        auto ctx = q->GetContext();
        if (ctx) {
            ctx->Redisplay(q, Standard_False);
            for (const auto &dimension : dimentions()) {
                ctx->RecomputePrsOnly(dimension, Standard_False);
            }
        }
    }

    void createXDimension() {
        auto face0 = findFaceByPoint(gp_Pnt(0., mHeight / 2., mDepth / 2.));
        auto face1 = findFaceByPoint(gp_Pnt(mWidth, mHeight / 2., mDepth / 2.));
        mXDimension = new InteractiveDimentionLenght(face0, face1);
    }

    void createYDimension() {
        auto face0 = findFaceByPoint(gp_Pnt(mWidth / 2., 0., mDepth / 2.));
        auto face1 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight, mDepth / 2.));
        mYDimension = new InteractiveDimentionLenght(face0, face1);
        mYDimension->SetFlyout(-15.);
    }

    void createZDimension() {
        auto face0 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight / 2., 0.));
        auto face1 = findFaceByPoint(gp_Pnt(mWidth / 2., mHeight / 2., mDepth));
        mZDimension = new InteractiveDimentionLenght(face0, face1);
        mZDimension->SetFlyout(-15.);
    }

    InteractivePrimitiveBox *q;
    Standard_Real mWidth = 100.;
    Standard_Real mHeight = 200.;
    Standard_Real mDepth = 300.;
    Standard_Real mMinLenght = 1.;
    Handle(InteractiveDimentionLenght) mXDimension, mYDimension, mZDimension;
    Bnd_Box mBox;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractivePrimitiveBox, InteractivePrimitive)

InteractivePrimitiveBox::InteractivePrimitiveBox()
    : InteractivePrimitive()
    , d(new InteractivePrimitiveBoxPrivate)
{
    d->q = this;

    AIS_Shape::SetShape(d->makeShape());

    d->mBox = d->boundingBox();

    d->createXDimension();
    d->createYDimension();
    d->createZDimension();
}

InteractivePrimitiveBox::~InteractivePrimitiveBox()
{
    delete d;
}

Standard_Real InteractivePrimitiveBox::getWidth() const
{
    return d->mWidth;
}

Standard_Real InteractivePrimitiveBox::getHeight() const
{
    return d->mHeight;
}

Standard_Real InteractivePrimitiveBox::getDepth() const
{
    return d->mDepth;
}

void InteractivePrimitiveBox::setWidth(Standard_Real width)
{
    d->mWidth = width < d->mMinLenght ? d->mMinLenght : width;
    d->updateGeometry();
}

void InteractivePrimitiveBox::setHeight(Standard_Real height)
{
    d->mHeight = height < d->mMinLenght ? d->mMinLenght : height;
    d->updateGeometry();
}

void InteractivePrimitiveBox::setDepth(Standard_Real depth)
{
    d->mDepth = depth < d->mMinLenght ? d->mMinLenght : depth;
    d->updateGeometry();
}

void InteractivePrimitiveBox::setAdvancedManipulatorsVisible(Standard_Boolean visible)
{
    auto ctx = GetContext();
    if (!ctx) {
        return;
    }

    if (visible) {
        if (!ctx->IsDisplayed(d->mXDimension)) {
            for (const auto &dimension : d->dimentions()) {
                dimension->SetZLayer(ZLayer());
                AddChild(dimension);
                ctx->Display(dimension, Standard_False);
                ctx->SetSelectionSensitivity(dimension, PrsDim_DimensionSelectionMode_All, 10);
            }
        }
        return;
    }

    if (!visible) {
        for (const auto &dimension : d->dimentions()) {
            if (ctx->IsDisplayed(dimension)) {
                ctx->Remove(dimension, Standard_False);
                RemoveChild(dimension);
            }
        }
    }
}

void InteractivePrimitiveBox::handleDimentionLenght(InteractiveDimentionLenght *dimension, Standard_Real value)
{
    if (dimension == d->mXDimension) {
        setWidth(value);
    } else if (dimension == d->mYDimension) {
        setHeight(value);
    } else if (dimension == d->mZDimension) {
        setDepth(value);
    }
}

const Bnd_Box &InteractivePrimitiveBox::BoundingBox()
{
    return d->mBox;
}
