/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "aspectwindow.h"

#include <QWidget>

namespace ExamplesBase {

static const int RESIZE_PRECISION = 2;

class AspectWindowPrivate
{
    friend class AspectWindow;

    enum EN_ResizedSideFlags
    {
        ENRSF_NO     = 0x00,
        ENRSF_LEFT   = 0x01,
        ENRSF_RIGHT  = 0x02,
        ENRSF_TOP    = 0x04,
        ENRSF_BOTTOM = 0x08,

        ENRSF_TOP_LEFT     = ENRSF_LEFT  + ENRSF_TOP,
        ENRSF_TOP_RIGHT    = ENRSF_RIGHT + ENRSF_TOP,
        ENRSF_BOTTOM_LEFT  = ENRSF_LEFT  + ENRSF_BOTTOM,
        ENRSF_BOTTOM_RIGHT = ENRSF_RIGHT + ENRSF_BOTTOM
    };
    typedef int TResizedSideFlag;

    AspectWindowPrivate(QWidget *view)
        : wdgt(view)
        , oldGeometry(view->rect())
    {
    }

    inline static TResizedSideFlag dedicateSide(const QRect &oldGeom,
                                                const QRect &newGeom) {
        TResizedSideFlag result = ENRSF_NO;
        if (std::abs(newGeom.left() - oldGeom.left()) > RESIZE_PRECISION)
            result += ENRSF_LEFT;
        if (std::abs(newGeom.right() - oldGeom.right()) > RESIZE_PRECISION)
            result += ENRSF_RIGHT;
        if (std::abs(newGeom.top() - oldGeom.top()) > RESIZE_PRECISION)
            result += ENRSF_TOP;
        if (std::abs(newGeom.bottom() - oldGeom.bottom()) > RESIZE_PRECISION)
            result += ENRSF_BOTTOM;
        return result;
    }

    Aspect_TypeOfResize updateGeometry() {
        const std::map <TResizedSideFlag, Aspect_TypeOfResize> mapResizeTypes = {
            { ENRSF_NO          , Aspect_TOR_NO_BORDER               },
            { ENRSF_LEFT        , Aspect_TOR_LEFT_BORDER             },
            { ENRSF_RIGHT       , Aspect_TOR_RIGHT_BORDER            },
            { ENRSF_TOP         , Aspect_TOR_TOP_BORDER              },
            { ENRSF_BOTTOM      , Aspect_TOR_BOTTOM_BORDER           },
            { ENRSF_TOP_LEFT    , Aspect_TOR_LEFT_AND_TOP_BORDER     },
            { ENRSF_TOP_RIGHT   , Aspect_TOR_TOP_AND_RIGHT_BORDER    },
            { ENRSF_BOTTOM_LEFT , Aspect_TOR_BOTTOM_AND_LEFT_BORDER  },
            { ENRSF_BOTTOM_RIGHT, Aspect_TOR_RIGHT_AND_BOTTOM_BORDER }
        };

        Aspect_TypeOfResize result = Aspect_TOR_UNKNOWN;
        if (!wdgt->isMinimized()) {
            const QRect newGeometry = wdgt->geometry();
            const TResizedSideFlag side = dedicateSide(oldGeometry, newGeometry);
            auto it = mapResizeTypes.find(side);
            if (it != mapResizeTypes.cend())
                result = it->second;
            oldGeometry = newGeometry;
        }
        return result;
    }

    QRect wdgtGeometry() const {
        QRect result;
        if (wdgt)
            result = wdgt->geometry();
        return result;
    }

    QWidget *wdgt;
    QRect oldGeometry;
};



AspectWindow::AspectWindow(QWidget *view) :
    Aspect_Window(),
    d_ptr(new AspectWindowPrivate(view))
{
    SetBackground(Quantity_NOC_WHITE);
}

AspectWindow::~AspectWindow()
{
    delete d_ptr;
}

Aspect_Drawable AspectWindow::NativeHandle() const
{
    // static cast not works in windows and reinterpet cast in linux
    return (Aspect_Drawable)(d_ptr->wdgt->winId());
}

Aspect_Drawable AspectWindow::NativeParentHandle() const
{
    WId parentId = 0;
    const QWidget * const parent = d_ptr->wdgt->parentWidget();
    if (parent)
        parentId = parent->winId();
    return (Aspect_Drawable)parentId;
}

Aspect_TypeOfResize AspectWindow::DoResize()
{
    return d_ptr->updateGeometry();
}

Standard_Boolean AspectWindow::IsMapped() const
{
    return !(d_ptr->wdgt->isMinimized() || d_ptr->wdgt->isHidden());
}

Standard_Boolean AspectWindow::DoMapping() const
{
    return Standard_True;
}

void AspectWindow::Map() const
{
    d_ptr->wdgt->show();
    d_ptr->wdgt->update();
}

void AspectWindow::Unmap() const
{
    d_ptr->wdgt->hide();
    d_ptr->wdgt->update();
}

void AspectWindow::Position(Standard_Integer &theX1, Standard_Integer &theY1,
                            Standard_Integer &theX2, Standard_Integer &theY2) const
{
    const QRect geometry = d_ptr->wdgtGeometry();
    theX1 = geometry.left();
    theX2 = geometry.right();
    theY1 = geometry.top();
    theY2 = geometry.bottom();
}

Standard_Real AspectWindow::Ratio() const
{
    const QRectF geometry = d_ptr->wdgtGeometry();
    return static_cast <Standard_Real>
            ((geometry.right() - geometry.left()) /
             (geometry.bottom() - geometry.top()));
}

void AspectWindow::Size(Standard_Integer &theWidth, Standard_Integer &theHeight) const
{
    const QRect geometry = d_ptr->wdgtGeometry();
    theWidth  = geometry.width();
    theHeight = geometry.height();
}

Aspect_FBConfig AspectWindow::NativeFBConfig() const
{
    return nullptr;
}

}
