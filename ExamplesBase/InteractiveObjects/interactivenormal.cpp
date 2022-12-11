#include "interactivenormal.h"

#include <cassert>
#include <map>

#include <AIS_InteractiveContext.hxx>
#include <Extrema_ExtElC.hxx>
#include <gce_MakeDir.hxx>
#include <gp_Quaternion.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <NCollection_String.hxx>
#include <Select3D_SensitiveCircle.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_ToolDisk.hxx>
#include <Standard_Version.hxx>
#include <V3d_View.hxx>

namespace ExamplesBase {

class InteractiveNormalPrivate
{
    friend class InteractiveNormal;

    //! Auxiliary tool for filtering picking ray.
    class SensRotation
    {
    public:
        SensRotation(const gp_Dir &planeNormal)
            : mPlaneNormal(planeNormal)
            , mAngleTol(10.0 * M_PI / 180.0) {}

        //! Checks if picking ray can be used for detection.
        Standard_Boolean isValidRay (const SelectBasics_SelectingVolumeManager &volMgr) const {
#if OCC_VERSION_HEX > 0x070500
            auto selectionType = SelectMgr_SelectionType_Point;
#else
            auto selectionType = SelectBasics_SelectingVolumeManager::Point;
#endif
            if (volMgr.GetActiveSelectionType() != selectionType) {
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
            : Select3D_SensitiveCircle(owner, circle, Standard_False)
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

    struct Primitive
    {
        Handle(Graphic3d_Group) group;
        Handle(Graphic3d_Aspects) aspect;
    };

    enum Primitives {
        PrimitiveNormal,
        PrimitiveAngle,
        PrimitiveNormalArrow,
        PrimitiveAngleArrow,
        PrimitiveNormalRing,
        PrimitiveAngleRing,
        PrimitiveLabel,
    };

    enum Selections {
        SelectionFirst,

        SelectionNormal = SelectionFirst,
        SelectionAngle,
        SelectionNormalRing,
        SelectionAngleRing,

        SelectionLast,
    };

    InteractiveNormalPrivate(InteractiveNormal *q_ptr) {
        q = q_ptr;
        q->SetInfiniteState();
        q->SetMutable(Standard_True);
        q->SetAutoHilight(Standard_False);
        auto textAspect = q->Attributes()->TextAspect();
        if (!textAspect) {
            textAspect = new Prs3d_TextAspect;
        }
        textAspect->SetHeight(mLabelHeight);
        q->Attributes()->SetTextAspect(textAspect);
    }

    void compute(const Handle(Prs3d_Presentation) &presentation,
                 const Quantity_Color &selectionColor) {
        mPrimitives[PrimitiveNormalRing] = createRing(presentation, normalDir(), Quantity_NOC_GRAY40, 0.);
        mPrimitives[PrimitiveAngleRing] = createRing(presentation, angleDir(), Quantity_NOC_GRAY60, 0.);
        mPrimitives[PrimitiveNormal] = createLine(presentation, normalDir(), mLen, Quantity_NOC_YELLOW);
        mPrimitives[PrimitiveAngle] = createLine(presentation, angleDir(), angleLen(), Quantity_NOC_BLUE);
        mPrimitives[PrimitiveNormalArrow] = createArrow(presentation, normalDir(), mLen, selectionColor, 0.);
        mPrimitives[PrimitiveAngleArrow] = createArrow(presentation, angleDir(), angleLen(), selectionColor, 0.);
        if (!mLabel.IsEmpty()) {
            mPrimitives[PrimitiveLabel] = createLabel(presentation, Quantity_NOC_YELLOW);
        }
    }

    void computeSelection(const Handle(SelectMgr_Selection) &selection) {
        for (int i = SelectionFirst; i < SelectionLast; ++i) {
            const Selections type = static_cast <Selections> (i);
            auto &owner = mSelections[type];
            Standard_Boolean bSelected = Standard_False;
            if (owner) {
                bSelected = owner->IsSelected();
            }

            owner = new SelectMgr_EntityOwner(q);
            owner->SetComesFromDecomposition(Standard_True);
            switch (type) {
                case SelectionNormal: {
                    auto sens = new Select3D_SensitiveSegment(owner,
                                                              pos(),
                                                              endPos(normalDir(), mLen));
                    sens->SetSensitivityFactor(12);
                    selection->Add(sens);
                    break;
                }

                case SelectionAngle: {
                    auto sens = new Select3D_SensitiveSegment(owner,
                                                              pos(),
                                                              endPos(angleDir(), angleLen()));
                    sens->SetSensitivityFactor(15);
                    selection->Add(sens);
                    break;
                }

                case SelectionNormalRing:
                case SelectionAngleRing: {
                    if (!isSelected()) {
                        break;
                    }

                    gp_Dir dir = normalDir();
                    if (type == SelectionAngleRing) {
                        dir = angleDir();
                    }
                    const gp_Circ circle(gp_Ax2(gp::Origin(), dir), ringRadius());
                    auto sensCircle = new SensCircle(owner, circle, mFacetesCount * 2);
                    sensCircle->SetSensitivityFactor(10);
                    selection->Add(sensCircle);
                    // enlarge sensitivity by triangulation
                    auto radius = ringRadius();
                    Prs3d_ToolDisk tool(radius, radius + mRingWidth, mFacetesCount * 2, mFacetesCount * 2);
                    gp_Trsf trsf;
                    trsf.SetTransformation(gp_Ax3(pos(), dir));
                    auto triangulation = tool.CreatePolyTriangulation(trsf);
                    auto sensTr = new SensTriangulation(owner, triangulation, dir);
                    selection->Add(sensTr);
                    break;
                }

                case SelectionLast:
                    break;
            }
            owner->SetSelected(bSelected);
        }
    }

    void drawHiglighted(const Handle(SelectMgr_EntityOwner) &owner,
                        const Handle(Prs3d_Presentation) &presentation,
                        const Quantity_Color &color) {
        auto owId = SelectionNormal;
        if (isSelected() && ownerId(owId, owner)) {
            switch (owId) {
                case SelectionNormal:
                    createArrow(presentation, normalDir(), mLen, color, 1.);
                    break;
                case SelectionAngle:
                    createArrow(presentation, angleDir(), angleLen(), color, 1.);
                    break;
                case SelectionNormalRing:
                    createRing(presentation, normalDir(), color, 1.);
                    break;
                case SelectionAngleRing:
                    createRing(presentation, angleDir(), color, 1.);
                    break;

                case SelectionLast:
                    break;
            }
        } else {
            createLine(presentation, normalDir(), mLen, color);
            createLine(presentation, angleDir(), angleLen(), color);
            if (!mLabel.IsEmpty()) {
                createLabel(presentation, color);
            }
        }
    }

    void hilightSelected() {
        for (const auto &p : mPrimitives) {
            Standard_Real alpha = 1.;
            switch (p.first) {
                case PrimitiveNormal:
                case PrimitiveAngle:
                    alpha = 0.;
                    break;
                case PrimitiveNormalArrow:
                case PrimitiveAngleArrow:
                case PrimitiveNormalRing:
                case PrimitiveAngleRing:
                    alpha = 1.;
                    break;
                case PrimitiveLabel:
                    p.second.aspect->SetColor(Quantity_NOC_GRAY80);
                    break;
            }
            auto rgba = p.second.aspect->InteriorColorRGBA();
            rgba.SetAlpha(alpha);
            p.second.aspect->SetInteriorColor(rgba);
        }
    }

    void clearSelected() {
        for (auto &s : mSelections) {
            s.second->SetSelected(false);
        }

        for (const auto &p : mPrimitives) {
            Standard_Real alpha = 1.;
            switch (p.first) {
                case PrimitiveNormal:
                case PrimitiveAngle:
                    alpha = 1.;
                    break;
                case PrimitiveNormalArrow:
                case PrimitiveAngleArrow:
                case PrimitiveNormalRing:
                case PrimitiveAngleRing:
                    alpha = 0.;
                    break;
                case PrimitiveLabel:
                    p.second.aspect->SetColor(Quantity_NOC_YELLOW);
                    break;
            }
            auto rgba = p.second.aspect->InteriorColorRGBA();
            rgba.SetAlpha(alpha);
            p.second.aspect->SetInteriorColor(rgba);
        }
    }

    Standard_Boolean ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                     const Handle(V3d_View) &view,
                                     const Handle(SelectMgr_EntityOwner) &owner,
                                     const Graphic3d_Vec2i &from,
                                     const Graphic3d_Vec2i &to,
                                     const AIS_DragAction action) {
        auto owId = SelectionNormal;
        if (!isSelected() || !ownerId(owId, owner)) {
            return Standard_False;
        }

        switch (action) {
            case AIS_DragAction_Start:
                mOldCombinedTrsf = q->mapToGlobal(q->LocalTransformation());
                break;

            case AIS_DragAction_Update:
                processDragActionUpdate(view, from, to, owId);
                break;

            case AIS_DragAction_Stop:
                context->RecomputeSelectionOnly(q);
                break;

            case AIS_DragAction_Abort:
                q->SetLocalTransformation(q->mapToLocal(mOldCombinedTrsf));
                break;
        }
        return Standard_True;
    }

    void processDragActionUpdate(const Handle(V3d_View) &view,
                                 const Graphic3d_Vec2i &from,
                                 const Graphic3d_Vec2i &to,
                                 const Selections ownerId) {
        // Get 3d points with projection vectors
        Graphic3d_Vec3d fromPnt, fromProj;
        view->ConvertWithProj(from.x(), from.y(),
                              fromPnt.x(), fromPnt.y(), fromPnt.z(),
                              fromProj.x(), fromProj.y(), fromProj.z());
        const gp_Lin fromLine(gp_Pnt(fromPnt.x(), fromPnt.y(), fromPnt.z()),
                              gp_Dir(fromProj.x(), fromProj.y(), fromProj.z()));
        Graphic3d_Vec3d toPnt, toProj;
        view->ConvertWithProj(to.x(), to.y(),
                              toPnt.x(), toPnt.y(), toPnt.z(),
                              toProj.x(), toProj.y(), toProj.z());
        const gp_Lin toLine(gp_Pnt(toPnt.x(), toPnt.y(), toPnt.z()),
                            gp_Dir(toProj.x(), toProj.y(), toProj.z()));
        switch (ownerId) {
            case SelectionNormal:
            case SelectionAngle: {
                gp_Dir dir = normalDir();
                if (ownerId == SelectionAngle) {
                    dir = angleDir();
                }
                processDragTranslation(dir, fromLine, toLine);
                break;
            }

            case SelectionNormalRing:
            case SelectionAngleRing: {
                gp_Dir dir = normalDir();
                if (ownerId == SelectionAngleRing) {
                    dir = angleDir();
                }
                processDragRotation(dir, fromLine, toLine, ownerId);
                break;
            }

            case SelectionLast:
                break;
        }
    }

    void processDragTranslation(const gp_Dir &dir,
                                const gp_Lin &fromLine,
                                const gp_Lin &toLine) {
        const gp_Lin arrowLine(pos().Transformed(mOldCombinedTrsf),
                               dir.Transformed(mOldCombinedTrsf));
        Extrema_ExtElC extremaFrom(fromLine, arrowLine, Precision::Angular());
        Extrema_ExtElC extremaTo(toLine, arrowLine, Precision::Angular());
        if (!extremaFrom.IsDone() || extremaFrom.IsParallel() || extremaFrom.NbExt() != 1
                || !extremaTo.IsDone() || extremaTo.IsParallel() || extremaTo.NbExt() != 1) {
          // translation cannot be done co-directed with camera
          return;
        }

        Extrema_POnCurv exPnt[4];
        extremaFrom.Points(1, exPnt[0], exPnt[1]);
        extremaTo.Points(1, exPnt[2], exPnt[3]);
        const gp_Pnt fromPos = exPnt[1].Value();
        const gp_Pnt toPos = exPnt[3].Value();
        if (toPos.Distance(fromPos) < Precision::Confusion()) {
            return;
        }

        gp_Trsf trsf;
        trsf.SetTranslationPart(gp_Vec(fromPos, toPos));
        q->SetLocalTransformation(q->mapToLocal(trsf * mOldCombinedTrsf));
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

    void processDragRotation(const gp_Dir &dir,
                             const gp_Lin &fromLine,
                             const gp_Lin &toLine,
                             const Selections ownerId) {
        const gp_Pnt posLoc = pos().Transformed(mOldCombinedTrsf);
        const gp_Ax2 startPos(posLoc, dir.Transformed(mOldCombinedTrsf));
        const gp_Ax1 currAxis = getAx1FromAx2Dir(startPos, ownerId == SelectionNormal ? 0 : 2);
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
            return;
        }

        const gp_Pnt fromPos = crossFrom.Point(1);
        const gp_Pnt toPos = crossTo.Point(1);
        if (toPos.Distance(fromPos) < Precision::Confusion()) {
            return;
        }

        const gp_Dir startAxis = gce_MakeDir(posLoc, fromPos);
        const gp_Dir toAxis = posLoc.IsEqual(fromPos, Precision::Confusion())
          ? getAx1FromAx2Dir(startPos, ((ownerId == SelectionNormal ? 0 : 2) + 1) % 3).Direction()
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
            return;
        }

        gp_Trsf trsf;
        trsf.SetRotation(currAxis, angle);
        q->SetLocalTransformation(q->mapToLocal(trsf * mOldCombinedTrsf));
    }

    gp_Pnt pos() const {
        return gp::Origin();
    }

    gp_Pnt endPos(const gp_Dir &dir, Standard_Real len) const {
        return pos().Translated(gp_Vec(dir.XYZ() * len));
    }

    gp_Dir normalDir() const {
        return gp::DZ();
    }

    gp_Dir angleDir() const {
        return gp::DX();
    }

    Standard_Real angleLen() const {
        return mLen * .5;
    }

    Standard_Real ringRadius() const {
        return angleLen() + .5;
    }

    gp_Pnt labelPos() const {
        return pos();
    }

    Primitive createLine(const Handle(Prs3d_Presentation) &presentation,
                         const gp_Dir &dir,
                         const Standard_Real len,
                         const Quantity_Color &color) {
        Primitive line;
        line.group = presentation->NewGroup();
        line.aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        line.aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        line.group->SetGroupPrimitivesAspect(line.aspect);
        auto array = new Graphic3d_ArrayOfPolylines(2);
        array->AddVertex(pos());
        array->AddVertex(endPos(dir, len));
        line.group->AddPrimitiveArray(array);
        return line;
    }

    Primitive createArrow(const Handle(Prs3d_Presentation) &presentation,
                          const gp_Dir &dir,
                          const Standard_Real len,
                          const Quantity_Color &color,
                          const Standard_Real alpha) {
        Primitive arrow;
        arrow.group = presentation->NewGroup();
        arrow.aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        arrow.aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        arrow.aspect->SetInteriorColor(Quantity_ColorRGBA(arrow.aspect->Color(), alpha));
        arrow.group->SetClosed(Standard_True);
        arrow.group->SetGroupPrimitivesAspect(arrow.aspect);
        auto array = Prs3d_Arrow::DrawShaded(gp_Ax1(pos(), dir),
                                             mWidth * mTubeRadiusPercent,
                                             len,
                                             mLen * mConeRadisPercent,
                                             mLen * mConeLenPercent,
                                             mFacetesCount);
        arrow.group->AddPrimitiveArray(array);
        return arrow;
    }

    Primitive createRing(const Handle(Prs3d_Presentation) &presentation,
                         const gp_Dir &rotateDir,
                         const Quantity_Color &color,
                         const Standard_Real alpha) {
        Primitive ring;
        ring.group = presentation->NewGroup();
        Aspect_InteriorStyle style;
        Graphic3d_MaterialAspect mAspect;
        ring.aspect = new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, mWidth);
        ring.aspect->SetAlphaMode(Graphic3d_AlphaMode_Mask);
        ring.aspect->SetInteriorColor(Quantity_ColorRGBA(ring.aspect->Color(), alpha));
        ring.group->SetClosed(Standard_True);
        ring.group->SetGroupPrimitivesAspect(ring.aspect);
        const Standard_Real radius = ringRadius();
        Prs3d_ToolDisk tool(radius, radius + mRingWidth, mFacetesCount * 2, mFacetesCount * 2);
        gp_Trsf trsf;
        trsf.SetTransformation(gp_Ax3(pos(), rotateDir));
        auto triangles = tool.CreateTriangulation(trsf);
        ring.group->AddPrimitiveArray(triangles);
        return ring;
    }

    Primitive createLabel(const Handle(Prs3d_Presentation) &presentation,
                          const Quantity_Color &color) {
        Primitive label;
        label.group = presentation->NewGroup();
        auto textAspect = new Graphic3d_AspectText3d(*q->Attributes()->TextAspect()->Aspect());
        textAspect->SetColor(color);
        label.aspect = textAspect;
        label.group->SetClosed(Standard_True);
        label.group->SetGroupPrimitivesAspect(label.aspect);
        Prs3d_Text::Draw(label.group, new Prs3d_TextAspect(textAspect), mLabel, labelPos());
        return label;
    }

    bool isSelected() const {
        for (const auto &s : mSelections) {
            if (s.second->IsSelected()) {
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

    InteractiveNormal *q = nullptr;
    Standard_Real mLen = 5.;
    Standard_Real mWidth = 2.;
    Standard_Real mRingWidth = 1.;
    TCollection_ExtendedString mLabel;
    Standard_Real mLabelHeight = 20.;

    // arrows
    Standard_Integer mFacetesCount = 12;
    Standard_Real mTubeRadiusPercent = .05;
    Standard_Real mConeRadisPercent = .04;
    Standard_Real mConeLenPercent = .1;

    std::map <Primitives, Primitive> mPrimitives;
    std::map <Selections, Handle(SelectMgr_EntityOwner)> mSelections;

    gp_Trsf mOldCombinedTrsf;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveNormal, AIS_InteractiveObject)

InteractiveNormal::InteractiveNormal()
    : AIS_InteractiveObject()
    , d(new InteractiveNormalPrivate(this))
{

}

InteractiveNormal::InteractiveNormal(const gp_Pnt &pos, const gp_Dir &normal)
    : AIS_InteractiveObject()
    , d(new InteractiveNormalPrivate(this))
{
    gp_Trsf trsf;
    trsf.SetTranslationPart(gp_Vec(pos.XYZ()));
    gp_Quaternion quat;
    quat.SetRotation(gp::DZ().XYZ(), normal.XYZ());
    trsf.SetRotationPart(quat);
    SetLocalTransformation(trsf);
}

InteractiveNormal::~InteractiveNormal()
{
    delete d;
}

void InteractiveNormal::setNormal(const gp_Trsf &normal)
{
    SetLocalTransformation(normal);
}

gp_Trsf InteractiveNormal::getNormal() const
{
    return LocalTransformation();
}

void InteractiveNormal::setLabel(const TCollection_AsciiString &txt)
{
    d->mLabel = txt;
}

gp_Trsf InteractiveNormal::mapToGlobal(const gp_Trsf &localTrsf) const
{
    gp_Trsf combinedParent;
    if (CombinedParentTransformation()) {
        combinedParent = CombinedParentTransformation()->Trsf();
    }
    return combinedParent * localTrsf;
}

gp_Trsf InteractiveNormal::mapToLocal(const gp_Trsf &globalTrsf) const
{
    gp_Trsf combinedParent;
    if (CombinedParentTransformation()) {
        combinedParent = CombinedParentTransformation()->Trsf();
    }
    return combinedParent.Inverted() * globalTrsf;
}

void InteractiveNormal::HilightSelected(const Handle(PrsMgr_PresentationManager) &,
                                        const SelectMgr_SequenceOfOwner &)
{
    d->hilightSelected();
    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputeSelectionOnly(this);
    }
}

void InteractiveNormal::ClearSelected()
{
    d->clearSelected();
    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputeSelectionOnly(this);
    }
}

void InteractiveNormal::HilightOwnerWithColor(const Handle(PrsMgr_PresentationManager) &thePM,
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

void InteractiveNormal::ComputeSelection(const Handle(SelectMgr_Selection) &theSelection,
                                         const Standard_Integer theMode)
{
    if (theMode == 2) { // why ???
        d->computeSelection(theSelection);
    }
}

Standard_Boolean InteractiveNormal::ProcessDragging(const Handle(AIS_InteractiveContext) &context,
                                                    const Handle(V3d_View) &view,
                                                    const Handle(SelectMgr_EntityOwner) &owner,
                                                    const Graphic3d_Vec2i &from,
                                                    const Graphic3d_Vec2i &to,
                                                    const AIS_DragAction action)
{
    return d->ProcessDragging(context, view, owner, from, to, action);
}

void InteractiveNormal::Compute(const Handle(PrsMgr_PresentationManager) &,
                                const Handle(Prs3d_Presentation) &thePrs,
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
    d->compute(thePrs, selectionColor);
}

}
