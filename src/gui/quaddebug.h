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

#ifndef QUADDEBUG_H
#define QUADDEBUG_H

#include "config.h"

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QSlider;
class QLabel;

class QuadDebug : public QWidget
{
    Q_OBJECT

private:
    bool _lock;
    QCheckBox *_activate_box;
    QSpinBox *_index_spinbox;
    QSlider* _index_slider;
    QCheckBox *_save_box;
    QLabel* _quad_label;
    QLabel* _elev_label;

private slots:
    void activate_box_changed();
    void index_spinbox_changed(int);
    void index_slider_changed(int);
    void save_box_changed();

public:
    QuadDebug(QWidget* parent = NULL);
    ~QuadDebug();

public slots:
    void set_index_range(int max);
    void set_quad_info(int s, int l, int x, int y, float max_dist_to_quad_plane, float min_elev, float max_elev);

signals:
    void change(int index, bool save);
};

#endif
