/*
 * Copyright (C) 2011, 2012
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

#ifndef LIGHT_H
#define LIGHT_H

#include "config.h"

#include <QWidget>

#include <ecmdb/ecm.h>

#include "state.h"
#include "glvm.h"

class QDial;
class QLabel;
class QSlider;
class QComboBox;
class QCheckBox;

class Light : public QWidget
{
    Q_OBJECT

private:
    ecm _unit_sphere;
    double _viewer_lat, _viewer_lon;
    bool _lock;
    QCheckBox* _activate_box;
    QColor _ambient;
    QLabel* _ambient_label;
    QColor _color;
    QLabel* _color_label;
    QSlider* _shininess_slider;
    QComboBox* _method_combobox;
    QDial* _dial;
    QSlider* _slider;

    glvm::vec3 gui_to_dir(int method);
    void dir_to_gui(int method, const glvm::vec3& dir);

private slots:
    void method_changed();
    void value_changed();
    void set_ambient();
    void set_color();

public:
    Light(const class light_parameters& light_parameters, QWidget* parent = NULL);
    ~Light();

public slots:
    void set_viewer_info(double lat, double lon, double elev);
    void set_light(class light_parameters);

signals:
    void update_light(class light_parameters);
};

#endif
