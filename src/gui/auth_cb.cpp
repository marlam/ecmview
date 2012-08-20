/*
 * Copyright (C) 2012
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

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

#include "auth_cb.h"

qt_auth_callback::qt_auth_callback() : auth_callback()
{
}

void qt_auth_callback::get(const std::string& url, std::string& username, std::string& password)
{
    QDialog* dlg = new QDialog();
    dlg->setWindowTitle("Enter username and password");
    QLabel* url_label = new QLabel(QString("URL: ") + QString::fromStdString(url));
    QLabel* username_label = new QLabel("Username:");
    QLineEdit* username_edit = new QLineEdit("");
    QLabel* password_label = new QLabel("Password:");
    QLineEdit* password_edit = new QLineEdit("");
    password_edit->setEchoMode(QLineEdit::Password);
    QPushButton* ok_btn = new QPushButton("OK");
    QPushButton* cancel_btn = new QPushButton("Cancel");
    QObject::connect(ok_btn, SIGNAL(pressed()), dlg, SLOT(accept()));
    QObject::connect(cancel_btn, SIGNAL(pressed()), dlg, SLOT(reject()));
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(url_label, 0, 0, 1, 4);
    layout->addWidget(username_label, 1, 0);
    layout->addWidget(username_edit, 1, 1, 1, 3);
    layout->addWidget(password_label, 2, 0);
    layout->addWidget(password_edit, 2, 1, 1, 3);
    layout->addWidget(ok_btn, 3, 2);
    layout->addWidget(cancel_btn, 3, 3);
    layout->setColumnStretch(1, 1);
    dlg->setLayout(layout);
    dlg->exec();
    if (dlg->result() == QDialog::Accepted && !username_edit->text().isEmpty() && !password_edit->text().isEmpty()) {
        username = username_edit->text().toStdString();
        password = password_edit->text().toStdString();
    }
}
