#ifndef STEPLOADER_H
#define STEPLOADER_H

#include "abstractmodelloader.h"

class StepLoader : public AbstractModelLoader
{
public:
    StepLoader();

    TopoDS_Shape load(const char *fName) final;
};

#endif // STEPLOADER_H
