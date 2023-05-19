#include "interactiveobjectitemmodeldelegate.h"

#include <QAbstractItemView>
#include <QComboBox>

#include "interactiveobjectitemmodel.h"

namespace ExamplesBase {

InteractiveObjectItemModelDelegate::InteractiveObjectItemModelDelegate(QAbstractItemView *itemView)
    : QStyledItemDelegate(itemView)
{

}

QWidget *InteractiveObjectItemModelDelegate::createEditor(QWidget *parent,
                                                          const QStyleOptionViewItem &option,
                                                          const QModelIndex &index) const
{
    Q_UNUSED(option);

    auto mdl = model();
    if (mdl) {
        auto widget = mdl->createFieldEditor(index);
        if (widget) {
            widget->setParent(parent);
        }
        return widget;
    }
    return nullptr;
}

void InteractiveObjectItemModelDelegate::setModelData(QWidget *editor,
                                                      QAbstractItemModel *model,
                                                      const QModelIndex &index) const
{
    auto cb = qobject_cast<QComboBox *>(editor);
    if (cb) {
        model->setData(index, cb->currentData());
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

InteractiveObjectItemModel *InteractiveObjectItemModelDelegate::model() const
{
    InteractiveObjectItemModel *ret = nullptr;
    auto view = qobject_cast<QAbstractItemView *>(parent());
    if (view) {
        ret = qobject_cast<InteractiveObjectItemModel *>(view->model());
    }
    return ret;
}

}
