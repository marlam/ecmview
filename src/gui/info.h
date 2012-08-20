/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFO_H
#define INFO_H

#include "config.h"

#include <QWidget>

#include "glvm.h"

class QLabel;
class QCheckBox;
class QSettings;

class Info : public QWidget
{
Q_OBJECT

private:
    QSettings* _settings;
    QLabel* _approx_info;
    QCheckBox* _viewer_info;
    QCheckBox* _pointer_info;

public:
    Info(QSettings* settings, QWidget* parent = NULL);
    void init();

private slots:
    void prefs_changed();

public slots:
    void update_approx_info(int approximated_quads);
    void update_viewer_info(double lat, double lon, double elev);
    void update_pointer_info(bool valid, double lat, double lon, double elev);

signals:
    void update_info_prefs(bool want_viewer_info, bool want_pointer_info);
};

#endif
