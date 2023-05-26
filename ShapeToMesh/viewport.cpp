#include "viewport.h"

#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRep_Tool.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_SelectionModeFlags.hxx>
#include <StdPrs_ToolTriangulatedShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <V3d_View.hxx>
#include <XSDRAWSTLVRML_DataSource.hxx>

#include <ExamplesBase/ModelLoader/modelloaderfactorymethod.h>
#include <ExamplesBase/ModelLoader/abstractmodelloader.h>

inline static Standard_Integer triangulate(TopoDS_Shape &shape,
                                           Standard_Real linearDeflection = .01,
                                           Standard_Real angularDeflection = .5) {
    if (!StdPrs_ToolTriangulatedShape::IsTriangulated(shape)) {
        BRepMesh_IncrementalMesh aMesher(shape, linearDeflection, Standard_False, angularDeflection, Standard_True);
        return aMesher.GetStatusFlags();
    }
    return 0;
}

inline static Handle(Poly_Triangulation) shapeToTriangulation(const TopoDS_Shape &shape) {
    std::vector<gp_Pnt> nodes;
    std::vector<Poly_Triangle> triangles;
    Standard_Integer offset = 0;
    for (TopExp_Explorer anExp(shape, TopAbs_FACE); anExp.More(); anExp.Next()) {
        auto &curFace = TopoDS::Face(anExp.Current());
        TopLoc_Location location;
        auto triangulation = BRep_Tool::Triangulation(curFace, location);
        if (!triangulation) {
            continue;
        }
        for (const auto &pnt : triangulation->Nodes()) {
            nodes.push_back(pnt.Transformed(location.Transformation()));
        }
        for (const auto &triangle : triangulation->Triangles()) {
            Standard_Integer N1, N2, N3;
            triangle.Get(N1, N2, N3);
            triangles.emplace_back(N1 + offset - 1, N2 + offset - 1, N3 + offset - 1);
        }
        offset += triangulation->Nodes().Size();
    }

    if (nodes.empty() || triangles.empty()) {
        return nullptr;
    }

    Standard_Integer nodesSize = static_cast<Standard_Integer>(nodes.size());
    TColgp_Array1OfPnt nodesArray(1, nodesSize);
    for (Standard_Integer i = 0; i < nodesSize; ++i) {
        nodesArray.SetValue(i + 1, nodes[i]);
    }
    Standard_Integer trianglesSize = static_cast<Standard_Integer>(triangles.size());
    Poly_Array1OfTriangle trianglesArray(1, trianglesSize);
    for (Standard_Integer i = 0; i < trianglesSize; ++i) {
        auto triangle = triangles[i];
        Standard_Integer N1, N2, N3;
        triangle.Get(N1, N2, N3);
        if (N1 < 0 || N2 < 0 || N3 < 0 || N1 >= nodesSize || N2 >= nodesSize || N3 >= nodesSize) {
            N1 = 0;
            N2 = 0;
            N3 = 0;
        }
        trianglesArray.SetValue(i + 1, Poly_Triangle(N1 + 1, N2 + 1, N3 + 1));
    }
    return new Poly_Triangulation(nodesArray, trianglesArray);
}

class ViewportPrivate
{
    friend class Viewport;

    void addMesh(TopoDS_Shape shape, const gp_XYZ &translation, const Handle(AIS_InteractiveContext) &ctx) {
        auto drawer = ctx->DefaultDrawer();
        auto deflection = StdPrs_ToolTriangulatedShape::GetDeflection(shape, drawer);
        if (triangulate(shape, deflection, drawer->DeviationAngle()) == 0) {
            auto triangulation = shapeToTriangulation(shape);
            if (triangulation) {
                Handle(MeshVS_Mesh) mesh = new MeshVS_Mesh;
                mesh->SetDisplayMode(MeshVS_DMF_WireFrame);
                mesh->SetDataSource(new XSDRAWSTLVRML_DataSource(triangulation));
                Handle(MeshVS_MeshPrsBuilder) builder = new MeshVS_MeshPrsBuilder(mesh);
                mesh->AddBuilder(builder, true);
                ctx->Display(mesh, Standard_True);
                gp_Trsf trsf;
                trsf.SetTranslation(translation);
                ctx->SetLocation(mesh, trsf);
            }
        }
    }
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d(new ViewportPrivate)
{

}

Viewport::~Viewport()
{
    delete d;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());

        Handle(AIS_InteractiveObject) object;
        Graphic3d_Vec3d pickedPoint, projection;
        view()->ConvertWithProj(aPnt.x(), aPnt.y(),
                                pickedPoint.x(), pickedPoint.y(), pickedPoint.z(),
                                projection.x(), projection.y(), projection.z());

        auto ctx = context();
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        if (ctx->MainSelector()->NbPicked()) {
            auto owner = ctx->MainSelector()->Picked(1);
            if (owner) {
                object = Handle(AIS_InteractiveObject)::DownCast(owner->Selectable());
                auto point = ctx->MainSelector()->PickedPoint(1);
                if (object) {
                    point.Transform(ctx->Location(object).Transformation().Inverted());
                    pickedPoint.SetValues(point.X(), point.Y(), point.Z());
                }
            }
        }

        gp_XYZ translation(pickedPoint.x(), pickedPoint.y(), pickedPoint.z());
        QMenu menu;
        menuRequest(object, translation, menu);
        return menu.exec(event->globalPos()) != nullptr;
    }
    return false;
}

void Viewport::menuRequest(const Handle(AIS_InteractiveObject) &object,
                           const gp_XYZ &pickedPoint,
                           QMenu &menu)
{
    if (!object) {
        menu.addAction(tr("Cylinder"), this, [this, object, pickedPoint]() {
            d->addMesh(BRepPrimAPI_MakeCylinder(50., 200.), pickedPoint, context());
        });
        menu.addAction(tr("Sphere"), this, [this, object, pickedPoint]() {
            d->addMesh(BRepPrimAPI_MakeSphere(100.), pickedPoint, context());
        });
        menu.addAction(tr("Load..."), this, [this, object, pickedPoint]() {
            ExamplesBase::ModelLoaderFactoryMethod factory;
            auto path =  QFileDialog::getOpenFileName(this,
                                                     tr("Choose model file"),
                                                     QString(),
                                                     factory.supportedFilters());
            if (!path.isEmpty()) {
                auto &loader = factory.loaderByFName(path);
                auto shape = loader.load(path.toLatin1());
                if (!shape.IsNull()) {
                    d->addMesh(shape, pickedPoint, context());
                }
            }
        });
    }
}
