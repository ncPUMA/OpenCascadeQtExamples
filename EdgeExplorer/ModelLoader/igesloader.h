#ifndef IGESLOADER_H
#define IGESLOADER_H

#include "abstractmodelloader.h"

class IgesLoader : public AbstractModelLoader
{
public:
    IgesLoader();

    TopoDS_Shape load(const char *fName) final;
};

#endif // IGESLOADER_H
