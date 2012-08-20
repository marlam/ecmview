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

#include "config.h"

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>

#include "str.h"

#include "glvm.h"
#include "glvm-str.h"

#include "guitools.h"
#include "info.h"

using namespace glvm;


Info::Info(QSettings* settings, QWidget *parent) :
    QWidget(parent), _settings(settings)
{
    QGridLayout *layout = new QGridLayout;

    _approx_info = new QLabel();
    _approx_info->setEnabled(true);
    layout->addWidget(_approx_info, 0, 0);
    layout->addWidget(new QLabel("  "), 0, 1);
    _viewer_info = new QCheckBox();
    connect(_viewer_info, SIGNAL(toggled(bool)), this, SLOT(prefs_changed()));
    layout->addWidget(_viewer_info, 0, 2);
    layout->addWidget(new QLabel("  "), 0, 3);
    _pointer_info = new QCheckBox("Pointer:");
    connect(_pointer_info, SIGNAL(toggled(bool)), this, SLOT(prefs_changed()));
    layout->addWidget(_pointer_info, 0, 4);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(3, 1);
    layout->setRowStretch(1, 1);
    setLayout(layout);
}

void Info::init()
{
    _approx_info->setText("Approximated quads: 123");
    _approx_info->setFixedSize(_approx_info->sizeHint());
    _approx_info->setText("Approximated quads:");
    _viewer_info->setText("Viewer: elev 12345.6 km");
    _viewer_info->setFixedSize(_viewer_info->sizeHint());
    _viewer_info->setText("Viewer:");
    _pointer_info->setText("Pointer: lat 01.234567 lon 01.234567 elev -1234.5 m");
    _pointer_info->setFixedSize(_pointer_info->sizeHint());
    _pointer_info->setText("Pointer:");

    _viewer_info->setChecked(_settings->value("Session/viewer-info", false).toBool());
    _pointer_info->setChecked(_settings->value("Session/pointer-info", false).toBool());
    prefs_changed();
}

void Info::prefs_changed()
{
    _settings->setValue("Session/viewer-info", _viewer_info->isChecked());
    _settings->setValue("Session/pointer-info", _pointer_info->isChecked());
    emit update_info_prefs(_viewer_info->isChecked(), _pointer_info->isChecked());
}

void Info::update_approx_info(int approximated_tiles)
{
    _approx_info->setText(str::asprintf("Approximated quads: %d", approximated_tiles).c_str());
}

void Info::update_viewer_info(double /* lat */, double /* lon */, double elev)
{
    if (_viewer_info->isChecked()) {
        _viewer_info->setText(str::asprintf("Viewer: elev %s", str::human_readable_length(elev).c_str()).c_str());
    } else {
        _viewer_info->setText("Viewer:");
    }
}

void Info::update_pointer_info(bool valid, double lat, double lon, double elev)
{
    if (_pointer_info->isChecked() && valid) {
        dvec3 pointer(lat, lon, elev);
        _pointer_info->setText(str::asprintf("Pointer: %s", str::human_readable_geodetic(pointer.x, pointer.y, pointer.z).c_str()).c_str());
    } else {
        _pointer_info->setText("Pointer:");
    }
}
