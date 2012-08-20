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

#include "config.h"

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QDial>
#include <QLabel>

#include "msg.h"

#include "glvm.h"

#include "light.h"

using namespace glvm;


Light::Light(const class light_parameters& light_parameters, QWidget* parent) :
    QWidget(parent),
    _unit_sphere(1.0, 1.0), _viewer_lat(0.0), _viewer_lon(0.0),
    _lock(false)
{
    QGridLayout* layout = new QGridLayout;

    _activate_box = new QCheckBox("Activate Lighting");
    connect(_activate_box, SIGNAL(stateChanged(int)), this, SLOT(value_changed()));
    layout->addWidget(_activate_box, 0, 0, 1, 2);

    QPushButton* ambient_button = new QPushButton("Ambient color:");
    connect(ambient_button, SIGNAL(pressed()), this, SLOT(set_ambient()));
    layout->addWidget(ambient_button, 1, 0);
    _ambient_label = new QLabel();
    _ambient_label->setScaledContents(true);
    layout->addWidget(_ambient_label, 1, 1);

    QPushButton* color_button = new QPushButton("Color:");
    connect(color_button, SIGNAL(pressed()), this, SLOT(set_color()));
    layout->addWidget(color_button, 2, 0);
    _color_label = new QLabel();
    _color_label->setScaledContents(true);
    layout->addWidget(_color_label, 2, 1);

    layout->addWidget(new QLabel("Shininess:"), 3, 0);
    _shininess_slider = new QSlider(Qt::Horizontal);
    _shininess_slider->setRange(0, 1000);
    _shininess_slider->setSingleStep(10);
    connect(_shininess_slider, SIGNAL(valueChanged(int)), this, SLOT(value_changed()));
    layout->addWidget(_shininess_slider, 3, 1);

    layout->addWidget(new QLabel("Positioning:"), 4, 0);
    _method_combobox = new QComboBox();
    _method_combobox->addItem("Global lon/lat");
    _method_combobox->addItem("Viewer az/lat");
    _method_combobox->setCurrentIndex(0);
    connect(_method_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
    layout->addWidget(_method_combobox, 4, 1);

    _dial = new QDial();
    _dial->setWrapping(true);
    _dial->setRange(-1800, +1800);
    _dial->setSingleStep(10);
    connect(_dial, SIGNAL(valueChanged(int)), this, SLOT(value_changed()));
    layout->addWidget(_dial, 5, 0);

    _slider = new QSlider(Qt::Horizontal);
    _slider->setRange(-900, +900);
    _slider->setSingleStep(10);
    connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(value_changed()));
    layout->addWidget(_slider, 5, 1);

    layout->setRowStretch(6, 1);
    layout->setColumnStretch(1, 1);
    setLayout(layout);

    set_light(light_parameters);
}

Light::~Light()
{
}

vec3 Light::gui_to_dir(int method)
{
    if (method == 0) {
        dvec3 dir;
        _unit_sphere.geocentric_to_cartesian(radians(_slider->value() / 10.0), radians(_dial->value() / 10.0), 0.0, dir.vl);
        return vec3(dir);
    } else {
        dvec3 v;
        _unit_sphere.geocentric_to_cartesian(_viewer_lat, _viewer_lon, 0.0, v.vl);
        dvec3 local_dir;
        _unit_sphere.geocentric_to_cartesian(
                radians((_slider->value() + 900) / 20.0),
                radians((1800 - _dial->value()) / 10.0),
                0.0, local_dir.vl);
        dvec3 dir = toQuat(dvec3(0.0, 0.0, 1.0), v) * local_dir;
        return vec3(dir);
    }
}

void Light::dir_to_gui(int method, const vec3& dir)
{
    if (method == 0) {
        dvec3 d(dir);
        dvec2 geoc;
        _unit_sphere.cartesian_to_geocentric(d.vl, &(geoc[0]), &(geoc[1]));
        _slider->setValue(degrees(geoc[0]) * 10.0);
        _dial->setValue(degrees(geoc[1]) * 10.0);
    } else {
        dvec3 v;
        _unit_sphere.geocentric_to_cartesian(_viewer_lat, _viewer_lon, 0.0, v.vl);
        dvec3 local_dir = toQuat(v, dvec3(0.0, 0.0, 1.0)) * dvec3(dir);
        dvec2 local_geoc;
        _unit_sphere.cartesian_to_geocentric(local_dir.vl, &(local_geoc[0]), &(local_geoc[1]));
        _slider->setValue(degrees(local_geoc[0]) * 20.0);
        _dial->setValue(1800 - degrees(local_geoc[1]) * 10.0);
    }
}

void Light::method_changed()
{
    int to_method = _method_combobox->currentIndex();
    int from_method = (to_method == 0 ? 1 : 0);
    vec3 d = gui_to_dir(from_method);
    dir_to_gui(to_method, d);
}

void Light::value_changed()
{
    if (!_lock) {
        class light_parameters lp;
        lp.active = _activate_box->isChecked();
        lp.ambient = vec3(_ambient.red() / 255.0f, _ambient.green() / 255.0f, _ambient.blue() / 255.0f);
        lp.shininess = _shininess_slider->value() / 10000.0f;
        lp.color = vec3(_color.red() / 255.0f, _color.green() / 255.0f, _color.blue() / 255.0f);
        lp.dir = gui_to_dir(_method_combobox->currentIndex());
        emit update_light(lp);
    }
}

void Light::set_ambient()
{
    QColor color = QColorDialog::getColor(_ambient, this);
    _ambient = color;
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixel(0, 0, qRgb(color.red(), color.green(), color.blue()));
    _ambient_label->setPixmap(QPixmap::fromImage(img));
    value_changed();
}

void Light::set_color()
{
    QColor color = QColorDialog::getColor(_color, this);
    _color = color;
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixel(0, 0, qRgb(color.red(), color.green(), color.blue()));
    _color_label->setPixmap(QPixmap::fromImage(img));
    value_changed();
}

void Light::set_viewer_info(double lat, double lon, double /* elev */)
{
    _viewer_lat = lat;
    _viewer_lon = lon;
}

void Light::set_light(class light_parameters lp)
{
    QColor color;
    QImage img(1, 1, QImage::Format_RGB888);

    _lock = true;
    _activate_box->setChecked(lp.active);

    color.setRgbF(lp.ambient.r, lp.ambient.g, lp.ambient.b);
    img.setPixel(0, 0, qRgb(color.red(), color.green(), color.blue()));
    _ambient = color;
    _ambient_label->setPixmap(QPixmap::fromImage(img));

    color.setRgbF(lp.color.r, lp.color.g, lp.color.b);
    img.setPixel(0, 0, qRgb(color.red(), color.green(), color.blue()));
    _color = color;
    _color_label->setPixmap(QPixmap::fromImage(img));

    _shininess_slider->setValue(lp.shininess * 10000.0f);
    dir_to_gui(_method_combobox->currentIndex(), lp.dir);
    _lock = false;
}
