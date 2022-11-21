#ifndef EB_IGESLOADER_H
#define EB_IGESLOADER_H

#include "abstractmodelloader.h"

namespace ExamplesBase {

class IgesLoader : public AbstractModelLoader
{
public:
    IgesLoader();

    TopoDS_Shape load(const char *fName) final;
};

}

#endif // EB_IGESLOADER_H
