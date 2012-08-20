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
#include <QLabel>
#include <QGroupBox>

#include "str.h"

#include "guitools.h"
#include "statistics.h"


Statistics::Statistics(QWidget* parent) : QDialog(parent)
{
    QGridLayout* layout = new QGridLayout;
    int layout_row = 0;

    _gui_box = new QGroupBox("GUI");
    QGridLayout* _gui_box_layout = new QGridLayout;
    QLabel* gui_box_fps_label = new QLabel("FPS:");
    _gui_box_layout->addWidget(gui_box_fps_label, 0, 0);
    _gui_fps_info = new QLabel("");
    _gui_box_layout->addWidget(_gui_fps_info, 0, 1, 1, 4);

    _gui_box_layout->addWidget(new QLabel("Near:"), 2, 0);
    _gui_box_layout->addWidget(new QLabel("Far:"), 3, 0);
    _gui_box_layout->addWidget(new QLabel("Quads culled:"), 4, 0);
    _gui_box_layout->addWidget(new QLabel("Quads rendered:"), 5, 0);
    _gui_box_layout->addWidget(new QLabel("Quads approximated:"), 6, 0);
    _gui_box_layout->addWidget(new QLabel("Lowest quad level:"), 7, 0);
    _gui_box_layout->addWidget(new QLabel("Highest quad level:"), 8, 0);
    for (int dp = 0; dp < 4; dp++) {
        _gui_box_layout->addWidget(new QLabel(str::asprintf("Depth pass %d  ", dp).c_str()), 1, dp + 1);
        _gui_near_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_near_info[dp], 2, dp + 1);
        _gui_far_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_far_info[dp], 3, dp + 1);
        _gui_qc_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_qc_info[dp], 4, dp + 1);
        _gui_qr_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_qr_info[dp], 5, dp + 1);
        _gui_qa_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_qa_info[dp], 6, dp + 1);
        _gui_lq_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_lq_info[dp], 7, dp + 1);
        _gui_hq_info[dp] = new QLabel("");
        _gui_box_layout->addWidget(_gui_hq_info[dp], 8, dp + 1);
    }
    _gui_box->setLayout(_gui_box_layout);
    layout->addWidget(_gui_box, layout_row++, 0);

#if HAVE_LIBEQUALIZER
    _eq_box = new QGroupBox("Equalizer");
    QGridLayout* _eq_box_layout = new QGridLayout;
    QLabel* _eq_box_fps_label = new QLabel("FPS:");
    _eq_box_layout->addWidget(_eq_box_fps_label, 0, 0);
    _eq_fps_info = new QLabel("");
    _eq_box_layout->addWidget(_eq_fps_info, 0, 1);
    _eq_box->setLayout(_eq_box_layout);
    layout->addWidget(_eq_box, layout_row++, 0);
#endif

    layout->setRowStretch(layout_row, 1);
    setLayout(layout);
    setModal(false);
    if (parent) {
        setWindowTitle(parent->windowTitle() + ": Statistics");
    }
}

Statistics::~Statistics()
{
}

void Statistics::update_gui(float fps, const class renderpass_info& info)
{
    if (this->isVisible()) {
        if (fps > 0.0f) {
            _gui_fps_info->setText(toQString(str::asprintf("%.2f", fps)));
            for (int dp = 0; dp < 4; dp++) {
                if (info.depth_passes > dp) {
                    _gui_near_info[dp]->setText(toQString(str::human_readable_length(info.frustum[dp].n())));
                    _gui_far_info[dp]->setText(toQString(str::human_readable_length(info.frustum[dp].f())));
                    _gui_qc_info[dp]->setText(toQString(str::from(info.quads_culled[dp])));
                    _gui_qr_info[dp]->setText(toQString(str::from(info.quads_rendered[dp])));
                    _gui_qa_info[dp]->setText(toQString(str::from(info.quads_approximated[dp])));
                    _gui_lq_info[dp]->setText(toQString(str::from(info.lowest_quad_level[dp])));
                    _gui_hq_info[dp]->setText(toQString(str::from(info.highest_quad_level[dp])));
                } else {
                    _gui_near_info[dp]->setText("");
                    _gui_far_info[dp]->setText("");
                    _gui_qc_info[dp]->setText("");
                    _gui_qr_info[dp]->setText("");
                    _gui_qa_info[dp]->setText("");
                    _gui_lq_info[dp]->setText("");
                    _gui_hq_info[dp]->setText("");
                }
            }
            _gui_box->setEnabled(true);
        } else {
            _gui_fps_info->setText("");
            for (int dp = 0; dp < 4; dp++) {
                _gui_near_info[dp]->setText("");
                _gui_far_info[dp]->setText("");
                _gui_qc_info[dp]->setText("");
                _gui_qr_info[dp]->setText("");
                _gui_qa_info[dp]->setText("");
                _gui_lq_info[dp]->setText("");
                _gui_hq_info[dp]->setText("");
            }
            _gui_box->setEnabled(false);
        }
    }
}

#if HAVE_LIBEQUALIZER
void Statistics::update_eq(float fps)
{
    if (this->isVisible()) {
        if (fps > 0.0f) {
            _eq_fps_info->setText(toQString(str::asprintf("%.2f", fps)));
            _eq_box->setEnabled(true);
        } else {
            _eq_fps_info->setText("");
            _eq_box->setEnabled(false);
        }
    }
}
#endif
