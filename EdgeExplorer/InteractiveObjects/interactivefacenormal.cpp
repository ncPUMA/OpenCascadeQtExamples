#include "interactivefacenormal.h"

#include "interactivenormal.h"

#include <cassert>
#include <map>

#include <QDebug>

#include <Adaptor2d_HLine2d.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Extrema_ExtCC.hxx>
#include <Extrema_ExtElC.hxx>
#include <gce_MakeDir.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GC_MakeSegment.hxx>
#include <GeomAdaptor.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Line.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Quaternion.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <NCollection_String.hxx>
#include <ProjLib_HProjectedCurve.hxx>
#include <ProjLib_ProjectOnSurface.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_IsoAspect.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_ToolDisk.hxx>
#include <Select3D_SensitiveCircle.hxx>
#include <Select3D_SensitiveCurve.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <StdPrs_Curve.hxx>
#include <StdPrs_Isolines.hxx>
#include <StdPrs_ShadedSurface.hxx>
#include <TopoDS_Face.hxx>
#include <V3d_View.hxx>

class InteractiveFaceNormalPrivate
{
    friend class InteractiveFaceNormal;

    //! Auxiliary tool for filtering picking ray.
    class SensRotation
    {
    public:
        SensRotation(const gp_Dir &planeNormal)
            : mPlaneNormal(planeNormal)
            , mAngleTol(10.0 * M_PI / 180.0) {}

        //! Checks if picking ray can be used for detection.
        Standard_Boolean isValidRay (const SelectBasics_SelectingVolumeManager &volMgr) const {
            if (volMgr.GetActiveSelectionType() != SelectBasics_SelectingVolumeManager::Point) {
                return Standard_False;
            }
            const gp_Vec ray(volMgr.GetNearPickedPnt(), volMgr.GetFarPickedPnt());
            return !ray.IsNormal(mPlaneNormal, mAngleTol);
        }

    private:
        gp_Dir mPlaneNormal;
        Standard_Real mAngleTol;
    };

    //! Sensitive circle with filtering picking ray.
    class SensCircle : public Select3D_SensitiveCircle, public SensRotation
    {
    public:
        SensCircle(const Handle(SelectMgr_EntityOwner) &owner,
                   const gp_Circ &circle,
                   const Standard_Integer pointCount)
            : Select3D_SensitiveCircle(owner, circle, Standard_False, pointCount)
            , SensRotation(circle.Position().Direction()) { }

        //! Checks whether the circle overlaps current selecting volume
        virtual Standard_Boolean Matches(SelectBasics_SelectingVolumeManager& volMgr,
                                         SelectBasics_PickResult& pickResult) Standard_OVERRIDE {
            return isValidRay(volMgr) && Select3D_SensitiveCircle::Matches(volMgr, pickResult);
        }
    };

    //! Sensitive triangulation with filtering picking ray.
    class SensTriangulation : public Select3D_SensitiveTriangulation, public SensRotation
    {
    public:
        SensTriangulation(const Handle(SelectMgr_EntityOwner) &owner,
                          const Handle(Poly_Triangulation) &triangulation,
                          const gp_Dir &planeNormal)
            : Select3D_SensitiveTriangulation(owner, triangulation, TopLoc_Location(), Standard_True)
            , SensRotation(planeNormal) { }

        //! Checks whether the circle overlaps current selecting volume
        virtual Standard_Boolean Matches(SelectBasics_SelectingVolumeManager &volMgr,
                                         SelectBasics_PickResult &pickResult) Standard_OVERRIDE {
            return isValidRay(volMgr) && Select3D_SensitiveTriangulation::Matches(volMgr, pickResult);
        }
    };

    enum Aspects {
        AspectNormal,
        AspectNormalAngle,
        AspectULine,
        AspectVLine,
        AspectNormalRing,
        AspectAngleRing,
        AspectOpositeRing,
    };

    enum Selections {
        SelectionNormalAndAngle,
        SelectionULine,
        SelectionVLine,
        SelectionNormalRing,
        SelectionAngleRing,
        SelectionOpositeRing,
    };

    InteractiveFaceNormalPrivate(InteractiveFaceNormal *q_ptr, const TopoDS_Face &face, const gp_Pnt &pnt) {
        q = q_ptr;
        mFace = face;
        auto aSurf = BRep_Tool::Surface(mFace);
        Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
        mUV = surfAnalis->ValueOfUV(pnt, Precision::Confusion());
        q->SetInfiniteState();
        q->SetMutable(Standard_True);
        q->SetAutoHilight(Standard_False);
        auto textAspect = q->Attributes()->TextAspect();
        textAspect->SetHeight(mLabelHeight);
        q->Attributes()->SetTextAspect(textAspect);
    }

    void clearSelected() {
        for (auto &s : mSelections) {
            if (s.second) {
                s.second->SetSelected(false);
            }
        }

        for (const auto &p : mAspects) {
            Standard_Real alpha = 1.;
            switch (p.first) {
                case AspectNormal:
                case AspectNormalAngle:
                    break;
                case AspectULine:
                case AspectVLine:
                case AspectNormalRing:
                case AspectAngleRing:
                case AspectOpositeRing:
                    alpha = 0.;
                    break;
            }
            auto rgba = p.second->InteriorColorRGBA();
            rgba.SetAlpha(alpha);
            p.second->SetInteriorColor(rgba);
        }
    }

    void hilightSelected() {
        for (const auto &p : mAspects) {
            Standard_Real alpha = 1.;
            switch (p.first) {
                case AspectNormal:
                case AspectNormalAngle:
                    break;
                case AspectULine:
                case AspectVLine:
                case AspectNormalRing:
                case AspectAngleRing:
                case AspectOpositeRing:
                    alpha = 1.;
                    break;
            }
            auto rgba = p.second->InteriorColorRGBA();
            rgba.SetAlpha(alpha);
            p.second->SetInteriorColor(rgba);
        }
    }

    void drawHiglighted(const Handle(SelectMgr_EntityOwner) &owner,
                        const Handle(Prs3d_Presentation) &presentation,
                        const Quantity_Color &color) {
        gp_Pnt pos, normalEnd, angleEnd;
        calcNormalPoints(pos, normalEnd, angleEnd);
        gp_Dir normalDir, angleDir, opositeDir;
        calcNormalDirections(normalDir, angleDir, opositeDir);
        auto owId = SelectionNormalAndAngle;
        if (isSelected() && ownerId(owId, owner)) {
            switch (owId) {
                case SelectionNormalAndAngle:
                    break;
                case SelectionULine:
                    createULine(presentation, color, 1.);
                    break;
                case SelectionVLine:
                    createVLine(presentation, color, 1.);
                    break;
                case SelectionNormalRing:
                    createRing(presentation, normalDir, color, 1.);
                    break;
                case SelectionAngleRing:
                    createRing(presentation, angleDir, color, 1.);
                    break;
                case SelectionOpositeRing:
                    createRing(presentation, opositeDir, color, 1.);
                    break;
            }
        } else {
            gp_Pnt pos, normalEnd, angleEnd;
            calcNormalPoints(pos, normalEnd, angleEnd);
            createLine(presentation, pos, normalEnd, Quantity_NOC_RED);
            createLine(presentation, pos, angleEnd, Quantity_NOC_BLUE);
        }
    }

    void calcNormalPoints(gp_Pnt &pos, gp_Pnt &normalEnd, gp_Pnt &angleEnd) const {
        auto aSurf = BRep_Tool::Surface(mFace);
        Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
        pos = surfAnalis->Value(mUV);

        GeomLProp_SLProps props(aSurf, mUV.X(), mUV.Y(), 1, 0.01);
        gp_Dir normal = props.Normal();
        if (mFace.Orientation() == TopAbs_REVERSED || mFace.Orientation() == TopAbs_INTERNAL) {
            normal.Reverse();
        }
        gp_Trsf trsf;
        trsf.SetRotationPart(mRotation);
        normal.Transform(trsf);
        normalEnd = pos.Translated(normal.XYZ() * 5.);
        gp_Dir angle = props.D1U().Normalized();
        angle.Transform(trsf);
        angleEnd = pos.Translated(angle.XYZ() * 2.5);
    }

    void calcNormalDirections(gp_Dir &normal, gp_Dir &angle, gp_Dir &oposite) const {
        auto aSurf = BRep_Tool::Surface(mFace);
        Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
        const gp_Pnt pos = surfAnalis->Value(mUV);

        GeomLProp_SLProps props(aSurf, mUV.X(), mUV.Y(), 1, 0.01);
        normal = props.Normal();
        if (mFace.Orientation() == TopAbs_REVERSED || mFace.Orientation() == TopAbs_INTERNAL) {
            normal.Reverse();
        }
        gp_Trsf trsf;
        trsf.SetRotationPart(mRotation);
        normal.Transform(trsf);
        angle = props.D1U().Normalized();
        angle.Transform(trsf);
        oposite = props.D1V().Normalized();
        oposite.Transform(trsf);
    }

    void computeSelection(const Handle(SelectMgr_Selection) &selection) {qDebug() << "CompSel";
        Standard_Boolean bHasSelection = Standard_False;
        std::map <Selections, Standard_Boolean> selected;
        for (const auto &s : qAsConst(mSelections)) {
            const Standard_Boolean val = s.second && s.second->IsSelected();
            selected[s.first] = val;
            bHasSelection |= val;
            if (s.second) {
                s.second->SetSelected(Standard_False);
            }
        }

        selection->Clear();
        mSelections.clear();

        if (!bHasSelection) {
            // NormalAndAngle
            Handle(SelectMgr_EntityOwner) owner = new SelectMgr_EntityOwner(q);
            owner->SetComesFromDecomposition(Standard_True);

            gp_Pnt pos, normalEnd, angleEnd;
            calcNormalPoints(pos, normalEnd, angleEnd);
            Handle(Select3D_SensitiveSegment) sensN1 =
                    new Select3D_SensitiveSegment(owner, pos, normalEnd);
            sensN1->SetSensitivityFactor(5);
            selection->Add(sensN1);
            Handle(Select3D_SensitiveSegment) sensN2 =
                    new Select3D_SensitiveSegment(owner, pos, angleEnd);
            sensN2->SetSensitivityFactor(5);
            selection->Add(sensN2);
            Standard_Boolean isSelected = Standard_False;
            for (const auto &p : selected) {
                if ((p.first == SelectionNormalAndAngle
                     || p.first == SelectionULine
                     || p.first == SelectionVLine) && p.second) {
                    isSelected = Standard_True;
                    break;
                }
            }
            owner->SetSelected(isSelected);
            mSelections[SelectionNormalAndAngle] = owner;
        } else {
            // ULine
            {
                Handle(SelectMgr_EntityOwner) owner = new SelectMgr_EntityOwner(q);
                owner->SetComesFromDecomposition(Standard_True);
                Standard_Real u1, u2, v1, v2;
                BRepTools::UVBounds(mFace, u1, u2, v1, v2);
                const Adaptor2d_Line2d line(mUV, gp_Vec2d(mUV, gp_Pnt2d(u2, mUV.Y())), 0, Abs(u2 - u1) / 5.);
                const Adaptor3d_CurveOnSurface curveOnSurf(new Adaptor2d_HLine2d(line), new BRepAdaptor_HSurface(mFace));
                Handle(Select3D_SensitiveCurve) sens =
                        new Select3D_SensitiveCurve(owner, GeomAdaptor::MakeCurve(curveOnSurf), 50);
                sens->SetSensitivityFactor(15);
                selection->Add(sens);
                Standard_Boolean isSelected = Standard_False;
                for (const auto &p : selected) {
                    if ((p.first == SelectionNormalAndAngle
                         || p.first == SelectionULine) && p.second) {
                        isSelected = Standard_True;
                        break;
                    }
                }
                owner->SetSelected(isSelected);
                mSelections[SelectionULine] = owner;
            }
            // VLine
            {
                Handle(SelectMgr_EntityOwner) owner = new SelectMgr_EntityOwner(q);
                owner->SetComesFromDecomposition(Standard_True);
                Standard_Real u1, u2, v1, v2;
                BRepTools::UVBounds(mFace, u1, u2, v1, v2);
                const Adaptor2d_Line2d line(mUV, gp_Vec2d(mUV, gp_Pnt2d(mUV.X(), v2)), 0, Abs(v2 - v1) / 5.);
                const Adaptor3d_CurveOnSurface curveOnSurf(new Adaptor2d_HLine2d(line), new BRepAdaptor_HSurface(mFace));
                Handle(Select3D_SensitiveCurve) sens =
                        new Select3D_SensitiveCurve(owner, GeomAdaptor::MakeCurve(curveOnSurf), 50);
                sens->SetSensitivityFactor(15);
                selection->Add(sens);
                auto it = selected.find(SelectionVLine);
                owner->SetSelected(it != selected.cend() && it->second);
                mSelections[SelectionVLine] = owner;
            }
            // Rings
            {
                auto aSurf = BRep_Tool::Surface(mFace);
                Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
                const gp_Pnt pos = surfAnalis->Value(mUV);
                gp_Dir normalDir, angleDir, opositeDir;
                calcNormalDirections(normalDir, angleDir, opositeDir);
                for (int i = SelectionNormalRing; i <= SelectionOpositeRing; ++i) {
                    const Selections curSelectionType = static_cast <Selections> (i);
                    gp_Dir rotateDir = normalDir;
                    if (curSelectionType == SelectionAngleRing) {
                        rotateDir = angleDir;
                    } else if (curSelectionType == SelectionOpositeRing) {
                        rotateDir = opositeDir;
                    }

                    Handle(SelectMgr_EntityOwner) owner = new SelectMgr_EntityOwner(q);
                    owner->SetComesFromDecomposition(Standard_True);
                    const gp_Circ circle(gp_Ax2(pos, rotateDir), mRingRadius);
                    Handle(SensCircle) sensCircle = new SensCircle(owner, circle, mFacetesCount);
                    sensCircle->SetSensitivityFactor(10);
                    selection->Add(sensCircle);
                    // enlarge sensitivity by triangulation
                    const Prs3d_ToolDisk tool(mRingRadius, mRingRadius + mRingWidth, mFacetesCount * 2, mFacetesCount * 2);
                    const gp_Ax3 aSystem(pos, rotateDir);
                    gp_Trsf trsf;
                    trsf.SetTransformation(aSystem, gp_Ax3());
                    auto triangulation = tool.CreatePolyTriangulation(trsf);
                    Handle(SensTriangulation) sensTr = new SensTriangulation(owner, triangulation, rotateDir);
                    selection->Add(sensTr);
                    auto it = selected.find(curSelectionType);
                    owner->SetSelected(it != selected.cend() && it->second);
                    mSelections[curSelectionType] = owner;
                }
            }
        }
    }

    void compute(const Handle(Prs3d_Presentation) &presentation,
                 const Quantity_Color &selectionColor) {
        gp_Pnt pos, normalEnd, angleEnd;
        calcNormalPoints(pos, normalEnd, angleEnd);
        mAspects[AspectNormal] = createLine(presentation, pos, normalEnd, mNormalColor);
        mAspects[AspectNormalAngle] = createLine(presentation, pos, angleEnd, mNormalAngleColor);
        Standard_Real alpha = isSelected() ? 1. : 0.;
        mAspects[AspectULine] = createULine(presentation, selectionColor, alpha);
        mAspects[AspectVLine] = createVLine(presentation, selectionColor, alpha);
        gp_Dir normalDir, angleDir, opositeDir;
        calcNormalDirections(normalDir, angleDir, opositeDir);
        mAspects[AspectAngleRing] = createRing(presentation, normalDir, Quantity_NOC_GRAY40, alpha);
//        mAspects[AspectNormalRing] = createRing(presentation, angleDir, Quantity_NOC_GRAY60, alpha);
//        mAspects[AspectOpositeRing] = createRing(presentation, opositeDir, selectionColor, alpha);
    }

    Handle(Graphic3d_Aspects) createLine(const Handle(Prs3d_Presentation) &presentation,
                                         const gp_Pnt &start,
                                         const gp_Pnt &end,
                                         const Quantity_Color &color) {
        auto group = presentation->NewGroup();
        Handle(Graphic3d_AspectLine3d) aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        group->SetGroupPrimitivesAspect(aspect);
        Handle(Graphic3d_ArrayOfPolylines) array = new Graphic3d_ArrayOfPolylines(2);
        array->AddVertex(start);
        array->AddVertex(end);
        group->AddPrimitiveArray(array);
        return aspect;
    }

    Handle(Graphic3d_Aspects) createULine(const Handle(Prs3d_Presentation) &presentation,
                                          const Quantity_Color &color,
                                          const Standard_Real alpha) {
        Handle(Prs3d_Drawer) drawer = new Prs3d_Drawer();
        Handle(Graphic3d_AspectLine3d) aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        aspect->SetInteriorColor(Quantity_ColorRGBA(aspect->Color(), alpha));
        drawer->SetLineAspect(new Prs3d_LineAspect(aspect));

        auto surf = BRep_Tool::Surface(mFace);
        Standard_Real u1, u2, v1, v2;
        surf->Bounds(u1, u2, v1, v2);
        Standard_Real lenK = 1.;
        if (surf->IsUClosed()) {
            lenK /= surf->Value(u1, v1).Distance(surf->Value(u1 + 1, v1));
        }
        Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(mUV, gp_Pnt2d(mUV.X() + mLen * lenK, mUV.Y()));
        const Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(mFace));
        StdPrs_Curve::Add(presentation, curveOnSurf, drawer);
        return aspect;
    }

    Handle(Graphic3d_Aspects) createVLine(const Handle(Prs3d_Presentation) &presentation,
                                          const Quantity_Color &color,
                                          const Standard_Real alpha) {
        Handle(Prs3d_Drawer) drawer = new Prs3d_Drawer();
        Handle(Graphic3d_AspectLine3d) aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        aspect->SetInteriorColor(Quantity_ColorRGBA(aspect->Color(), alpha));
        drawer->SetLineAspect(new Prs3d_LineAspect(aspect));

        auto surf = BRep_Tool::Surface(mFace);
        Standard_Real u1, u2, v1, v2;
        surf->Bounds(u1, u2, v1, v2);
        Standard_Real lenK = 1.;
        if (surf->IsVClosed()) {
            lenK /= surf->Value(u1, v1).Distance(surf->Value(u1, v1 + 1.));
        }
        Handle(Geom2d_TrimmedCurve) curve = GCE2d_MakeSegment(mUV, gp_Pnt2d(mUV.X(), mUV.Y() + mLen * lenK));
        const Adaptor3d_CurveOnSurface curveOnSurf(new Geom2dAdaptor_HCurve(curve), new BRepAdaptor_HSurface(mFace));
        StdPrs_Curve::Add(presentation, curveOnSurf, drawer);
        return aspect;
    }

    Handle(Graphic3d_Aspects) createRing(const Handle(Prs3d_Presentation) &presentation,
                                         const gp_Dir &rotateDir,
                                         const Quantity_Color &color,
                                         const Standard_Real alpha) {
        auto group = presentation->NewGroup();
        Handle(Graphic3d_AspectLine3d) aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        aspect->SetInteriorColor(Quantity_ColorRGBA(aspect->Color(), alpha));
        group->SetClosed(Standard_True);
        group->SetGroupPrimitivesAspect(aspect);
        Prs3d_ToolDisk tool(mRingRadius, mRingRadius + mRingWidth, mFacetesCount, mFacetesCount);
        auto aSurf = BRep_Tool::Surface(mFace);
        Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
        const gp_Pnt pos = surfAnalis->Value(mUV);
        const gp_Ax3 aSystem(pos, rotateDir);
        gp_Trsf trsf;
        trsf.SetTransformation(aSystem, gp_Ax3());
        auto triangles = tool.CreateTriangulation(trsf);
        group->AddPrimitiveArray(triangles);
        return aspect;
    }

    bool isSelected() const {
        for (const auto &s : mSelections) {
            if (s.second && s.second->IsSelected()) {
                return true;
            }
        }
        return false;
    }

    bool ownerId(Selections &selected, const Handle(SelectMgr_EntityOwner) &owner) const {
        auto hlOwnerIt = std::find_if(mSelections.cbegin(),
                                      mSelections.cend(),
                                      [&owner](const auto &pair){
            return pair.second == owner;
        });
        if (hlOwnerIt != mSelections.cend()) {
            selected = hlOwnerIt->first;
            return true;
        }
        return false;
    }

    //! Return Ax1 for specified direction of Ax2.
    inline static gp_Ax1 getAx1FromAx2Dir(const gp_Ax2 &axis, int index) {
      switch (index) {
        case 0: return gp_Ax1(axis.Location(), axis.XDirection());
        case 1: return gp_Ax1(axis.Location(), axis.YDirection());
        case 2: return axis.Axis();
      }
      assert(false); // getAx1FromAx2Dir: Invalid axis index
      return axis.Axis();
    }

    bool processDragActionUpdate(const Handle(V3d_View) &view,
                                 const Graphic3d_Vec2i &from,
                                 const Graphic3d_Vec2i &to,
                                 const Selections ownerId) {
        // Get 3d points with projection vectors
        Graphic3d_Vec3d fromPnt, fromProj;
        view->ConvertWithProj(from.x(), from.y(),
                              fromPnt.x(), fromPnt.y(), fromPnt.z(),
                              fromProj.x(), fromProj.y(), fromProj.z());
        gp_Lin fromLine(gp_Pnt(fromProj.x(), fromProj.y(), fromProj.z()),
                        gp_Dir(fromPnt.x(), fromPnt.y(), fromPnt.z()));
        Graphic3d_Vec3d toPnt, toProj;
        view->ConvertWithProj(to.x(), to.y(),
                              toPnt.x(), toPnt.y(), toPnt.z(),
                              toProj.x(), toProj.y(), toProj.z());
        gp_Lin toLine(gp_Pnt(toProj.x(), toProj.y(), toProj.z()),
                      gp_Dir(toPnt.x(), toPnt.y(), toPnt.z()));
        gp_Trsf trsf = parentTransform().Inverted();
        fromLine.Transform(trsf);
        toLine.Transform(trsf);
        auto aSurf = BRep_Tool::Surface(mFace);

        switch (ownerId) {
            case SelectionNormalAndAngle:
                break;

            case SelectionULine:
            case SelectionVLine: {
                GeomAdaptor_Curve fromCurve(new Geom_Line(fromLine));
                GeomAPI_IntCS fromIntersect(new Geom_Line(fromLine), aSurf);
                GeomAPI_IntCS toIntersect(new Geom_Line(toLine), aSurf);
                if (!fromIntersect.IsDone() || fromIntersect.NbPoints() == 0
                        || !toIntersect.IsDone() || toIntersect.NbPoints() == 0) {
                    return false;
                }

                for (int i = 1; i <= fromIntersect.NbPoints(); ++i) {
                    Standard_Real fromU, fromV, fromW;
                    fromIntersect.Parameters(i, fromU, fromV, fromW);
                    qDebug() << i << fromU << fromV;
                }

                Standard_Real fromU, fromV, fromW;
                fromIntersect.Parameters(1, fromU, fromV, fromW);
                Standard_Real toU, toV, toW;
                toIntersect.Parameters(1, toU, toV, toW);
                gp_Pnt2d newUV;
                if (ownerId == SelectionULine) {
                    newUV = gp_Pnt2d(mStartDragUV.X() + toU - fromU, mStartDragUV.Y());
                    qDebug() << mStartDragUV.X() << fromU << toU;
                } else {
                    newUV = gp_Pnt2d(mStartDragUV.X(), mStartDragUV.Y() + toV - fromV);
                    qDebug() << mStartDragUV.Y() << fromV << toV;
                }
                Standard_Real u1, u2, v1, v2;
                BRepTools::UVBounds(mFace, u1, u2, v1, v2);
                if (newUV.Distance(mUV) < Precision::Confusion()
                        || newUV.X() < u1 || newUV.X() > u2
                        || newUV.Y() < v1 || newUV.Y() > v2) {
                    return false;
                }
                mUV = newUV;
                return true;
            }

            case SelectionNormalRing:
            case SelectionAngleRing:
            case SelectionOpositeRing: {
                gp_Dir normalDir, angleDir, opositeDir;
                calcNormalDirections(normalDir, angleDir, opositeDir);
                gp_Dir dir = normalDir;
                int axisIndex = 0;
                if (ownerId == SelectionAngleRing) {
                    dir = angleDir;
                    axisIndex = 1;
                } else if (ownerId == SelectionOpositeRing) {
                    dir = opositeDir;
                    axisIndex = 2;
                }

                Handle(ShapeAnalysis_Surface) surfAnalis = new ShapeAnalysis_Surface(aSurf);
                const gp_Pnt posLoc = surfAnalis->Value(mUV);
                const gp_Ax2 startPos(posLoc, dir);
                const gp_Ax1 currAxis = getAx1FromAx2Dir(startPos, axisIndex);
                IntAna_IntConicQuad crossFrom(fromLine,
                                              gp_Pln(posLoc, startPos.Direction()),
                                              Precision::Angular(),
                                              Precision::Intersection());
                IntAna_IntConicQuad crossTo(toLine,
                                            gp_Pln(posLoc, startPos.Direction()),
                                            Precision::Angular(),
                                            Precision::Intersection());
                if (!crossFrom.IsDone() || crossFrom.IsParallel() || crossFrom.NbPoints() < 1 ||
                        !crossTo.IsDone() || crossTo.IsParallel() || crossTo.NbPoints() < 1) {
                    return false;
                }

                const gp_Pnt fromPos = crossFrom.Point(1);
                const gp_Pnt toPos = crossTo.Point(1);
                if (toPos.Distance(fromPos) < Precision::Confusion()) {
                    return false;
                }

                const gp_Dir startAxis = gce_MakeDir(posLoc, fromPos);
                const gp_Dir toAxis = posLoc.IsEqual(fromPos, Precision::Confusion())
                  ? getAx1FromAx2Dir(startPos, (axisIndex + 1) % 3).Direction()
                  : gce_MakeDir(posLoc, fromPos);

                const gp_Dir currentAxis = gce_MakeDir(posLoc, toPos);
                Standard_Real angle = startAxis.AngleWithRef(currentAxis, currAxis.Direction());
                const Standard_Real prevAngle = startAxis.AngleWithRef(gce_MakeDir(posLoc, fromPos), currAxis.Direction());

                // Change value of an angle if it should have different sign.
                if (angle * prevAngle < 0 && Abs(angle) < M_PI_2) {
                    Standard_Real sign = prevAngle > 0 ? -1.0 : 1.0;
                    angle = sign * (M_PI * 2 - angle);
                }

                if (Abs(angle) < Precision::Confusion()) {
                    return false;
                }

                gp_Quaternion quat;
                qDebug() << currAxis.Direction().X() << currAxis.Direction().Y() << currAxis.Direction().Z() << ownerId << angle * 180. / M_PI;
                quat.SetVectorAndAngle(dir, angle);
                mRotation += quat;
                return true;
            }
        }
        return false;
    }

    gp_Trsf parentTransform() const {
        gp_Trsf combinedParent;
        if (q->CombinedParentTransformation()) {
            combinedParent = q->CombinedParentTransformation()->Trsf();
        }
        return combinedParent;
    }

    InteractiveFaceNormal *q = nullptr;
    TopoDS_Face mFace;
    gp_Pnt2d mUV;
    gp_Quaternion mRotation;
    Quantity_Color mNormalColor = Quantity_NOC_YELLOW;
    Quantity_Color mNormalAngleColor = Quantity_NOC_BLUE;
    Standard_Real mLen = 5.;
    Standard_Real mWidth = 2.;
    Standard_Real mRingRadius = 5.5;
    Standard_Real mRingWidth = 1.;
    Standard_Integer mFacetesCount = 24;
    TCollection_ExtendedString mLabel;
    Standard_Real mLabelHeight = 20.;

    std::map <Aspects, Handle(Graphic3d_Aspects)> mAspects;
    std::map <Selections, Handle(SelectMgr_EntityOwner)> mSelections;

    gp_Pnt2d mStartDragUV;
    gp_Quaternion mStartDragRotation;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveFaceNormal, AIS_InteractiveObject)

InteractiveFaceNormal::InteractiveFaceNormal(const TopoDS_Face &face, const gp_Pnt &pntOnFace)
    : AIS_InteractiveObject()
    , d(new InteractiveFaceNormalPrivate(this, face, pntOnFace))
{

}

InteractiveFaceNormal::~InteractiveFaceNormal()
{
    delete d;
}

void InteractiveFaceNormal::setLabel(const TCollection_AsciiString &txt)
{
    d->mLabel = txt;
}

void InteractiveFaceNormal::HilightSelected(const Handle(PrsMgr_PresentationManager) &,
                                             const SelectMgr_SequenceOfOwner &)
{
    d->hilightSelected();
    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputeSelectionOnly(this);
    }
}

void InteractiveFaceNormal::ClearSelected()
{
    d->clearSelected();
    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputeSelectionOnly(this);
    }
}

void InteractiveFaceNormal::HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
                                                   const Handle(Prs3d_Drawer) &theStyle,
                                                   const Handle(SelectMgr_EntityOwner) &theOwner)
{
    auto presentation = GetHilightPresentation(thePM);
    if (presentation.IsNull()) {
        return;
    }

    presentation->Clear();
    Quantity_Color highlightColor = Quantity_NOC_CYAN1;
    if (myDynHilightDrawer) {
        highlightColor = myDynHilightDrawer->Color();
    } else {
        auto ctx = GetContext();
        if (ctx) {
            highlightColor = ctx->HighlightStyle()->Color();
        }
    }
    d->drawHiglighted(theOwner, presentation, highlightColor);
    auto layer = theStyle->ZLayer() != Graphic3d_ZLayerId_UNKNOWN
            ? theStyle->ZLayer()
            : myDrawer->ZLayer();
    if (presentation->GetZLayer() != layer) {
        presentation->SetZLayer(layer);
    }
    presentation->Highlight(theStyle);
    thePM->AddToImmediateList(presentation);
}

void InteractiveFaceNormal::ComputeSelection(const Handle(SelectMgr_Selection) &selection,
                                              const Standard_Integer mode)
{
    if (mode == 2) { // why ???
        d->computeSelection(selection);
    }
}

Standard_Boolean InteractiveFaceNormal::ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                                         const Handle(V3d_View) &view,
                                                         const Handle(SelectMgr_EntityOwner) &owner,
                                                         const Graphic3d_Vec2i &from,
                                                         const Graphic3d_Vec2i &to,
                                                         const AIS_DragAction action)
{
    auto owId = d->SelectionNormalAndAngle;
    if (!d->isSelected() || !d->ownerId(owId, owner)) {
        return Standard_False;
    }

    switch (action) {
        case AIS_DragAction_Start:
            d->mStartDragUV = d->mUV;
            d->mStartDragRotation = d->mRotation;
            break;

        case AIS_DragAction_Update:
            if (d->processDragActionUpdate(view, from, to, owId)) {
                context->RecomputePrsOnly(this, true);
                HilightOwnerWithColor(Presentations().First()->PresentationManager(),
                                      context->HighlightStyle(),
                                      owner);
            }
            break;

        case AIS_DragAction_Stop:
            context->RecomputeSelectionOnly(this);
            break;

        case AIS_DragAction_Abort:
            d->mUV = d->mStartDragUV;
            d->mRotation = d->mStartDragRotation;
            break;
    }
    return Standard_True;
}

void InteractiveFaceNormal::Compute(const Handle(PrsMgr_PresentationManager) &,
                                     const Handle(Prs3d_Presentation) &presentation,
                                     const Standard_Integer theMode)
{
    if (theMode != 0) {
      return;
    }

    Quantity_Color selectionColor = Quantity_NOC_GRAY80;
    if (myHilightDrawer) {
        selectionColor = myHilightDrawer->Color();
    } else {
        auto ctx = GetContext();
        if (ctx) {
            selectionColor = ctx->SelectionStyle()->Color();
        }
    }

    Handle(Prs3d_Drawer) drawer = new Prs3d_Drawer();
    drawer->SetUIsoAspect(new Prs3d_IsoAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 2., 1));
    drawer->SetVIsoAspect(new Prs3d_IsoAspect(Quantity_NOC_BLUE, Aspect_TOL_SOLID, 2., 1));
    StdPrs_Isolines::Add(presentation, d->mFace, drawer, 0.);

    d->compute(presentation, selectionColor);
}
