#ifndef INTERACTIVEOBJECTITEMMODELDELEGATE_H
#define INTERACTIVEOBJECTITEMMODELDELEGATE_H

#include <QStyledItemDelegate>

class InteractiveObjectItemModel;

class InteractiveObjectItemModelDelegate : public QStyledItemDelegate
{
public:
    InteractiveObjectItemModelDelegate(QAbstractItemView *itemView);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const final;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private:
    InteractiveObjectItemModel *model() const;
};

#endif // INTERACTIVEOBJECTITEMMODELDELEGATE_H
