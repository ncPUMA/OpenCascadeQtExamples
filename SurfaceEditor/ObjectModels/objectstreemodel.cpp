#include "objectstreemodel.h"

#include <set>
#include <stack>

#include <AIS_InteractiveContext.hxx>

#include "../Objects/interactiveobject.h"

class Item : public QStandardItem
{
public:
    Item(const Handle(InteractiveObject) &obj)
        : QStandardItem(obj->name())
        , object(obj)
    { }

    void setData(const QVariant &value, int role = Qt::UserRole + 1) final {
        if (role == Qt::EditRole) {
            object->setName(value.toString());
        }
        QStandardItem::setData(value, role);
    }

    Handle(InteractiveObject) object;
};

inline static Item *findItem(const ObjectsTreeModel &model, const Handle(InteractiveObject) &obj) {
    std::stack<Item *> stack;
    std::set<Item *> recursionSet;
    int rowCount = model.rowCount();
    for (int i = 0; i < rowCount; ++i) {
        stack.push(static_cast<Item *>(model.item(i)));
    }
    while (!stack.empty()) {
        auto item = stack.top();
        stack.pop();
        if (recursionSet.find(item) != recursionSet.cend()) {
            continue;
        }
        recursionSet.insert(item);
        if (item->object == obj) {
            return item;
        }
        int rowCount = item->rowCount();
        for (int i = 0; i < rowCount; ++i) {
            stack.push(static_cast<Item *>(item->child(i)));
        }
    }
    return nullptr;
}

ObjectsTreeModel::ObjectsTreeModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderLabels({tr("Name")});
}

void ObjectsTreeModel::update(const Handle(AIS_InteractiveContext) &context)
{
    clear();

    setHorizontalHeaderLabels({tr("Name")});

    AIS_ListOfInteractive allInteractive;
    context->ObjectsInside(allInteractive, AIS_KOI_Shape, 0);
    std::stack<std::pair<QStandardItem *, Handle(InteractiveObject)> > stack;
    std::set<Handle(InteractiveObject)> recursionSet;
    for (const auto &interactive : allInteractive) {
        auto obj = Handle(InteractiveObject)::DownCast(interactive);
        if (obj && !obj->Parent()) {
            stack.push(std::make_pair(nullptr, obj));
        }
    }
    while (!stack.empty()) {
        auto pair = stack.top();
        stack.pop();
        if (recursionSet.find(pair.second) != recursionSet.cend()) {
            continue;
        }
        recursionSet.insert(pair.second);
        auto item = new Item(pair.second);
        if (pair.first) {
            pair.first->appendRow(item);
        } else {
            appendRow(item);
        }
        for (const auto &child : pair.second->Children()) {
            auto obj = Handle(InteractiveObject)::DownCast(child);
            if (obj) {
                stack.push(std::make_pair(item, obj));
            }
        }
    }
}

void ObjectsTreeModel::addObject(const Handle(InteractiveObject) &object)
{
    Handle(InteractiveObject) parentObj;
    auto parent = object->Parent();
    if (parent) {
        parentObj = Handle(InteractiveObject)::DownCast(parent);
    }
    if (!parentObj) {
        appendRow(new Item(object));
    } else {
        auto parentItem = findItem(*this, parentObj);
        if (parentItem) {
            parentItem->appendRow(new Item(object));
        } else {
            appendRow(new Item(object));
        }
    }
}

void ObjectsTreeModel::removeObject(const Handle(InteractiveObject) &object)
{
    auto item = findItem(*this, object);
    if (item->parent()) {
        item->parent()->removeRow(item->row());
    } else {
        removeRow(item->row());
    }
}

Handle(InteractiveObject) ObjectsTreeModel::object(const QModelIndex &index) const
{
    Handle(InteractiveObject) ret;
    auto item = static_cast<Item *>(itemFromIndex(index));
    if (item) {
        ret = item->object;
    }
    return ret;
}

QModelIndex ObjectsTreeModel::index(const Handle(InteractiveObject) &object) const
{
    QModelIndex ret;
    auto item = findItem(*this, object);
    if (item) {
        ret = item->index();
    }
    return ret;
}
