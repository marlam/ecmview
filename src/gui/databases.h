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

#ifndef DATABASES_H
#define DATABASES_H

#include "config.h"

#include <string>
#include <vector>
#include <map>

#include <QWidget>
#include <QDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QTreeView>

#include "state.h"


class QSettings;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QStackedWidget;
class QTabWidget;


class DBInfo : public QWidget
{
Q_OBJECT

public:
    DBInfo(const class database_description& dd, QWidget* parent = NULL);
};

class DBProcessingParameters : public QWidget
{
Q_OBJECT

private:
    QSettings* _settings;
    bool _lock;
    state *_master_state;
    const uuid _db_uuid;
    int _p_index;

    /* Common */
    QCheckBox* _active_checkbox;
    QSpinBox* _priority_spinbox;
    QDoubleSpinBox* _weight_spinbox;
    QPushButton* _reset_button;             // only for lens
    /* ecmdb::category_elevation */
    QDoubleSpinBox* _elevation_scale_factor_spinbox;
    QSlider* _elevation_scale_factor_slider;
    QDoubleSpinBox* _elevation_scale_center_spinbox;
    QSlider* _elevation_scale_center_slider;
    /* ecmdb::category_texture */
    QDoubleSpinBox* _texture_contrast_spinbox;
    QSlider* _texture_contrast_slider;
    QDoubleSpinBox* _texture_brightness_spinbox;
    QSlider* _texture_brightness_slider;
    QDoubleSpinBox* _texture_saturation_spinbox;
    QSlider* _texture_saturation_slider;
    QDoubleSpinBox* _texture_hue_spinbox;
    QSlider* _texture_hue_slider;
    /* ecmdb::category_data */
    QDoubleSpinBox* _data_offset_spinbox;
    QSlider* _data_offset_slider;
    QDoubleSpinBox* _data_factor_spinbox;
    QSlider* _data_factor_slider;
    QLabel* _data_gradient_label;
    /* processing_parameters::category_e2c */
    QLabel* _e2c_gradient_label;
    QCheckBox* _e2c_adapt_brightness_checkbox;
    QDoubleSpinBox* _e2c_isolines_distance_spinbox;
    QDoubleSpinBox* _e2c_isolines_thickness_spinbox;
    QLabel* _e2c_isolines_color_label;

    QPixmap pixmap_from_gradient(int gradient_length, const uint8_t* gradient);
    void choose_gradient(int& gradient_length, uint8_t* gradient);

private slots:
    /* Common */
    void set_active(bool x);
    void set_priority(int x);
    void set_weight(double x);
    /* ecmdb::category_elevation */
    void set_elevation_scale_factor(double x);
    void slide_elevation_scale_factor(int x);
    void set_elevation_scale_center(double x);
    void slide_elevation_scale_center(int x);
    /* ecmdb::category_texture */
    void set_texture_contrast(double x);
    void slide_texture_contrast(int x);
    void set_texture_brightness(double x);
    void slide_texture_brightness(int x);
    void set_texture_saturation(double x);
    void slide_texture_saturation(int x);
    void set_texture_hue(double x);
    void slide_texture_hue(int x);
    /* ecmdb::category_data */
    void set_data_offset(double x);
    void slide_data_offset(int x);
    void set_data_factor(double x);
    void slide_data_factor(int x);
    void set_data_gradient();
    /* processing_parameters::category_e2c */
    void set_e2c_gradient();
    void set_e2c_adapt_brightness();
    void set_e2c_isolines_distance(double x);
    void set_e2c_isolines_thickness(double x);
    void set_e2c_isolines_color();

public:
    DBProcessingParameters(QSettings* settings, bool lens, state* master_state, const uuid& db_uuid, QWidget* parent = NULL);

    QPushButton* reset_button()
    {
        return _reset_button;
    }

    void update();
};

class DBDialog : public QDialog
{
Q_OBJECT

private:
    state *_master_state;
    const uuid _db_uuid;
    DBInfo* _db_info;
    DBProcessingParameters* _global_processing_parameters;
    DBProcessingParameters* _lens_processing_parameters;
    QTabWidget* _tab;

private slots:
    void reset_lens_processing_parameters();

public:
    DBDialog(QSettings* settings, state* master_state, const uuid& db_uuid, QWidget* parent = NULL);
};

class ItemDisplayInfo
{
public:
    state *master_state;
    uuid db_uuid;
    QString display_text;

    ItemDisplayInfo();
    ItemDisplayInfo(const ItemDisplayInfo& idi);
    ItemDisplayInfo(state *master_state, const uuid& db_uuid, const QString& display_text);

    void fromString(const QString& s);
    QString toString() const;
};

class DBItem : public QStandardItem
{
public:
    state *master_state;
    const uuid db_uuid;
    DBDialog *db_dialog;
    QString display_text;

    DBItem(QSettings* settings, state *master_state, const uuid& db_uuid, const QString& display_text);
    ~DBItem();
    QVariant data(int role) const;
};

class ItemDelegate : public QItemDelegate
{
Q_OBJECT
private:
    ItemDisplayInfo _display_info;
public:
    ItemDelegate(QObject* parent = 0);
    void drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const;
};

class DBModel : public QStandardItemModel
{
Q_OBJECT
private:
    QSettings* _settings;
    class state* _master_state;
    DBItem* _elevation_category_item;
    DBItem* _texture_category_item;
public:
    DBModel(QSettings* settings, class state* _master_state, QObject* parent = NULL);
    void init();
    void clear();
    void add_db(const uuid& db_uuid);
    void remove_db(const uuid db_uuid);
};

class DBView : public QTreeView
{
Q_OBJECT
private:
    ItemDelegate* _item_delegate;
public:
    DBView(QWidget* parent = NULL);
    ~DBView();
};

class DBs : public QWidget
{
Q_OBJECT

private:
    class state* _master_state;
    DBModel* _dbmodel;
    DBView* _dbview;

private slots:
    void item_doubleclicked(const QModelIndex& index);
    void item_rightclicked(const QModelIndex& index);

public:
    DBs(QSettings* settings, class state* master_state, QWidget* parent = NULL);

    void open(const std::string& url, const std::string& username, const std::string& password);
    void close(const uuid& uuid);

    void update();

    void close_dialogs();
};

#endif
