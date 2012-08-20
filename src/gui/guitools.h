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

#ifndef GUITOOLS_H
#define GUITOOLS_H

#include "config.h"

#include <QString>
#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>


inline QString toQString(const std::string& s)
{
    return QString::fromStdString(s);
}

class MaskSizeSpinBox : public QSpinBox
{
Q_OBJECT

public:
    MaskSizeSpinBox(QWidget* parent = NULL) : QSpinBox(parent)
    {
        setRange(1, 19);
        setSingleStep(2);
    }

    QValidator::State validate(QString& input, int &) const
    {
        int number;
        bool number_valid;
        number = input.toInt(&number_valid);
        if (number_valid && number >= minimum() && number <= maximum() && number % 2 == 1)
            return QValidator::Acceptable;
        else
            return QValidator::Invalid;
    }
};

class WidthHeightDialog : public QDialog
{
Q_OBJECT

private:
    QSpinBox *_w_spinbox;
    QSpinBox *_h_spinbox;
    QCheckBox *_ar_checkbox;
    double _aspect_ratio;
    bool _lock;

private slots:
    void w_changed();
    void h_changed();
    void ar_changed();

public:
    WidthHeightDialog(int w, int h, QWidget *parent = NULL);
    ~WidthHeightDialog();

    bool run(int &w, int &h);
};

#endif
