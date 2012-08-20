/*
 * Copyright (C) 2009, 2010, 2011, 2012
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
#include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include "msg.h"

#include "glvm.h"

#include "lens.h"


Lens::Lens(const class lens_parameters& lens_parameters, QWidget* parent) : QWidget(parent), _lock(false)
{
    QGridLayout* layout = new QGridLayout;

    _activate_box = new QCheckBox("Activate Lens");
    _activate_box->setChecked(lens_parameters.active);
    connect(_activate_box, SIGNAL(stateChanged(int)), this, SLOT(value_changed()));
    layout->addWidget(_activate_box, 0, 0, 1, 3);

    layout->addWidget(new QLabel("Latitude:"), 1, 0);
    _lat_box = new QDoubleSpinBox();
    _lat_box->setRange(-90.0, +90.0);
    _lat_box->setDecimals(7);
    connect(_lat_box, SIGNAL(valueChanged(double)), this, SLOT(value_changed()));
    layout->addWidget(_lat_box, 1, 1);
    layout->addWidget(new QLabel("<html>&ordm;</html>"), 1, 2);

    layout->addWidget(new QLabel("Longitude:"), 2, 0);
    _lon_box = new QDoubleSpinBox();
    _lon_box->setRange(-180.0, +180.0);
    _lon_box->setDecimals(7);
    connect(_lon_box, SIGNAL(valueChanged(double)), this, SLOT(value_changed()));
    layout->addWidget(_lon_box, 2, 1);
    layout->addWidget(new QLabel("<html>&ordm;</html>"), 2, 2);

    layout->addWidget(new QLabel("Radius:"), 3, 0);
    _rad_box = new QDoubleSpinBox();
    _rad_box->setRange(1.0, 100000000.0);
    _rad_box->setDecimals(3);
    connect(_rad_box, SIGNAL(valueChanged(double)), this, SLOT(value_changed()));
    layout->addWidget(_rad_box, 3, 1);
    layout->addWidget(new QLabel("m"), 3, 2);

    layout->setRowStretch(4, 1);
    layout->setColumnStretch(1, 1);
    setLayout(layout);

    set_lens(lens_parameters);
}

Lens::~Lens()
{
}

void Lens::value_changed()
{
    if (!_lock) {
        class lens_parameters lp;
        lp.active = _activate_box->isChecked();
        lp.pos = glvm::radians(glvm::dvec2(_lat_box->value(), _lon_box->value()));
        lp.radius = _rad_box->value();
        emit update_lens(lp);
    }
}

void Lens::set_lens(class lens_parameters lp)
{
    _lock = true;
    _activate_box->setChecked(lp.active);
    _lat_box->setValue(glvm::degrees(lp.pos.x));
    _lon_box->setValue(glvm::degrees(lp.pos.y));
    _rad_box->setValue(lp.radius);
    _lock = false;
}
