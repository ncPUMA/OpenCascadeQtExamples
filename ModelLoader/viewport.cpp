/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewport.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>

class ViewportPrivate
{
    friend class Viewport;

    TopoDS_Shape loadModel(const QString &path) {
        TopoDS_Shape result;
        STEPControl_Reader aReader;
        const IFSelect_ReturnStatus aStatus = aReader.ReadFile(path.toLatin1().constData());
        if (aStatus == IFSelect_RetDone) {
            bool anIsFailsOnly = false;
            aReader.PrintCheckLoad(anIsFailsOnly, IFSelect_ItemsByEntity);

            int aRootsNumber = aReader.NbRootsForTransfer();
            aReader.PrintCheckTransfer(anIsFailsOnly, IFSelect_ItemsByEntity);
            for (Standard_Integer i = 1; i <= aRootsNumber; i++) {
                aReader.TransferRoot(i);
            }
            result = aReader.OneShape();
        } else {
            qDebug() << "Error read file, error code" << aStatus;
        }
        return result;
    }

    Viewport *q_ptr;
    Handle(AIS_Shape) mModel;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QMenu menu;
        menu.addAction(tr("Load Model"), this, [this](){
            auto fName = QFileDialog::getOpenFileName(this, tr("Chose model"), QString(), tr("STEP (*.step *.stp)"));
            if (!fName.isEmpty()) {
                qDebug() << "Start loading" << fName;
                QElapsedTimer tm;
                tm.restart();
                auto shape = d_ptr->loadModel(fName);
                qDebug() << "File read complete at" << tm.elapsed() << "ms.";
                if (!shape.IsNull()) {
                    auto ctx = context();
                    ctx->Remove(d_ptr->mModel, Standard_False);
                    d_ptr->mModel = new AIS_Shape(shape);
                    tm.restart();
                    ctx->Display(d_ptr->mModel, Standard_False);
                    qDebug() << "Model display complete at" << tm.elapsed() << "ms.";
                } else {
                    qDebug() << "Error reading model";
                }
            }
        });
        return menu.exec(event->globalPos()) != nullptr;
    }
    return false;
}
