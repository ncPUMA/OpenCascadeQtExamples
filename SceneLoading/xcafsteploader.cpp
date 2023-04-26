#include "xcafsteploader.h"

#include <QDebug>
#include <QString>

#include <AIS_Shape.hxx>
#include <NCollection_DataMap.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TopExp_Explorer.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

class XCafStepLoaderPrivate
{
    friend class XCafStepLoader;

    struct InternalData
    {
        Handle(AIS_InteractiveObject) parent;
        TCollection_ExtendedString name;
        TopoDS_Shape shape;
        TopLoc_Location location;
        Standard_Boolean hasColors[XCAFDoc_ColorCurv + 1];
        Quantity_ColorRGBA colors[XCAFDoc_ColorCurv + 1];
    };

    void addShape(const InternalData &data, Handle(AIS_Shape) &object) {
        if (data.shape.ShapeType() == TopAbs_COMPOUND) {
            for (TopExp_Explorer anExp(data.shape, TopAbs_SOLID); anExp.More(); anExp.Next()) {
                InternalData chData = data;
                chData.name.Clear();
                chData.shape = anExp.Current();
                chData.parent = object;
                Handle(AIS_Shape) chObj = new AIS_Shape(TopoDS_Shape());
                auto drawer = chObj->Attributes();
                auto shAspect = drawer->ShadingAspect();
                auto fbAspect = drawer->FaceBoundaryAspect();
                if (chData.hasColors[XCAFDoc_ColorGen]) {
                    shAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
                    fbAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
                }
                if (chData.hasColors[XCAFDoc_ColorSurf]) {
                    shAspect->SetColor(data.colors[XCAFDoc_ColorSurf].GetRGB());
                }
                if (chData.hasColors[XCAFDoc_ColorCurv]) {
                    fbAspect->SetColor(data.colors[XCAFDoc_ColorCurv].GetRGB());
                }

                object->AddChild(chObj);
                addShape(chData, chObj);
                objects.Bind(chObj, chData);
            }
        } else {
            object->SetShape(data.shape);
        }
    }

    void addShape(const TDF_Label &label, Handle(AIS_InteractiveObject) parent = nullptr) {
        TDF_Label origLabel;
        if (XCAFDoc_ShapeTool::IsReference(label)) {
            XCAFDoc_ShapeTool::GetReferredShape(label, origLabel);
        } else {
            origLabel = label;
        }

        InternalData data;
        Handle(TDataStd_Name) name;
        if (label.FindAttribute(TDataStd_Name::GetID(), name)) {
            data.name = name->Get();
        }
        XCAFDoc_ShapeTool::GetShape(label, data.shape);
        Handle(XCAFDoc_Location) location;
        if (label.FindAttribute(XCAFDoc_Location::GetID(), location)) {
            data.location = location->Get();
        }

        if (materialTool) {
            Handle(TCollection_HAsciiString) name, description, densName, densValType;
            Standard_Real density;
            if (materialTool->GetMaterial(origLabel, name, description, density, densName, densValType)) {
                qDebug() << "Found material for" << QString::fromStdU16String(data.name.ToExtString())
                         << name->ToCString()
                         << description->ToCString()
                         << density
                         << densName->ToCString()
                         << densValType->ToCString();
            }
        }

        if (colorTool) {
            for (int i = XCAFDoc_ColorGen; i <= XCAFDoc_ColorCurv; ++i) {
                Quantity_ColorRGBA clr;
                data.hasColors[i] = colorTool->GetColor(origLabel, static_cast<XCAFDoc_ColorType>(i), clr);
                if (data.hasColors[i]) {
                    data.colors[i] = clr;
                }
            }
        }

        if (layerTool) {
            TDF_LabelSequence labels;
            if (layerTool->GetLayers(origLabel, labels)) {
                for (const auto &lbl : labels) {
                    TCollection_ExtendedString name;
                    layerTool->GetLayer(lbl, name);
                    qDebug() << "Layer" << QString::fromStdU16String(name.ToExtString());
                }
            }
        }

        Handle(AIS_Shape) obj = new AIS_Shape(TopoDS_Shape());
        auto drawer = obj->Attributes();
        auto shAspect = drawer->ShadingAspect();
        auto fbAspect = drawer->FaceBoundaryAspect();
        if (data.hasColors[XCAFDoc_ColorGen]) {
            shAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
            fbAspect->SetColor(data.colors[XCAFDoc_ColorGen].GetRGB());
        }
        if (data.hasColors[XCAFDoc_ColorSurf]) {
            shAspect->SetColor(data.colors[XCAFDoc_ColorSurf].GetRGB());
        }
        if (data.hasColors[XCAFDoc_ColorCurv]) {
            fbAspect->SetColor(data.colors[XCAFDoc_ColorCurv].GetRGB());
        }

        if (parent) {
            data.parent = parent;
            parent->AddChild(obj);
        }

        if (XCAFDoc_ShapeTool::IsAssembly(label)) {
            for (TDF_ChildIterator it(label); it.More(); it.Next()) {
                addShape(it.Value(), obj);
            }
        } else {
            addShape(data, obj);
        }
        objects.Bind(obj, data);
    }

    Handle(TDocStd_Document) document;
    Handle(XCAFDoc_ShapeTool) shapeTool;
    Handle(XCAFDoc_MaterialTool) materialTool;
    Handle(XCAFDoc_ColorTool) colorTool;
    Handle(XCAFDoc_LayerTool) layerTool;
    typedef NCollection_DataMap<Handle(AIS_InteractiveObject), InternalData> InternalMap;
    InternalMap objects;
};

XCafStepLoader::XCafStepLoader()
    : d(new XCafStepLoaderPrivate)
{

}

XCafStepLoader::~XCafStepLoader()
{
    delete d;
}

bool XCafStepLoader::loadStep(const char *fName)
{
    d->objects.Clear();
    d->document = new TDocStd_Document("BinXCAF");
    STEPCAFControl_Reader reader;
    reader.SetColorMode(Standard_True);
    reader.SetNameMode(Standard_True);
    reader.SetMatMode(Standard_True);
    if (reader.ReadFile(fName) != IFSelect_RetDone || !reader.Transfer(d->document)) {
        return false;
    }

    const auto mainLabel = d->document->Main();
    d->shapeTool = XCAFDoc_DocumentTool::ShapeTool(mainLabel);
    d->colorTool = XCAFDoc_DocumentTool::ColorTool(mainLabel);
    d->layerTool = XCAFDoc_DocumentTool::LayerTool(mainLabel);

    TDF_LabelSequence shapes;
    d->shapeTool->GetFreeShapes(shapes);
    for (const auto &label : shapes) {
        d->addShape(label);
    }
    return !d->objects.IsEmpty();
}

AIS_ListOfInteractive XCafStepLoader::result() const
{
    AIS_ListOfInteractive ret;
    for (XCafStepLoaderPrivate::InternalMap::Iterator it(d->objects); it.More(); it.Next()) {
        if (it.Value().parent.IsNull()) {
            ret.Append(it.Key());
        }
    }
    return ret;
}

TCollection_ExtendedString XCafStepLoader::name(const Handle(AIS_InteractiveObject) &object) const
{
    XCafStepLoaderPrivate::InternalData data;
    if (d->objects.Find(object, data)) {
        return data.name;
    }
    return TCollection_ExtendedString();
}
