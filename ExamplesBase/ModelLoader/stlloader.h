#ifndef EB_STLLOADER_H
#define EB_STLLOADER_H

#include "abstractmodelloader.h"

namespace ExamplesBase {

class StlLoader : public AbstractModelLoader
{
public:
    StlLoader();

    TopoDS_Shape load(const char *fName) final;
};

}

#endif // EB_STLLOADER_H
