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

#ifndef RENDERPROPS_H
#define RENDERPROPS_H

#include "config.h"

#include <QDialog>

#include "renderer_parameters.h"

class QLabel;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;

class RenderProps : public QDialog
{
Q_OBJECT

private:
    QLabel* _background_color_label;
    QDoubleSpinBox* _quad_screen_size_ratio_spinbox;
    QComboBox* _fixed_quadtree_depth_combobox;
    QSpinBox* _quad_subdivision_spinbox;
    QCheckBox* _wireframe_checkbox;
    QCheckBox* _bounding_boxes_checkbox;
    QCheckBox* _quad_borders_checkbox;
    QCheckBox* _mipmapping_checkbox;
    QCheckBox* _force_lod_sync_checkbox;
    QCheckBox* _statistics_overlay_checkbox;
    QSpinBox* _gpu_cache_size_spinbox;
    QSpinBox* _mem_cache_size_spinbox;

private slots:
    void get_background_color();
    void send_signal();

public:
    RenderProps(const class renderer_parameters& renderer_parameters, QWidget* parent = NULL);
    ~RenderProps();

signals:
    void update_renderer_parameters(const class renderer_parameters renderer_parameters);
};

#endif
