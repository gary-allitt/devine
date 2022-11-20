#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QActionGroup>
#include <QTimer>
#include "glue.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTreeWidgetItem;
class NativeEvents;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSettings the_settings;
    QString the_filter_was;
    QTimer the_redraw_timer;
    QTimer the_filter_debounce_timer;

    NativeEvents* the_native_events;
    QActionGroup the_show_by_group;

    QAction* the_disable;
    QAction* the_enable;
    QAction* the_uninstall;
    QAction* the_properties;

    QStringList the_extended_columns;

    Glue g;


    void EnumDevices();
    void LoadView();
    void LoadByTypeView();
    void LoadByConnectionView();
    void PopulateLeaf(QTreeWidgetItem* parent);
    void FetchColumnSettings();

    struct device
    {
      QString description;
      GUID class_guid;
      QString class_guid_readable;
      QString first_hardware_id;
      QString instance_id;
      QString instance_id_display;
      QString manufacturer;
      QString provider;
      QString _class;
      QString icon;
      QString parent;
      bool connected;
      bool has_children;
      bool enabled;
      bool problem;
    };

    vector<device> the_devices;
    map<QString, GUID> the_used_device_class_guids;

private slots: 
    void OnItemExpanded(QTreeWidgetItem*);
    void OnDeviceChange();
    void OnShowHidden(bool show);
    void OnAbout();
    void OnFilter();
    void OnShowBy(); 
    void OnSelectColumns();
    void OnRightClick(const QPoint& pos);
    void OnDisable();
    void OnEnable();
    void OnUninstall();
    void OnProperties(); 
    void OnScanHardware(); 
};
#endif // MAINWINDOW_H
