#ifndef BREPLOADER_H
#define BREPLOADER_H

#include "abstractmodelloader.h"

class BrepLoader : public AbstractModelLoader
{
public:
    BrepLoader();

    TopoDS_Shape load(const char *fName) final;
};

#endif // BREPLOADER_H
