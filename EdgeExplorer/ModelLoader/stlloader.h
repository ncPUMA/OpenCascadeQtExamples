#ifndef STLLOADER_H
#define STLLOADER_H

#include "abstractmodelloader.h"

class StlLoader : public AbstractModelLoader
{
public:
    StlLoader();

    TopoDS_Shape load(const char *fName) final;
};

#endif // STLLOADER_H
