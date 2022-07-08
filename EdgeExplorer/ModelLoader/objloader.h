#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "abstractmodelloader.h"

class ObjLoader : public AbstractModelLoader
{
public:
    ObjLoader();

    TopoDS_Shape load(const char *fName) final;
};

#endif // OBJLOADER_H
