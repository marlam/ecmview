/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#include <sstream>

#include <QMainWindow>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QIcon>
#include <QGridLayout>
#include <QToolBox>
#include <QMenuBar>
#include <QMenu>
#include <QCloseEvent>
#include <QTimer>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QListView>
#include <QTreeView>
#include <QClipboard>
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QRadioButton>

#include "fio.h"
#include "str.h"
#include "msg.h"

#if HAVE_LIBEQUALIZER
# include "eqcontext.h"
#endif

#include "guitools.h"
#include "mainwindow.h"


MainWindow::MainWindow(int* argc, char** argv, state* master_state, QWidget* parent) :
    QMainWindow(parent), _argc(argc), _argv(argv), _master_state(master_state), _eqcontext(NULL)
{
    // Set application properties
    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(QIcon(":appicon.png"));
    _settings = new QSettings(QString(_master_state->conf_file.c_str()), QSettings::IniFormat);

    // Restore renderer parameters
    _settings->beginGroup("Session");
    {
        QString renderer_param_string = _settings->value("RendererParameters", "").toString();
        std::istringstream iss(renderer_param_string.toStdString());
        std::string name, value;
        s11n::load(iss, name, value);
        s11n::load(value, _master_state->renderer);
    }
    _settings->endGroup();

    // Restore window settings
    _settings->beginGroup("MainWindow");
    restoreGeometry(_settings->value("geometry").toByteArray());
    restoreState(_settings->value("windowstate").toByteArray());
    _settings->endGroup();

    // Create widgets
    _dbs = new DBs(_settings, _master_state, this);
    _light = new Light(_master_state->light, this);
    _lens = new Lens(_master_state->lens, this);
#ifndef NDEBUG
    _quaddebug = new QuadDebug(this);
#endif
    _info = new Info(_settings, this);
    _guicontext = new GUIContext(_master_state, this);
    _renderprops = NULL;
    _statistics = NULL;

    // Connect widgets
    connect(_guicontext, SIGNAL(update_light(class light_parameters)),
            _light, SLOT(set_light(class light_parameters)));
    connect(_guicontext, SIGNAL(update_viewer_info(double, double, double)),
            _light, SLOT(set_viewer_info(double, double, double)));
    connect(_light, SIGNAL(update_light(class light_parameters)),
            _guicontext, SLOT(set_light(class light_parameters)));
    connect(_guicontext, SIGNAL(update_lens(class lens_parameters)),
            _lens, SLOT(set_lens(class lens_parameters)));
    connect(_lens, SIGNAL(update_lens(class lens_parameters)),
            _guicontext, SLOT(set_lens(class lens_parameters)));
    connect(_guicontext, SIGNAL(update_approx_info(int)),
            _info, SLOT(update_approx_info(int)));
    connect(_guicontext, SIGNAL(update_viewer_info(double, double, double)),
            _info, SLOT(update_viewer_info(double, double, double)));
    connect(_guicontext, SIGNAL(update_pointer_info(bool, double, double, double)),
            _info, SLOT(update_pointer_info(bool, double, double, double)));
    connect(_info, SIGNAL(update_info_prefs(bool, bool)),
            _guicontext, SLOT(update_info_prefs(bool, bool)));
#ifndef NDEBUG
    connect(_guicontext, SIGNAL(update_quad_debug_range(int)), _quaddebug, SLOT(set_index_range(int)));
    connect(_guicontext, SIGNAL(update_quad_debug_info(int, int, int, int, float, float, float)),
            _quaddebug, SLOT(set_quad_info(int, int, int, int, float, float, float)));
    connect(_quaddebug, SIGNAL(change(int, bool)), _guicontext, SLOT(set_quad_debug(int, bool)));
#endif

    // Initialize widgets
    _info->init();

    // Create the central widget
    QGroupBox *light_box = new QGroupBox("Lighting");
    QGridLayout *light_layout = new QGridLayout();
    light_layout->addWidget(_light, 0, 0);
    light_box->setLayout(light_layout);
    QGroupBox *lens_box = new QGroupBox("Lens");
    QGridLayout *lens_layout = new QGridLayout();
    lens_layout->addWidget(_lens, 0, 0);
    lens_box->setLayout(lens_layout);
#ifndef NDEBUG
    QGroupBox *quaddebug_box = new QGroupBox("Quad Debug");
    QGridLayout *quaddebug_layout = new QGridLayout();
    quaddebug_layout->addWidget(_quaddebug, 0, 0);
    quaddebug_box->setLayout(quaddebug_layout);
#endif
    QGroupBox *datasets_box = new QGroupBox("Databases");
    QGridLayout *datasets_layout = new QGridLayout();
    datasets_layout->addWidget(_dbs, 0, 0);
    datasets_box->setLayout(datasets_layout);
    QGridLayout *layout = new QGridLayout();
    int row = 0;
    layout->addWidget(light_box, row++, 0);
    layout->addWidget(lens_box, row++, 0);
#ifndef NDEBUG
    layout->addWidget(quaddebug_box, row++, 0);
#endif
    layout->addWidget(datasets_box, row++, 0);
    layout->addWidget(_guicontext, 0, 1, row, 1);
    layout->addWidget(_info, row, 1);
    QWidget *widget = new QWidget;
    layout->setRowStretch(row - 1, 1);
    layout->setColumnStretch(1, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    /* Create menus */

    // File menu
    QMenu *file_menu = menuBar()->addMenu("&File");
    QAction *open_local_dataset_act = new QAction("&Open local data set...", this);
    connect(open_local_dataset_act, SIGNAL(triggered()), this, SLOT(open_local_dataset()));
    file_menu->addAction(open_local_dataset_act);
    QAction *open_remote_dataset_act = new QAction("Open remote data set...", this);
    connect(open_remote_dataset_act, SIGNAL(triggered()), this, SLOT(open_remote_dataset()));
    file_menu->addAction(open_remote_dataset_act);
    file_menu->addSeparator();
    QAction *open_state_act = new QAction("Load state...", this);
    open_state_act->setShortcut(QString("Ctrl+O"));
    connect(open_state_act, SIGNAL(triggered()), this, SLOT(open_state()));
    file_menu->addAction(open_state_act);
    QAction *save_state_as_act = new QAction("Save state as...", this);
    connect(save_state_as_act, SIGNAL(triggered()), this, SLOT(save_state_as()));
    file_menu->addAction(save_state_as_act);
    QAction *save_state_act = new QAction("Save state", this);
    save_state_act->setShortcut(QString("Ctrl+S"));
    connect(save_state_act, SIGNAL(triggered()), this, SLOT(save_state()));
    file_menu->addAction(save_state_act);
    file_menu->addSeparator();
    QAction *quit_act = new QAction("&Quit...", this);
    quit_act->setShortcut(QString("Ctrl+Q"));
    connect(quit_act, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_act);
    // View menu
    QMenu *view_menu = menuBar()->addMenu("&View");
    QAction *fullscreen_act = new QAction("Fullscreen...", this);
    fullscreen_act->setShortcut(QString("Ctrl+F"));
    connect(fullscreen_act, SIGNAL(triggered()), this, SLOT(fullscreen()));
    view_menu->addAction(fullscreen_act);
    view_menu->addSeparator();
    QAction *copy_current_view_act = new QAction("Copy current view", this);
    copy_current_view_act->setShortcut(QString("Ctrl+C"));
    connect(copy_current_view_act, SIGNAL(triggered()), this, SLOT(copy_current_view()));
    view_menu->addAction(copy_current_view_act);
    QAction *save_current_view_act = new QAction("Save current view...", this);
    connect(save_current_view_act, SIGNAL(triggered()), this, SLOT(save_current_view()));
    view_menu->addAction(save_current_view_act);
    QAction *save_view_act = new QAction("Save view...", this);
    connect(save_view_act, SIGNAL(triggered()), this, SLOT(save_view()));
    view_menu->addAction(save_view_act);
    // Dialog menu
    QMenu *dialog_menu = menuBar()->addMenu("&Dialog");
    QAction *renderprops_act = new QAction("Toggle O&ptions", this);
    renderprops_act->setCheckable(false);
    renderprops_act->setShortcut(QString("Ctrl+P"));
    connect(renderprops_act, SIGNAL(triggered()), this, SLOT(toggle_renderprops()));
    dialog_menu->addAction(renderprops_act);
    QAction *statistics_act = new QAction("Toggle S&tatistics", this);
    statistics_act->setCheckable(false);
    statistics_act->setShortcut(QString("Ctrl+T"));
    connect(statistics_act, SIGNAL(triggered()), this, SLOT(toggle_statistics()));
    dialog_menu->addAction(statistics_act);
#if HAVE_LIBEQUALIZER
    QAction *equalizer_act = new QAction(tr("Toggle &Equalizer"), this);
    equalizer_act->setCheckable(false);
    equalizer_act->setShortcut(tr("Ctrl+E"));
    connect(equalizer_act, SIGNAL(triggered()), this, SLOT(toggle_equalizer()));
    dialog_menu->addAction(equalizer_act);
#endif
    // Help menu
    QMenu *help_menu = menuBar()->addMenu("&Help");
    QAction *show_aboutbox_act = new QAction("&About", this);
    show_aboutbox_act->setShortcut(QString("Ctrl+A"));
    connect(show_aboutbox_act, SIGNAL(triggered()), this, SLOT(show_aboutbox()));
    help_menu->addAction(show_aboutbox_act);

    // Try the GUI context to see if it works
    _guicontext->render();

    if (_guicontext->works()) {
        QTimer *renderloop_timer = new QTimer(this);
        connect(renderloop_timer, SIGNAL(timeout()), this, SLOT(renderloop()));
        renderloop_timer->start(0);
    }

    show();
}

MainWindow::~MainWindow()
{
    delete _settings;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Quit Equalizer
#if HAVE_LIBEQUALIZER
    if (_eqcontext) {
        _eqcontext->quit();
        delete _eqcontext;
        _eqcontext = NULL;
    }
#endif

    // Close Data Set Dialogs
    _dbs->close_dialogs();

    // Save renderer parameters
    _settings->beginGroup("Session");
    {
        std::ostringstream oss;
        s11n::save(oss, "renderer-parameters", _master_state->renderer);
        _settings->setValue("RendererParameters", QString(oss.str().c_str()));
    }
    _settings->endGroup();

    // Save window settings
    _settings->beginGroup("MainWindow");
    _settings->setValue("geometry", saveGeometry());
    _settings->setValue("windowState", saveState());
    _settings->endGroup();

    event->accept();
}

/* Special actions */

void MainWindow::renderloop()
{
    // GUI: render and update statistics
    _guicontext->render();
#if HAVE_LIBEQUALIZER
    // Equalizer: render and update statistics
    if (_eqcontext && _eqcontext->is_running()) {
        _eqcontext->render();
        if (_statistics) {
            _statistics->update_eq(_eqcontext->fps());
        }
    } else {
        if (_eqcontext) {
            _eqcontext->quit();
            delete _eqcontext;
            _eqcontext = NULL;
        }
        if (_statistics) {
            _statistics->update_eq(-1.0);
        }
    }
#endif
}

/* Menu actions */

void MainWindow::open_local_dataset()
{
    QFileDialog *file_dialog = new QFileDialog(this);
    _settings->beginGroup("Session");
    file_dialog->setDirectory(_settings->value("last-open-dir", QDir::homePath()).toString());
    _settings->endGroup();
    file_dialog->setWindowTitle("Open local data set");
    file_dialog->setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog->setFileMode(QFileDialog::DirectoryOnly);
    if (!file_dialog->exec())
        return;
    QStringList dir_names = file_dialog->selectedFiles();
    if (dir_names.empty())
        return;
    _settings->beginGroup("Session");
    _settings->setValue("last-open-dir", file_dialog->directory().path());
    _settings->endGroup();
    std::string d = fio::from_sys(dir_names[0].toStdString());
    if (d[d.length() - 1] != '/') {
        d.append("/");
    }
    _dbs->open(std::string("file://") + d, "", "");
}

void MainWindow::open_remote_dataset()
{
    QDialog *url_dialog = new QDialog(this);
    url_dialog->setWindowTitle("Open remote data set");
    QLabel *url_label = new QLabel("URL:");
    QLineEdit *url_edit = new QLineEdit("");

    QGroupBox *auth_box = new QGroupBox("Authentication");
    auth_box->setCheckable(true);
    auth_box->setChecked(false);
    QLabel *username_label = new QLabel("Username:");
    connect(auth_box, SIGNAL(toggled(bool)), username_label, SLOT(setEnabled(bool)));
    QLineEdit *username_edit = new QLineEdit("");
    connect(auth_box, SIGNAL(toggled(bool)), username_edit, SLOT(setEnabled(bool)));
    connect(auth_box, SIGNAL(toggled(bool)), username_edit, SLOT(clear()));
    QLabel *password_label = new QLabel("Password:");
    connect(auth_box, SIGNAL(toggled(bool)), password_label, SLOT(setEnabled(bool)));
    QLineEdit *password_edit = new QLineEdit("");
    connect(auth_box, SIGNAL(toggled(bool)), password_edit, SLOT(setEnabled(bool)));
    connect(auth_box, SIGNAL(toggled(bool)), password_edit, SLOT(clear()));
    password_edit->setEchoMode(QLineEdit::Password);
    QGridLayout *box_layout = new QGridLayout();
    box_layout->addWidget(username_label, 0, 0);
    box_layout->addWidget(username_edit, 0, 1, 1, 3);
    box_layout->addWidget(password_label, 1, 0);
    box_layout->addWidget(password_edit, 1, 1, 1, 3);
    box_layout->setColumnStretch(1, 1);
    auth_box->setLayout(box_layout);

    QPushButton *ok_btn = new QPushButton("OK");
    QPushButton *cancel_btn = new QPushButton("Cancel");
    connect(ok_btn, SIGNAL(pressed()), url_dialog, SLOT(accept()));
    connect(cancel_btn, SIGNAL(pressed()), url_dialog, SLOT(reject()));
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(url_label, 0, 0);
    layout->addWidget(url_edit, 0, 1, 1, 3);
    layout->addWidget(auth_box, 1, 0, 1, 4);
    layout->addWidget(ok_btn, 2, 2);
    layout->addWidget(cancel_btn, 2, 3);
    layout->setColumnStretch(1, 1);
    url_edit->setMinimumWidth(400);
    url_dialog->setLayout(layout);
    url_dialog->exec();
    if (url_dialog->result() == QDialog::Accepted && !url_edit->text().isEmpty())
    {
        QString url = url_edit->text();
        if (!url.endsWith('/'))
            url.append('/');
        _dbs->open(url.toStdString(), username_edit->text().toStdString(), password_edit->text().toStdString());
    }
}

void MainWindow::save_state_as()
{
    QFileDialog *file_dialog = new QFileDialog(this);
    _settings->beginGroup("Session");
    file_dialog->setDirectory(_settings->value("last-save-state-dir", QDir::homePath()).toString());
    _settings->endGroup();
    file_dialog->setWindowTitle("Save state");
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::AnyFile);
    file_dialog->setDefaultSuffix(PACKAGE_TARNAME "-state");
    QStringList filters;
    filters << QString(PACKAGE_NAME " state (*." PACKAGE_TARNAME "-state)") << QString("All files (*)");
    file_dialog->setFilters(filters);
    if (!file_dialog->exec() || file_dialog->selectedFiles().empty())
        return;
    QString file_name = file_dialog->selectedFiles().at(0);
    _settings->beginGroup("Session");
    _settings->setValue("last-save-state-dir", file_dialog->directory().path());
    _settings->endGroup();
    _state_file_name = file_name.toStdString();
    save_state();
}

void MainWindow::save_state()
{
    if (_state_file_name.empty()) {
        save_state_as();
        return;
    }

    try {
        _master_state->save_statefile(_state_file_name);
    }
    catch (exc& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::open_state()
{
    QFileDialog* file_dialog = new QFileDialog(this);
    _settings->beginGroup("Session");
    file_dialog->setDirectory(_settings->value("last-load-state-dir", QDir::homePath()).toString());
    _settings->endGroup();
    file_dialog->setWindowTitle("Load state");
    file_dialog->setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog->setFileMode(QFileDialog::ExistingFile);
    QStringList filters;
    filters << QString(PACKAGE_NAME " state (*." PACKAGE_TARNAME "-state)") << QString("All files (*)");
    file_dialog->setFilters(filters);
    if (!file_dialog->exec() || file_dialog->selectedFiles().empty())
        return;
    QString file_name = file_dialog->selectedFiles().at(0);
    _settings->beginGroup("Session");
    _settings->setValue("last-load-state-dir", file_dialog->directory().path());
    _settings->endGroup();
    try {
        _master_state->load_statefile(file_name.toStdString());
    }
    catch (exc &e) {
        QMessageBox::critical(this, "Error", e.what());
        return;
    }
    _state_file_name = file_name.toStdString();
    _dbs->update();
}

void MainWindow::fullscreen()
{
    if (_guicontext->fullscreen()) {
        _guicontext->exit_fullscreen();
        return;
    }

    int n = QApplication::desktop()->screenCount();
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Fullscreen/Multiscreen Settings");
    QLabel *lbl = new QLabel("Configure fullscreen mode:");
    QRadioButton *single_btn = new QRadioButton("Single screen:");
    QComboBox *single_box = new QComboBox();
    single_box->addItem("Primary screen");
    if (n > 1) {
        for (int i = 0; i < n; i++) {
            single_box->addItem(str::asprintf("Screen %d", i + 1).c_str());
        }
    }
    QRadioButton *dual_btn = new QRadioButton("Dual screen:");
    QComboBox *dual_box0 = new QComboBox();
    QComboBox *dual_box1 = new QComboBox();
    if (n > 1) {
        for (int i = 0; i < n; i++) {
            dual_box0->addItem(str::asprintf("Screen %d", i + 1).c_str());
            dual_box1->addItem(str::asprintf("Screen %d", i + 1).c_str());
        }
    }
    QRadioButton *multi_btn = new QRadioButton("Multi screen:");
    QLineEdit *multi_edt = new QLineEdit();
    QRegExp rx("\\d{1,2}(,\\d{1,2}){0,15}");
    multi_edt->setValidator(new QRegExpValidator(rx, 0));
    QPushButton *cancel_btn = new QPushButton("Cancel");
    QPushButton *ok_btn = new QPushButton("OK");
    ok_btn->setDefault(true);
    connect(cancel_btn, SIGNAL(pressed()), dlg, SLOT(reject()));
    connect(ok_btn, SIGNAL(pressed()), dlg, SLOT(accept()));
    QGridLayout *layout0 = new QGridLayout();
    layout0->addWidget(lbl, 0, 0, 1, 3);
    layout0->addWidget(single_btn, 1, 0);
    layout0->addWidget(single_box, 1, 1, 1, 2);
    layout0->addWidget(dual_btn, 2, 0);
    layout0->addWidget(dual_box0, 2, 1);
    layout0->addWidget(dual_box1, 2, 2);
    layout0->addWidget(multi_btn, 3, 0);
    layout0->addWidget(multi_edt, 3, 1, 1, 2);
    QGridLayout *layout1 = new QGridLayout();
    layout1->addWidget(cancel_btn, 0, 0);
    layout1->addWidget(ok_btn, 0, 1);
    QGridLayout *layout = new QGridLayout();
    layout->addLayout(layout0, 0, 0);
    layout->addLayout(layout1, 1, 0);
    dlg->setLayout(layout);

    // Set initial values
    if (n < 3) {
        multi_btn->setEnabled(false);
        multi_edt->setEnabled(false);
    } else {
        multi_edt->setText("1,2,3");
    }
    if (n < 2) {
        dual_btn->setEnabled(false);
        dual_box0->setEnabled(false);
        dual_box1->setEnabled(false);
    } else {
        dual_box0->setCurrentIndex(0);
        dual_box1->setCurrentIndex(1);
    }
    int fullscreen_screens = _settings->value("Session/fullscreen-screens", "0").toInt();
    std::vector<int> conf_screens;
    for (int i = 0; i < 16; i++) {
        if (fullscreen_screens & (1 << i)) {
            conf_screens.push_back(i);
        }
    }
    if (conf_screens.size() >= 3 && n >= 3) {
        QString screen_list;
        for (size_t i = 0; i < conf_screens.size(); i++) {
            screen_list += str::from(conf_screens[i] + 1).c_str();
            if (i < conf_screens.size() - 1) {
                screen_list += ',';
            }
        }
        multi_btn->setChecked(true);
        multi_edt->setText(screen_list);
    } else if (conf_screens.size() == 2 && n >= 2) {
        dual_box0->setCurrentIndex(conf_screens[0]);
        dual_box1->setCurrentIndex(conf_screens[1]);
        dual_btn->setChecked(true);
    } else {
        if (conf_screens.size() > 0 && conf_screens[0] < n) {
            single_box->setCurrentIndex(conf_screens[0] + 1);
        } else {
            single_box->setCurrentIndex(0);
        }
        single_btn->setChecked(true);
    }

    // Run dialog
    dlg->exec();
    if (dlg->result() == QDialog::Accepted) {
        if (single_btn->isChecked()) {
            if (single_box->currentIndex() == 0) {
                fullscreen_screens = 0;
            } else {
                fullscreen_screens = (1 << (single_box->currentIndex() - 1));
            }
        } else if (dual_btn->isChecked()) {
            fullscreen_screens = (1 << dual_box0->currentIndex());
            fullscreen_screens |= (1 << dual_box1->currentIndex());
        } else {
            fullscreen_screens = 0;
            QStringList screens = multi_edt->text().split(',', QString::SkipEmptyParts);
            for (int i = 0; i < screens.size(); i++) {
                int s = str::to<int>(screens[i].toAscii().data());
                if (s >= 1 && s <= 16) {
                    fullscreen_screens |= (1 << (s - 1));
                }
            }
        }
        _settings->setValue("Session/fullscreen-screens", fullscreen_screens);
        _guicontext->enter_fullscreen(fullscreen_screens);
    }
}

void MainWindow::copy_current_view()
{
    QImage *img = _guicontext->get_current_image();
    QApplication::clipboard()->setImage(*img);
    delete img;
}

void MainWindow::save_image(QImage *img)
{
    QFileDialog *file_dialog = new QFileDialog(this);
    _settings->beginGroup("Session");
    file_dialog->setDirectory(_settings->value("last-save-image-dir", QDir::homePath()).toString());
    _settings->endGroup();
    file_dialog->setWindowTitle(tr("Save image"));
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::AnyFile);
    file_dialog->setDefaultSuffix("png");
    QStringList filters;
    filters << tr("PNG images (*.png)") << tr("All files (*)");
    file_dialog->setFilters(filters);
    if (!file_dialog->exec() || file_dialog->selectedFiles().empty())
        return;

    QString file_name = file_dialog->selectedFiles().at(0);
    _settings->beginGroup("Session");
    _settings->setValue("last-save-image-dir", file_dialog->directory().path());
    _settings->endGroup();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool success = img->save(file_name, "png");
    QApplication::restoreOverrideCursor();
    if (!success)
        QMessageBox::critical(this, tr("Error"), QString(tr("Saving %1 failed.")).arg(file_name));
}

void MainWindow::save_current_view()
{
    QImage *img = _guicontext->get_current_image();
    if (!img->isNull())
        save_image(img);
    delete img;
}

void MainWindow::save_view()
{
    WidthHeightDialog *wh_dialog = new WidthHeightDialog(_guicontext->width(), _guicontext->height(), this);
    int w, h;
    if (!wh_dialog->run(w, h))
        return;
    QImage *img = _guicontext->render_to_image(w, h);
    if (!img->isNull())
        save_image(img);
    delete img;
}

void MainWindow::toggle_renderprops()
{
    if (_renderprops && _renderprops->isVisible()) {
        _renderprops->close();
        delete _renderprops;
        _renderprops = NULL;
    } else {
        if (_renderprops) {
            _renderprops->close();
            delete _renderprops;
        }
        _renderprops = new RenderProps(_master_state->renderer, this);
        connect(_renderprops, SIGNAL(update_renderer_parameters(const class renderer_parameters)),
                _guicontext, SLOT(update_renderer_parameters(const class renderer_parameters)));
        _renderprops->show();
        _renderprops->raise();
        _renderprops->activateWindow();
    }
}

void MainWindow::toggle_statistics()
{
    if (_statistics && _statistics->isVisible()) {
        _statistics->close();
        delete _statistics;
        _statistics = NULL;
    } else {
        if (_statistics) {
            _statistics->close();
            delete _statistics;
        }
        _statistics = new Statistics(this);
        connect(_guicontext, SIGNAL(update_statistics(float, const renderpass_info&)),
                    _statistics, SLOT(update_gui(float, const renderpass_info&)));
        _statistics->show();
        _statistics->raise();
        _statistics->activateWindow();
    }
}

void MainWindow::toggle_equalizer()
{
#if HAVE_LIBEQUALIZER
    if (_eqcontext) {
        _eqcontext->quit();
        delete _eqcontext;
        _eqcontext = NULL;
    } else {
        try {
            _eqcontext = new eq_context(_argc, _argv, _master_state);
        }
        catch (std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
#endif
}

void MainWindow::show_aboutbox()
{
    QMessageBox::about(this, tr("About " PACKAGE_NAME), tr(
                "<p>%1 version %2</p>"
                "<p>Copyright (C) 2012 <a href=\"http://www.cg.informatik.uni-siegen.de/\">"
                "Computer Graphics Group, University of Siegen</a>.<br>"
                "Written by <a href=\"mailto:martin.lambers@uni-siegen.de\">Martin Lambers</a>."
                "</p><p>"
                "This is free software. You may redistribute copies of it "
                "under the terms of the <a href=\"http://www.gnu.org/licenses/gpl.html\">"
                "GNU General Public License</a>. "
                "There is NO WARRANTY, to the extent permitted by law.</p>"
                "<p>See <a href=\"%3\">%3</a> for more information on this software.</p>"
                ).arg(PACKAGE_NAME).arg(PACKAGE_VERSION).arg(PACKAGE_URL));
}
