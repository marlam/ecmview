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

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "renderprops.h"


RenderProps::RenderProps(const class renderer_parameters& renderer_parameters, QWidget *parent) : QDialog(parent)
{
    QGridLayout *layout = new QGridLayout;

    int row = 0;

    QPushButton* background_color_button = new QPushButton("Background color:");
    connect(background_color_button, SIGNAL(pressed()), this, SLOT(get_background_color()));
    layout->addWidget(background_color_button, row, 0);
    _background_color_label = new QLabel();
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixel(0, 0, qRgb(
                renderer_parameters.background_color[0],
                renderer_parameters.background_color[1],
                renderer_parameters.background_color[2]));
    _background_color_label->setScaledContents(true);
    _background_color_label->setPixmap(QPixmap::fromImage(img));
    layout->addWidget(_background_color_label, row, 1);
    row++;

    QLabel *quad_screen_size_ratio_label = new QLabel("Max. quad screen size ratio:");
    layout->addWidget(quad_screen_size_ratio_label, row, 0);
    _quad_screen_size_ratio_spinbox = new QDoubleSpinBox(this);
    _quad_screen_size_ratio_spinbox->setRange(1.0, 9.9);
    _quad_screen_size_ratio_spinbox->setSingleStep(0.1);
    _quad_screen_size_ratio_spinbox->setValue(renderer_parameters.quad_screen_size_ratio);
    connect(_quad_screen_size_ratio_spinbox, SIGNAL(valueChanged(double)), this, SLOT(send_signal()));
    layout->addWidget(_quad_screen_size_ratio_spinbox, row, 1);
    row++;

    QLabel *fixed_quadtree_depth_label = new QLabel("Fixed quadtree depth:");
    layout->addWidget(fixed_quadtree_depth_label, row, 0);
    _fixed_quadtree_depth_combobox = new QComboBox(this);
    _fixed_quadtree_depth_combobox->addItem("Off");
    _fixed_quadtree_depth_combobox->addItem("1 levels");
    _fixed_quadtree_depth_combobox->addItem("2 levels");
    _fixed_quadtree_depth_combobox->addItem("3 levels");
    _fixed_quadtree_depth_combobox->addItem("4 levels");
    _fixed_quadtree_depth_combobox->addItem("5 levels");
    _fixed_quadtree_depth_combobox->addItem("6 levels");
    _fixed_quadtree_depth_combobox->addItem("7 levels");
    _fixed_quadtree_depth_combobox->addItem("8 levels");
    _fixed_quadtree_depth_combobox->addItem("9 levels");
    _fixed_quadtree_depth_combobox->setCurrentIndex(renderer_parameters.fixed_quadtree_depth);
    connect(_fixed_quadtree_depth_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(send_signal()));
    layout->addWidget(_fixed_quadtree_depth_combobox, row, 1);
    row++;

    QLabel *quad_subdivision_label = new QLabel("Quad subdivision level:");
    layout->addWidget(quad_subdivision_label, row, 0);
    _quad_subdivision_spinbox = new QSpinBox(this);
    _quad_subdivision_spinbox->setRange(0, 8);
    _quad_subdivision_spinbox->setSingleStep(1);
    _quad_subdivision_spinbox->setValue(renderer_parameters.quad_subdivision);
    connect(_quad_subdivision_spinbox, SIGNAL(valueChanged(int)), this, SLOT(send_signal()));
    layout->addWidget(_quad_subdivision_spinbox, row, 1);
    row++;

    QLabel *wireframe_label = new QLabel("Wireframe:");
    layout->addWidget(wireframe_label, row, 0);
    _wireframe_checkbox = new QCheckBox(this);
    _wireframe_checkbox->setChecked(renderer_parameters.wireframe);
    connect(_wireframe_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_wireframe_checkbox, row, 1);
    row++;

    QLabel *bounding_boxes_label = new QLabel("Quad Bounding Boxes:");
    layout->addWidget(bounding_boxes_label, row, 0);
    _bounding_boxes_checkbox = new QCheckBox(this);
    _bounding_boxes_checkbox->setChecked(renderer_parameters.bounding_boxes);
    connect(_bounding_boxes_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_bounding_boxes_checkbox, row, 1);
    row++;

    QLabel *quad_borders_label = new QLabel("Quad Borders:");
    layout->addWidget(quad_borders_label, row, 0);
    _quad_borders_checkbox = new QCheckBox(this);
    _quad_borders_checkbox->setChecked(renderer_parameters.quad_borders);
    connect(_quad_borders_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_quad_borders_checkbox, row, 1);
    row++;

    QLabel *mipmapping_label = new QLabel("Mipmapping:");
    layout->addWidget(mipmapping_label, row, 0);
    _mipmapping_checkbox = new QCheckBox(this);
    _mipmapping_checkbox->setChecked(renderer_parameters.mipmapping);
    connect(_mipmapping_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_mipmapping_checkbox, row, 1);
    row++;

    QLabel *force_lod_sync_label = new QLabel("Force LOD sync:");
    layout->addWidget(force_lod_sync_label, row, 0);
    _force_lod_sync_checkbox = new QCheckBox(this);
    _force_lod_sync_checkbox->setChecked(renderer_parameters.force_lod_sync);
    connect(_force_lod_sync_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_force_lod_sync_checkbox, row, 1);
    row++;

    QLabel *statistics_overlay_label = new QLabel("Statistics Overlay:");
    layout->addWidget(statistics_overlay_label, row, 0);
    _statistics_overlay_checkbox = new QCheckBox(this);
    _statistics_overlay_checkbox->setChecked(renderer_parameters.statistics_overlay);
    connect(_statistics_overlay_checkbox, SIGNAL(toggled(bool)), this, SLOT(send_signal()));
    layout->addWidget(_statistics_overlay_checkbox, row, 1);
    row++;

    QLabel *gpu_cache_size_label = new QLabel("GPU cache size (MB):");
    layout->addWidget(gpu_cache_size_label, row, 0);
    _gpu_cache_size_spinbox = new QSpinBox(this);
    _gpu_cache_size_spinbox->setRange(0, sizeof(size_t) <= 4 ? 2048 : 16384);
    _gpu_cache_size_spinbox->setSingleStep(32);
    _gpu_cache_size_spinbox->setValue(renderer_parameters.gpu_cache_size / static_cast<size_t>(1 << 20));
    connect(_gpu_cache_size_spinbox, SIGNAL(valueChanged(int)), this, SLOT(send_signal()));
    layout->addWidget(_gpu_cache_size_spinbox, row, 1);
    row++;

    QLabel *mem_cache_size_label = new QLabel("Memory cache size (MB):");
    layout->addWidget(mem_cache_size_label, row, 0);
    _mem_cache_size_spinbox = new QSpinBox(this);
    _mem_cache_size_spinbox->setRange(0, sizeof(size_t) <= 4 ? 2048 : 16384);
    _mem_cache_size_spinbox->setSingleStep(32);
    _mem_cache_size_spinbox->setValue(renderer_parameters.mem_cache_size / static_cast<size_t>(1 << 20));
    connect(_mem_cache_size_spinbox, SIGNAL(valueChanged(int)), this, SLOT(send_signal()));
    layout->addWidget(_mem_cache_size_spinbox, row, 1);
    row++;

    layout->setRowStretch(row, 1);
    setLayout(layout);
    setModal(false);
    if (parent) {
        setWindowTitle(parent->windowTitle() + ": Render Properties");
    }
}

RenderProps::~RenderProps()
{
}

void RenderProps::get_background_color()
{
    unsigned int rgb = _background_color_label->pixmap()->toImage().pixel(0, 0);
    QColor color = QColorDialog::getColor(QColor(qRed(rgb), qGreen(rgb), qBlue(rgb)), this);
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixel(0, 0, qRgb(color.red(), color.green(), color.blue()));
    _background_color_label->setPixmap(QPixmap::fromImage(img));
    send_signal();
}

void RenderProps::send_signal()
{
    renderer_parameters renderer_params;
    unsigned int rgb = _background_color_label->pixmap()->toImage().pixel(0, 0);
    renderer_params.background_color[0] = qRed(rgb);
    renderer_params.background_color[1] = qGreen(rgb);
    renderer_params.background_color[2] = qBlue(rgb);
    renderer_params.quad_screen_size_ratio = _quad_screen_size_ratio_spinbox->value();
    renderer_params.fixed_quadtree_depth = _fixed_quadtree_depth_combobox->currentIndex();
    renderer_params.quad_subdivision = _quad_subdivision_spinbox->value();
    renderer_params.wireframe = _wireframe_checkbox->isChecked();
    renderer_params.bounding_boxes = _bounding_boxes_checkbox->isChecked();
    renderer_params.quad_borders = _quad_borders_checkbox->isChecked();
    renderer_params.mipmapping = _mipmapping_checkbox->isChecked();
    renderer_params.force_lod_sync = _force_lod_sync_checkbox->isChecked();
    renderer_params.statistics_overlay = _statistics_overlay_checkbox->isChecked();
    renderer_params.gpu_cache_size = static_cast<size_t>(_gpu_cache_size_spinbox->value()) * static_cast<size_t>(1 << 20);
    renderer_params.mem_cache_size = static_cast<size_t>(_mem_cache_size_spinbox->value()) * static_cast<size_t>(1 << 20);
    emit update_renderer_parameters(renderer_params);
}
