/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXAMPLESBASE_GLOBAL_H
#define EXAMPLESBASE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLESBASE_LIBRARY)
#  define EXAMPLESBASE_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLESBASE_EXPORT Q_DECL_IMPORT
#endif

#endif // EXAMPLESBASE_GLOBAL_H
