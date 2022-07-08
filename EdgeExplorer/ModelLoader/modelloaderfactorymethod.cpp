#include "modelloaderfactorymethod.h"

#include <vector>
#include <map>

#include <QStringList>

#include "breploader.h"
#include "steploader.h"
#include "igesloader.h"
#include "stlloader.h"
#include "objloader.h"

class EmptyModelLoader : public AbstractModelLoader
{
public:
    EmptyModelLoader() : AbstractModelLoader() { }

    TopoDS_Shape load(const char *) final {
        return TopoDS_Shape();
    }
};



class ModelLoaderFactoryMethodPrivate
{
    friend class ModelLoaderFactoryMethod;

    ModelLoaderFactoryMethodPrivate() {
        appendLoader("STEP (*.step *.stp)", new StepLoader());
        appendLoader("BREP (*.brep)", new BrepLoader());
        appendLoader("IGES (*.iges)", new IgesLoader());
        appendLoader("STL (*.stl)"  , new StlLoader());
        appendLoader("OBJ (*.obj)"  , new ObjLoader());
    }

    ~ModelLoaderFactoryMethodPrivate() {
        for(const auto &pair : loaders)
            delete pair.second;
    }

    AbstractModelLoader& loader(const QString &filter) {
        AbstractModelLoader * result = &emptyLoader;
        for(const auto &pair : loaders) {
            if (pair.first == filter) {
                result = pair.second;
                break;
            }
        }
        return *result;
    }

    AbstractModelLoader& loaderByFName(const QString &fName) {
        AbstractModelLoader * result = &emptyLoader;
        const QStringList ls = fName.split('.');
        if (ls.size() > 1) {
            const QString &ext = ls.last();
            for(const auto &pair : loaders) {
                if (pair.first.contains(ext, Qt::CaseInsensitive)) {
                    result = pair.second;
                    break;
                }
            }
        }
        return *result;
    }

    void appendLoader(const char *format, AbstractModelLoader * const loader) {
        loaders.push_back(std::make_pair(QString(format), loader));
    }

    std::vector <std::pair <QString, AbstractModelLoader *> > loaders;
    EmptyModelLoader emptyLoader;
};



ModelLoaderFactoryMethod::ModelLoaderFactoryMethod() :
    d_ptr(new ModelLoaderFactoryMethodPrivate())
{

}

ModelLoaderFactoryMethod::~ModelLoaderFactoryMethod()
{
    delete d_ptr;
}

QString ModelLoaderFactoryMethod::supportedFilters() const
{
    QStringList ls;
    for(const auto &pair : d_ptr->loaders)
        ls << pair.first;
    return ls.join(";;");
}

AbstractModelLoader &ModelLoaderFactoryMethod::loader(const QString &filter)
{
    return d_ptr->loader(filter);
}

AbstractModelLoader &ModelLoaderFactoryMethod::loaderByFName(const QString &fName)
{
    return d_ptr->loaderByFName(fName);
}
