#ifndef ABSTRACTMODELLOADER_H
#define ABSTRACTMODELLOADER_H

#include <TopoDS_Shape.hxx>

class AbstractModelLoader
{
public:
    AbstractModelLoader() = default;
    virtual ~AbstractModelLoader() = default;

    virtual TopoDS_Shape load(const char *fName) = 0;

private:
    AbstractModelLoader(const AbstractModelLoader &) = delete;
    AbstractModelLoader& operator=(const AbstractModelLoader &) = delete;
};

#endif // ABSTRACTMODELLOADER_H
