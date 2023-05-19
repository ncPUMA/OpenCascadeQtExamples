#include "interactiveobjectitemmodelsurface.h"

#include <QComboBox>

#include "../Objects/interactivesurface.h"

namespace ExamplesBase {

class InteractiveObjectItemModelSurfacePrivate
{
    friend class InteractiveObjectItemModelSurface;

    QString surfaceTypeText(InteractiveSurface::SurfaceTypes surfaceType) const {
        QString ret;
        auto it = surfaceTypeTextMap.find(surfaceType);
        if (it != surfaceTypeTextMap.cend()) {
            ret = it->second;
        } else {
            ret = InteractiveObjectItemModelSurface::tr("Type %1").arg(surfaceType);
        }
        return ret;
    }

    Handle(InteractiveSurface) surface;
    QStandardItem *surfaceType = nullptr;
    std::map <InteractiveSurface::SurfaceTypes, QString> surfaceTypeTextMap = {
        { InteractiveSurface::SurfaceTypeSurface, InteractiveObjectItemModelSurface::tr("Surface") },
        { InteractiveSurface::SurfaceTypeGlass, InteractiveObjectItemModelSurface::tr("Glass") },
        { InteractiveSurface::SurfaceTypeMirror, InteractiveObjectItemModelSurface::tr("Mirror") },
    };
};

InteractiveObjectItemModelSurface::InteractiveObjectItemModelSurface(const Handle(InteractiveSurface) &object, QObject *parent)
    : InteractiveObjectItemModel(object, parent)
    , d(new InteractiveObjectItemModelSurfacePrivate)
{
    d->surface = object;

    QList<QStandardItem *> items = { new QStandardItem(tr("Surface type")), new QStandardItem };
    items[0]->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    d->surfaceType = items[1];
    appendRow(items);
}

InteractiveObjectItemModelSurface::~InteractiveObjectItemModelSurface()
{
    delete d;
}

void InteractiveObjectItemModelSurface::updateFields()
{
    InteractiveSurface::SurfaceTypes type = d->surface->surfaceType();
    d->surfaceType->setData(type);
    d->surfaceType->setText(d->surfaceTypeText(type));

    updateSurfaceFields();
}

bool InteractiveObjectItemModelSurface::setFieldData(const QModelIndex &index, const QVariant &value)
{
    auto item = itemFromIndex(index);
    if (item == d->surfaceType) {
        bool ok = false;
        int vl = value.toInt(&ok);
        if (!ok) {
            return false;
        }
        d->surface->setSurfaceType(static_cast<InteractiveSurface::SurfaceTypes>(vl));
        return true;
    }

    return setSurfaceFieldData(index, value);
}

QWidget *InteractiveObjectItemModelSurface::fieldEditor(const QModelIndex &index) const
{
    auto item = itemFromIndex(index);
    if (item == d->surfaceType) {
        auto cb = new QComboBox;
        for (const auto &pair : d->surfaceTypeTextMap) {
            cb->addItem(pair.second, pair.first);
        }
        return cb;
    }

    return surfaceFieldEditor(index);
}

}
