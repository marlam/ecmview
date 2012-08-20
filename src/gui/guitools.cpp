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

#include <cmath>

#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>

#include "guitools.h"


WidthHeightDialog::WidthHeightDialog(int w, int h, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    if (parent)
    {
        setWindowTitle(parent->windowTitle() + ": Width and Height");
    }

    _aspect_ratio = static_cast<double>(w) / static_cast<double>(h);
    _lock = false;

    QGridLayout *wh_layout = new QGridLayout;
    QLabel *w_label = new QLabel("Width:");
    wh_layout->addWidget(w_label, 0, 0);
    QLabel *h_label = new QLabel("Height:");
    wh_layout->addWidget(h_label, 0, 1);
    _w_spinbox = new QSpinBox();
    _w_spinbox->setRange(1, 99999);
    _w_spinbox->setValue(w);
    connect(_w_spinbox, SIGNAL(valueChanged(int)), this, SLOT(w_changed()));
    wh_layout->addWidget(_w_spinbox, 1, 0);
    _h_spinbox = new QSpinBox();
    _h_spinbox->setRange(1, 99999);
    _h_spinbox->setValue(h);
    connect(_h_spinbox, SIGNAL(valueChanged(int)), this, SLOT(h_changed()));
    wh_layout->addWidget(_h_spinbox, 1, 1);
    _ar_checkbox = new QCheckBox("Keep aspect ratio");
    _ar_checkbox->setCheckState(Qt::Checked);
    connect(_ar_checkbox, SIGNAL(stateChanged(int)), this, SLOT(ar_changed()));
    wh_layout->addWidget(_ar_checkbox, 2, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton("&OK", this);
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), this, SLOT(accept()));
    wh_layout->addWidget(ok_btn, 3, 0);
    QPushButton *cancel_btn = new QPushButton("&Cancel", this);
    connect(cancel_btn, SIGNAL(clicked()), this, SLOT(reject()));
    wh_layout->addWidget(cancel_btn, 3, 1);
    this->setLayout(wh_layout);
}

WidthHeightDialog::~WidthHeightDialog()
{
}

void WidthHeightDialog::w_changed()
{
    if (!_lock && _ar_checkbox->isChecked())
    {
        int h = std::round(static_cast<double>(_w_spinbox->value()) / _aspect_ratio);
        _lock = true;
        _h_spinbox->setValue(h);
        _lock = false;
    }
}

void WidthHeightDialog::h_changed()
{
    if (!_lock && _ar_checkbox->isChecked())
    {
        int w = std::round(static_cast<double>(_h_spinbox->value()) * _aspect_ratio);
        _lock = true;
        _w_spinbox->setValue(w);
        _lock = false;
    }
}

void WidthHeightDialog::ar_changed()
{
    if (_ar_checkbox->isChecked())
    {
        _aspect_ratio = static_cast<double>(_w_spinbox->value())
            / static_cast<double>(_h_spinbox->value());
    }
}

bool WidthHeightDialog::run(int &w, int &h)
{
    if (this->exec() == QDialog::Rejected)
    {
        return false;
    }
    w = _w_spinbox->value();
    h = _h_spinbox->value();
    return true;
}
