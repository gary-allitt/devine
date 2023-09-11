#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QActionGroup>
#include <QTimer>
#include "glue.h"
#include "device.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define SKIPEMPTYPARTS Qt::SkipEmptyParts
#else
#define SKIPEMPTYPARTS QString::SkipEmptyParts
#endif

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

    QStringList the_extended_columns;
    bool the_import_mode;

    Glue g;


    void LoadView();
    void LoadByTypeView();
    void LoadByConnectionView();
    void PopulateLeaf(QTreeWidgetItem* parent);
    void FetchColumnSettings();
    bool ItemMatchesFilter(QTreeWidgetItem* item);
    bool DoImportActual(const QString& a_file_name);

    vector<QStringList> the_filter_or_s;


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
    void OnSetupMenu();
    void OnExport();
    void OnImport();
};
#endif // MAINWINDOW_H
