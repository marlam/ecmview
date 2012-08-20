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

#ifndef LENS_H
#define LENS_H

#include "config.h"

#include <QWidget>

#include "lens_parameters.h"

class QCheckBox;
class QDoubleSpinBox;

class Lens : public QWidget
{
    Q_OBJECT

private:
    bool _lock;
    QCheckBox* _activate_box;
    QDoubleSpinBox* _lat_box;
    QDoubleSpinBox* _lon_box;
    QDoubleSpinBox* _rad_box;

private slots:
    void value_changed();

public:
    Lens(const class lens_parameters& lens_parameters, QWidget* parent = NULL);
    ~Lens();

public slots:
    void set_lens(class lens_parameters);

signals:
    void update_lens(class lens_parameters);
};

#endif
