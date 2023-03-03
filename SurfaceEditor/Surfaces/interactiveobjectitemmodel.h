#ifndef INTERACTIVEOBJECTITEMMODEL_H
#define INTERACTIVEOBJECTITEMMODEL_H

#include <QStandardItemModel>

#include <Standard_Handle.hxx>

class InteractiveObject;
class InteractiveObjectItemModelPrivate;

class InteractiveObjectItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit InteractiveObjectItemModel(const Handle(InteractiveObject) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModel();

    void update();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) final;

protected:
    template <typename T>
    inline static QList<QStandardItem *> createRow(const QString &text, std::map <T, QStandardItem *> &map, T id) {
        QList<QStandardItem *> ret = { new QStandardItem(text), new QStandardItem };
        ret[0]->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        map[id] = ret[1];
        return ret;
    }

    inline static QStandardItem *createGroup(const QString &text) {
        auto ret = new QStandardItem(text);
        ret->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        return ret;
    }

    template <typename T>
    bool findField(const QModelIndex &index, std::map <T, QStandardItem *> &map, T &id) const {
        auto item = itemFromIndex(index);
        auto it = std::find_if(map.cbegin(), map.cend(), [item](const std::pair<T, QStandardItem *> &pair){
            return pair.second == item;
        });
        if (it != map.cend()) {
            id = it->first;
            return true;
        }
        return false;
    }

    virtual void updateFields();
    virtual bool setFieldData(const QModelIndex &index, const QVariant &value);
    virtual QWidget *fieldEditor(const QModelIndex &index) const;

    QWidget *createFieldEditor(const QModelIndex &index) const;

private:
    InteractiveObjectItemModelPrivate *d;
    friend class InteractiveObjectItemModelDelegate;
};

#endif // INTERACTIVEOBJECTITEMMODEL_H
