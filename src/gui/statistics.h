/*
 * Copyright (C) 2008, 2009, 2010, 2012
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

#ifndef STATISTICS_H
#define STATISTICS_H

#include "config.h"

#include <QDialog>

#include "renderer.h"

class QLabel;
class QGroupBox;

class Statistics : public QDialog
{
Q_OBJECT

private:
    QGroupBox* _gui_box;
    QLabel* _gui_fps_info;
    QLabel* _gui_near_info[4];
    QLabel* _gui_far_info[4];
    QLabel* _gui_qc_info[4];
    QLabel* _gui_qr_info[4];
    QLabel* _gui_qa_info[4];
    QLabel* _gui_lq_info[4];
    QLabel* _gui_hq_info[4];
    QLabel* _gui_bt_info[4];
    QLabel* _gui_rt_info[4];
#if HAVE_LIBEQUALIZER
    QGroupBox* _eq_box;
    QLabel* _eq_fps_info;
#endif

public:
    Statistics(QWidget* parent = NULL);
    ~Statistics();

public slots:
    void update_gui(float fps, const renderpass_info& info);
#if HAVE_LIBEQUALIZER
    void update_eq(float fps);
#endif
};

#endif
