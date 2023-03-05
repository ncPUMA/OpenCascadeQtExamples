#ifndef OBJECTSTREEMODEL_H
#define OBJECTSTREEMODEL_H

#include <QStandardItemModel>

#include <Standard_Handle.hxx>

class AIS_InteractiveContext;
class InteractiveObject;

class ObjectsTreeModel : public QStandardItemModel
{
    Q_OBJECT

public:
    ObjectsTreeModel(QObject *parent = nullptr);

    void update(const Handle(AIS_InteractiveContext) &context);
    void addObject(const Handle(InteractiveObject) &object);
    void removeObject(const Handle(InteractiveObject) &object);
    Handle(InteractiveObject) object(const QModelIndex &index) const;
    QModelIndex index(const Handle(InteractiveObject) &object) const;
};

#endif // OBJECTSTREEMODEL_H
