#ifndef INTERACTIVEOBJECTITEMMODELCREATOR_H
#define INTERACTIVEOBJECTITEMMODELCREATOR_H

#include <Standard_Handle.hxx>

class InteractiveObjectItemModel;
class InteractiveObject;

class InteractiveObjectItemModelCreator
{
public:
    InteractiveObjectItemModelCreator();

    InteractiveObjectItemModel *createModel(const Handle(InteractiveObject) &interactive);
};

#endif // INTERACTIVEOBJECTITEMMODELCREATOR_H
