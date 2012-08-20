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
#include <QSpinBox>
#include <QSlider>
#include <QLabel>

#include "glvm.h"
#include "glvm-str.h"

#include "quaddebug.h"


QuadDebug::QuadDebug(QWidget *parent) : QWidget(parent), _lock(false)
{
    QGridLayout *layout = new QGridLayout;

    _activate_box = new QCheckBox("Activate Quad debugging");
    connect(_activate_box, SIGNAL(stateChanged(int)), this, SLOT(activate_box_changed()));
    layout->addWidget(_activate_box, 0, 0, 1, 2);

    layout->addWidget(new QLabel("Quad index:"), 1, 0);
    _index_spinbox = new QSpinBox();
    _index_spinbox->setRange(0, 999);
    connect(_index_spinbox, SIGNAL(valueChanged(int)), this, SLOT(index_spinbox_changed(int)));
    layout->addWidget(_index_spinbox, 1, 1);

    _index_slider = new QSlider(Qt::Horizontal);
    _index_slider->setRange(0, 999);
    connect(_index_slider, SIGNAL(valueChanged(int)), this, SLOT(index_slider_changed(int)));
    layout->addWidget(_index_slider, 2, 0, 1, 2);

    _save_box = new QCheckBox("Save quad data");
    connect(_save_box, SIGNAL(stateChanged(int)), this, SLOT(save_box_changed()));
    layout->addWidget(_save_box, 3, 0, 1, 2);

    layout->addWidget(new QLabel("Quad:"), 4, 0, 1, 2);
    _quad_label = new QLabel("");
    layout->addWidget(_quad_label, 5, 0, 1, 2);

    layout->addWidget(new QLabel("Max dist to QP / MinE / MaxE:"), 6, 0, 1, 2);
    _elev_label = new QLabel("");
    layout->addWidget(_elev_label, 7, 0, 1, 2);

    layout->setRowStretch(8, 1);
    layout->setColumnStretch(1, 1);
    setLayout(layout);
}

QuadDebug::~QuadDebug()
{
}

void QuadDebug::activate_box_changed()
{
    int index = (_activate_box->isChecked() ? _index_spinbox->value() : -1);
    emit change(index, _save_box->isChecked());
}

void QuadDebug::index_spinbox_changed(int i)
{
    if (!_lock) {
        _lock = true;
        _index_slider->setValue(i);
        emit change(_activate_box->isChecked() ? i : -1, _save_box->isChecked());
        _lock = false;
    }
}

void QuadDebug::index_slider_changed(int i)
{
    if (!_lock) {
        _lock = true;
        _index_spinbox->setValue(i);
        emit change(_activate_box->isChecked() ? i : -1, _save_box->isChecked());
        _lock = false;
    }
}

void QuadDebug::save_box_changed()
{
    int index = (_activate_box->isChecked() ? _index_spinbox->value() : -1);
    emit change(index, _save_box->isChecked());
}

void QuadDebug::set_index_range(int max)
{
    _index_spinbox->setRange(0, max);
    _index_slider->setRange(0, max);
}

void QuadDebug::set_quad_info(int s, int l, int x, int y, float max_dist_to_quad_plane, float min_elev, float max_elev)
{
    _save_box->setChecked(false);
    if (s >= 0) {
        _quad_label->setText(str::asprintf("[ %d %d %d %d ]", s, l, x, y).c_str());
        _elev_label->setText(str::asprintf("[ %g %g %g ]", max_dist_to_quad_plane, min_elev, max_elev).c_str());
    } else {
        _quad_label->setText("");
        _elev_label->setText("");
    }
}
