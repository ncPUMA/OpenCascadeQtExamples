/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_ASPECTWINDOW_H
#define EB_ASPECTWINDOW_H

#include <Aspect_Window.hxx>

class QWidget;

namespace ExamplesBase {

class AspectWindowPrivate;

class AspectWindow : public Aspect_Window
{
public:
    AspectWindow(QWidget *view);
    ~AspectWindow();

    Aspect_Drawable NativeHandle() const final;
    Aspect_Drawable NativeParentHandle() const final;

    Aspect_TypeOfResize DoResize() final;

    Standard_Boolean IsMapped() const final;
    Standard_Boolean DoMapping() const final;
    void Map() const final;
    void Unmap() const final;

    void Position(Standard_Integer &theX1, Standard_Integer &theY1,
                  Standard_Integer &theX2, Standard_Integer &theY2 ) const final;
    Standard_Real Ratio() const final;
    void Size(Standard_Integer& theWidth, Standard_Integer& theHeight) const final;

    Aspect_FBConfig NativeFBConfig() const final;

private:
    AspectWindowPrivate * const d_ptr;
};

}

#endif // EB_ASPECTWINDOW_H
