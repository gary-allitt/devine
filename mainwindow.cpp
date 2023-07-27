#include <windows.h>
#include <QString.h>
#include <QFileDialog>
#include <QMessageBox>

#include <map>
#include <set>
using namespace std;

#include <cfgmgr32.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "deviceinfoset.h"
#include "nativeevents.h"
#include "about.h"
#include "columns.h"

#include <initguid.h>
#include <devpkey.h>

static const int ROLE_INSTANCE_ID = Qt::UserRole + 0;
static const int ROLE_CLASS_GUID = Qt::UserRole + 1;
static const int ROLE_ENABLED = Qt::UserRole + 2;
static const int ROLE_IS_DEVICE = Qt::UserRole + 3;

// todo 
// only hidden view 
// review https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/hiding-devices-from-device-manager
// remember searches
// progress bar when deleting 
// warn if reboot needed after delete


MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), 
  the_settings("gary-allitt","devine"),
  the_show_by_group(this),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  the_native_events = new NativeEvents;
  qApp->installNativeEventFilter(the_native_events);
  the_show_by_group.addAction(ui->actionDevices_by_type);
  the_show_by_group.addAction(ui->actionDevices_by_connection);
  the_show_by_group.setExclusive(true);
  the_import_mode = false;

  resize(the_settings.value("mainwindow/width",1024).toInt(), the_settings.value("mainwindow/height", 800).toInt());

  QString show_by = the_settings.value("options/show_by", QString("type")).toString();
  if (show_by == "type")
  {
    ui->actionDevices_by_type->setChecked(true);
  }
  else if (show_by == "connection")
  {
    ui->actionDevices_by_connection->setChecked(true);
  }
  the_redraw_timer.setInterval(3000); // added / deleted entries show green / red for 3 seconds 
  the_redraw_timer.setSingleShot(true);
  the_filter_debounce_timer.setInterval(750);
  the_filter_debounce_timer.setSingleShot(true);
  
  ui->actionShow_hidden_devices->setChecked(the_settings.value("options/show_hidden").toBool());

  connect(ui->the_disable, &QAction::triggered, this, &MainWindow::OnDisable);
  connect(ui->the_enable, &QAction::triggered, this, &MainWindow::OnEnable);
  connect(ui->the_uninstall, &QAction::triggered, this, &MainWindow::OnUninstall);
  connect(ui->the_properties, &QAction::triggered, this, &MainWindow::OnProperties);

  connect(ui->the_tree, &QTreeWidget::itemExpanded, this, &MainWindow::OnItemExpanded);
  connect(the_native_events, &NativeEvents::sigDeviceChange, this, &MainWindow::OnDeviceChange);
  connect(ui->actionExport, &QAction::triggered, this, &MainWindow::OnExport);
  connect(ui->actionImport, &QAction::triggered, this, &MainWindow::OnImport);

  connect(ui->actionE_xit, &QAction::triggered, qApp, &QGuiApplication::quit);
  connect(ui->action_Scan_for_hardware_changes, &QAction::triggered, this, &MainWindow::OnScanHardware);
  connect(ui->actionShow_hidden_devices, &QAction::triggered, this, &MainWindow::OnShowHidden);
  connect(ui->action_About, &QAction::triggered, this, &MainWindow::OnAbout);
  connect(ui->the_filter, &QLineEdit::textEdited, [=]() { the_filter_debounce_timer.start(); });
  connect(&the_filter_debounce_timer, &QTimer::timeout, this, &MainWindow::OnFilter);

  connect(ui->actionDevices_by_type, &QAction::triggered, this, &MainWindow::OnShowBy);
  connect(ui->actionDevices_by_connection, &QAction::triggered, this, &MainWindow::OnShowBy);
  connect(ui->actionSelect_columns, &QAction::triggered, this, &MainWindow::OnSelectColumns);
  connect(ui->the_tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::OnRightClick);
  connect(ui->the_tree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::OnSetupMenu);
  connect(&the_redraw_timer, &QTimer::timeout, this, &MainWindow::OnDeviceChange);

  device::the_devices.reserve(4096);

  OnSetupMenu();

  FetchColumnSettings();
  OnDeviceChange();

}

MainWindow::~MainWindow()
{
  the_settings.setValue("mainwindow/width", width());
  the_settings.setValue("mainwindow/height", height());
  delete ui;
}

void MainWindow::OnShowBy()
{
  if (sender() == ui->actionDevices_by_type)
  {
    the_settings.setValue("options/show_by", "type");
  }
  else if (sender() == ui->actionDevices_by_connection)
  {
    the_settings.setValue("options/show_by", "connection");
  }
  ui->the_tree->clear(); 
  ui->the_filter->setText("");
  OnDeviceChange();
}

void MainWindow::FetchColumnSettings()
{
  the_extended_columns.clear();
  ui->the_tree->setColumnCount(5);
  if (the_settings.value("columns/instance_id", true).toBool())
  {
    the_extended_columns.push_back("instance_id");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "Instance id");
  }
  if (the_settings.value("columns/first_hardware_id", false).toBool())
  {
    the_extended_columns.push_back("first_hardware_id");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "Hardware id");
  }
  if (the_settings.value("columns/manufacturer", false).toBool())
  {
    the_extended_columns.push_back("manufacturer");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "Manufacturer");
  }
  if (the_settings.value("columns/provider", false).toBool())
  {
    the_extended_columns.push_back("provider");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "Provider");
  }
  if (the_settings.value("columns/class", false).toBool())
  {
    the_extended_columns.push_back("class");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "Class");
  }
  if (the_settings.value("columns/pdo_name", false).toBool())
  {
    the_extended_columns.push_back("pdo_name");
    ui->the_tree->headerItem()->setText(the_extended_columns.size(), "PDO name");
  }
  ui->the_tree->setColumnCount(the_extended_columns.size()+1);

}

void MainWindow::OnShowHidden(bool show)
{
  the_settings.setValue("options/show_hidden", show);
  OnDeviceChange();
}

void MainWindow::LoadView()
{
  if (ui->actionDevices_by_type->isChecked())
  {
    LoadByTypeView();
  }
  else if(ui->actionDevices_by_connection->isChecked())
  {
    LoadByConnectionView();
  }
}

void MainWindow::OnDeviceChange()
{
  if (!the_import_mode)
  {
    EnumDevices();
  }
  LoadView();
  OnFilter();
  OnSetupMenu();
}

void MainWindow::OnSelectColumns()
{
  Columns dlg(this);
  if (dlg.exec())
  {
    FetchColumnSettings();
    ui->the_tree->clear();
    LoadView();
    OnFilter();
  }
}

bool MainWindow::ItemMatchesFilter(QTreeWidgetItem* item)
{

  for (int c = 0; c < ui->the_tree->columnCount(); c++)
  {
    QString val = item->text(c).toLower();
    for (auto o : the_filter_or_s)
    {
      for (auto _ : o)
      {
        if (val.indexOf(_) < 0)
        {
          goto next;
        }
      }
      return(true);
next:;
    }
  }

  return(false);
}
void MainWindow::OnFilter()
{ 
  // the text in the filter field changed
  QString filter = ui->the_filter->text().trimmed().toLower();

  the_filter_or_s.clear(); 
  QStringList and_s = filter.split("|", Qt::SkipEmptyParts);
  for (auto _ : and_s)
  {
    the_filter_or_s.push_back(_.split(" ", Qt::SkipEmptyParts));
  }


  if (filter.length()) 
  {
    {
      vector<QTreeWidgetItem*> items;
      items.reserve(1024);
      QSignalBlocker bs(ui->the_tree);
      QTreeWidgetItemIterator it(ui->the_tree);
      while (*it)
      {
        if (!(*it)->isExpanded()
          && (*it)->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator)
        {
          items.push_back(*it);
        }
        ++it;
      }
      for (auto item : items)
      {                                   // call PopulateLeaf() in the context of original signal
        PopulateLeaf(item);               // so that transition effects know where the event is from
        if (item->childCount())
        {
          item->setExpanded(true);
        }
      }
    }
    {
      QTreeWidgetItemIterator it(ui->the_tree);
      while (*it)
      {
        (*it)->setHidden(true);
        ++it;
      }
    }
    {
      QTreeWidgetItemIterator it(ui->the_tree);
      while (*it)
      {
        if (ItemMatchesFilter(*it))
        {
          (*it)->setHidden(false);
          for (QTreeWidgetItem* parent = (*it)->parent(); parent; parent = parent->parent())
          {
            parent->setHidden(false);
          }
        }
        ++it;
      }
    }
  }
  else 
  {
    QTreeWidgetItemIterator it(ui->the_tree);
    while (*it)
    {
      if (the_filter_was.length())
      {
        if ((*it)->isExpanded())
        {
          (*it)->setExpanded(false);
        }
      }
      (*it)->setHidden(false);
      ++it;
    }
  }
  for (int i = 0; i < ui->the_tree->columnCount(); i++)
  {
    ui->the_tree->resizeColumnToContents(i);
  }
  the_filter_was = filter;
}


void MainWindow::OnAbout()
{
  About dlg(this);
  dlg.exec();
}

void MainWindow::OnItemExpanded(QTreeWidgetItem* parent)
{ 
  parent->treeWidget()->setSortingEnabled(false);
  PopulateLeaf(parent);
  // because we block signals when programatically expanding an item, this only gets 
  // called when a single item is expanded, so it's reasonably performant to sort and resize herer
  ui->the_tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  for (int i = 0; i < ui->the_tree->columnCount(); i++)
  {
    ui->the_tree->resizeColumnToContents(i);
  }
}

void MainWindow::PopulateLeaf(QTreeWidgetItem* parent)
{
  // set up a QTreeWidgetItem and where appropriate its descendants
  // note that this implements a simple model to reflect changes without a full reload 
  // on every change as in the default device manager 
  // this gives reasonable performance whilst avoiding the complexity of full blown model / view implementation
  map<QString, QTreeWidgetItem*> devices_were;
  set<QString> devices_now;

  QString parent_class_guid = parent->data(0, ROLE_CLASS_GUID).toString();
  QString parent_instance_id = parent->data(0, ROLE_INSTANCE_ID).toString();

  for (int i = 0; i < parent->childCount(); i++)
  {
    QTreeWidgetItem* item = parent->child(i);
    devices_were[item->data(0, ROLE_INSTANCE_ID).toString()] = item;
  }

  for (auto it = device::the_devices.begin(); it != device::the_devices.end(); it++)
  {
    if (!it->connected
      && !ui->actionShow_hidden_devices->isChecked())
    {
      continue; 
    }

    if ((ui->actionDevices_by_type->isChecked() && it->class_guid_readable == parent_class_guid)
      || (ui->actionDevices_by_connection->isChecked() && it->parent == parent_instance_id)
      )
    {
      devices_now.insert(it->instance_id);
      QTreeWidgetItem* item = NULL;
      auto ite = devices_were.find(it->instance_id);
      if (ite == devices_were.end())
      {
        item = new QTreeWidgetItem(parent);
        item->setData(0, ROLE_INSTANCE_ID, it->instance_id);
        item->setData(0, ROLE_CLASS_GUID, it->class_guid_readable);
        item->setData(0, ROLE_IS_DEVICE, true); // as opposed to a class in type view 
        if (sender() == the_native_events)
        { // newly added, show green for 5 seconds  
          the_redraw_timer.start();
          for (int c = 0; c < ui->the_tree->columnCount(); c++)
          {
            item->setForeground(c, QColor("#a3e635"));
            QFont f = item->font(c);
            f.setBold(true);
            item->setFont(c, f);
          }
        }
      }
      else
      {
        item = ite->second;
      }
      item->setData(0, ROLE_ENABLED, it->enabled);
      item->setIcon(0, g.IconFromPath(it->icon, it->connected, !it->enabled, it->problem));
      item->setText(0, it->description);
      if (sender() == &the_redraw_timer)
      {
        for (int c = 0; c < ui->the_tree->columnCount(); c++)
        {
          item->setForeground(c, Qt::black);
          QFont f = item->font(c);
          f.setBold(false);
          item->setFont(c, f);
        }
      }
      if (ui->actionDevices_by_connection->isChecked())
      {
        // implements on demand loading of children 
        if (it->has_children)
        { 
          item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
        else
        {
          item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        }
      }
      int c = 1;
      for (auto e : the_extended_columns)
      {
        if (e == "instance_id")
        {
          item->setText(c, it->instance_id_display);
        }
        if (e == "first_hardware_id")
        { 
          // hardware id is a multi sz, we show the first in the tree, the full list can be seen in properties. 
          item->setText(c, it->first_hardware_id); 
        }
        if (e == "manufacturer")
        {
          item->setText(c, it->manufacturer);
        }
        if (e == "provider")
        {
          item->setText(c, it->provider);
        }
        if (e == "class")
        {
          item->setText(c, it->_class);
        }
        if (e == "pdo_name")
        {
          item->setText(c, it->pdo_name);
        }
        c++;
      }
    }
  }

  vector<QTreeWidgetItem*> deletes;
  for (int i = 0; i < parent->childCount(); i++)
  {
    QTreeWidgetItem* item = parent->child(i);
    if (devices_now.find(item->data(0, ROLE_INSTANCE_ID).toString()) == devices_now.end())
    {
      deletes.push_back(item);
    }
    else  if (ui->actionDevices_by_connection->isChecked() /*&& item->isExpanded()*/)
    {
      PopulateLeaf(item);
      if (item->childCount() && ui->the_filter->text().trimmed().length())
      {
        item->setExpanded(true);
      }
    }
  }
  for (auto d : deletes)
  {
    if (sender() == the_native_events)
    { 
      // removed device, show in red for 3 seconds after wm_device change
      the_redraw_timer.start();
      for (int c = 0; c < ui->the_tree->columnCount(); c++)
      {
        d->setForeground(c, QColor("#f87171"));
        QFont f = d->font(c);
        f.setBold(true);
        d->setFont(c, f);
      }
    }
    // subsequent update will have sender() as below, remove the entry
    if (sender() == &the_redraw_timer
      || sender() == ui->actionShow_hidden_devices)
    {
      delete d;
    }
  }
}


void MainWindow::LoadByTypeView()
{
  ui->the_tree->setSortingEnabled(false);
  
  map<QString, QTreeWidgetItem*> classes_were;
  set<QString> classes_now;

  QTreeWidgetItem* root_item = ui->the_tree->invisibleRootItem();
  for (int i = 0; i < root_item->childCount(); i++)
  {
    QTreeWidgetItem* item = root_item->child(i);
    classes_were[item->text(0)] = item;
  }
  // get the list of device classes recorded in the previous enumeration 
  for(auto it = the_used_device_class_guids.begin(); it != the_used_device_class_guids.end(); it++)
  {
    QString _class;
    if (it->second != GUID_NULL)
    {
      _class= g.GetClassProperty(it->second, DEVPKEY_DeviceClass_Name);
    }
    else
    {
      _class= tr("Other devices");
    }
    classes_now.insert(_class);
    QTreeWidgetItem* item = NULL;
    if (_class.length())
    {
      auto ite = classes_were.find(_class);
      if (ite == classes_were.end())
      {
        item = new QTreeWidgetItem(ui->the_tree);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        item->setData(0, ROLE_CLASS_GUID, it->first);
        item->setData(0, ROLE_IS_DEVICE, false); // as opposed to a class in type view 
        ui->the_tree->addTopLevelItem(item);
      }
      else
      {
        item = ite->second;
      }
      item->setIcon(0, g.IconFromPath(g.GetClassIconPath(it->second), true, false, false));
      item->setText(0, _class);
    }
  }

  // populate any expanded level 1 items (device classes) 
  vector<QTreeWidgetItem*> deletes;
  for (int i = 0; i < root_item->childCount(); i++)
  {
    QTreeWidgetItem* item = root_item->child(i);
    if (classes_now.find(item->text(0)) == classes_now.end())
    {
      deletes.push_back(item);
    }
    else
    {
      if (item->isExpanded())
      {
        PopulateLeaf(item);
      }
    }
  }
  for (auto d : deletes)
  {
    delete d;
  }

  ui->the_tree->sortByColumn(0,Qt::SortOrder::AscendingOrder);
  for (int i = 0; i < ui->the_tree->columnCount(); i++)
  {
    ui->the_tree->resizeColumnToContents(i);
  }
}

void MainWindow::LoadByConnectionView()
{ 
  //hierarchical view of devices 
  ui->the_tree->setSortingEnabled(false);
  QString root;
  for (auto it = device::the_devices.begin(); it != device::the_devices.end(); it++)
  {
    if (it->parent == "")
    {
      root = it->instance_id;
      break;
    }
  }
  ui->the_tree->invisibleRootItem()->setData(0, ROLE_INSTANCE_ID, root);
  PopulateLeaf(ui->the_tree->invisibleRootItem());
  ui->the_tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  for (int i = 0; i < ui->the_tree->columnCount(); i++)
  {
    ui->the_tree->resizeColumnToContents(i);
  }
}


void MainWindow::EnumDevices()
{ 
  // on each update we build a complete list of device instances, this is relatively quick
  // loading into the treewidget is done on demand as this is a little slower 
  device::the_devices.clear();
 
  set<QString> parents;

  DeviceInfoSet dis(NULL, DIGCF_ALLCLASSES);
  for (int i=0; ; i++)
  {
    if (!dis.Enumerate(i))
    {
      break;
    }
    
    device d;
    d.parent = dis.GetDeviceProperty(&DEVPKEY_Device_Parent).toLower();
    d.connected = dis.GetDeviceRegistryProperty(SPDRP_PHYSICAL_DEVICE_OBJECT_NAME).length();
    d.instance_id_display = dis.GetDeviceProperty(&DEVPKEY_Device_InstanceId);
    d.instance_id = d.instance_id_display.toLower();
    if (d.instance_id_display.left(1) == "{")
    {
      d.instance_id_display = d.instance_id_display.mid(d.instance_id_display.indexOf("}\\") + 2);
    }
/*    if (!d.connected
      && !ui->actionShow_hidden_devices->isChecked())
    {
      if (ui->actionDevices_by_connection->isChecked()
         && d.parent == "")
      { // need to include the root device for connections view
      }
      else
      {
        continue;
      }
    }*/
    d.class_guid_readable = dis.GetDeviceRegistryProperty(SPDRP_CLASSGUID);
    
    if (d.class_guid_readable.length())
    {
      g.GUIDFromString(d.class_guid_readable.toStdString().c_str(), &d.class_guid);
    }
    else
    {
      d.class_guid = GUID_NULL;
      d.class_guid_readable = g.GuidToString(d.class_guid);
    }
    the_used_device_class_guids[d.class_guid_readable] = d.class_guid;
    QString description = dis.GetDeviceRegistryProperty(SPDRP_FRIENDLYNAME);
    if (!description.length())
    {
       description = dis.GetDeviceRegistryProperty(SPDRP_DEVICEDESC);
    }
    d.description = description;
    
    d.enabled = dis.GetDevicePropertyDW(&DEVPKEY_Device_ProblemCode) != CM_PROB_DISABLED;
    if (!d.enabled || !d.connected)
    {
      d.problem = false;
    }
    else
    {
      d.problem= dis.GetDevicePropertyDW(&DEVPKEY_Device_ProblemCode) != 0;
    }
    d.icon = dis.GetDeviceProperty(&DEVPKEY_DrvPkg_Icon);
    if (!d.icon.length())
    {
      d.icon = g.GetClassIconPath(d.class_guid);
    }
    d.first_hardware_id = dis.GetDeviceProperty(&DEVPKEY_Device_HardwareIds);
    d.manufacturer = dis.GetDeviceProperty(&DEVPKEY_Device_Manufacturer);
    d.provider = dis.GetDeviceProperty(&DEVPKEY_Device_DriverProvider);
    d._class= dis.GetDeviceProperty(&DEVPKEY_Device_Class);
    d.pdo_name = dis.GetDeviceProperty(&DEVPKEY_Device_PDOName);
    parents.insert(d.parent);

    if (d.instance_id.length())
    {  // if a device goes away after creating DeviceInfoSet we get an empty entry back, ignore it
      device::the_devices.push_back(d); 
    }
  }
  for (auto& _ : device::the_devices)
  {
    _.has_children = parents.find(_.instance_id) != parents.end();
  }

}

void MainWindow::OnSetupMenu()
{
  if (the_import_mode)
  {
    ui->the_disable->setEnabled(false);
    ui->the_enable->setEnabled(false);
    ui->the_uninstall->setEnabled(false);
    ui->the_properties->setEnabled(false);
    return;
  }

  bool allSelnsDisabledDevice = true;
  bool allSelnsEnabledDevice = true;
  bool allSelnsDevice = true;
  int cnt = 0;
  QTreeWidgetItemIterator it(ui->the_tree, QTreeWidgetItemIterator::Selected);
  while (*it)
  {
    if ((*it)->data(0, ROLE_ENABLED).toBool())
    {
      allSelnsDisabledDevice = false;
    }
    else
    {
      allSelnsEnabledDevice = false;
    }
    if (!(*it)->data(0, ROLE_IS_DEVICE).toBool())
    {
      allSelnsDisabledDevice = false;
      allSelnsEnabledDevice = false;
      allSelnsDevice = false;
    }
    cnt++;
    it++;
  }
  if (!cnt)
  {
    allSelnsDisabledDevice = false;
    allSelnsEnabledDevice = false;
    allSelnsDevice = false;
  }
  ui->the_disable->setEnabled(allSelnsEnabledDevice);
  ui->the_enable->setEnabled(allSelnsDisabledDevice);
  ui->the_uninstall->setEnabled(allSelnsDevice);
  ui->the_properties->setEnabled(allSelnsDevice && cnt == 1);
}

void MainWindow::OnRightClick(const QPoint& pos)
{
  OnSetupMenu();

  bool any = false;
  QMenu menu(this);
  if (ui->the_disable->isEnabled())
  {
    menu.addAction(ui->the_disable);
    any = true;
  }
  if (ui->the_enable->isEnabled())
  {
    menu.addAction(ui->the_enable);
    any = true;
  }
  if (ui->the_uninstall->isEnabled())
  {
    menu.addAction(ui->the_uninstall);
    any = true;
  }
  if (ui->the_properties->isEnabled())
  {
    if (any)
    {
      menu.addSeparator();
    }
    menu.addAction(ui->the_properties);
    any = true;
  }

  if (any)
  {
    QPoint pt(pos);
    menu.exec(ui->the_tree->mapToGlobal(pos));
  }
}

void MainWindow::OnDisable()
{ 
  // disable the selected device(s). Unlike the Windows device manager multiple selections are allowed 
  QTreeWidgetItemIterator it(ui->the_tree, QTreeWidgetItemIterator::Selected);
  DeviceInfoSet dis(NULL, DIGCF_ALLCLASSES);
  while (*it)
  {
    QString instance_id = (*it)->data(0, ROLE_INSTANCE_ID).toString();
    if (dis.Find(instance_id))
    {
      dis.DisableDevice();
    }
    ++it;
  }
}

void MainWindow::OnEnable()
{
  // enable the selected device(s). Unlike the Windows device manager multiple selections are allowed 
  QTreeWidgetItemIterator it(ui->the_tree, QTreeWidgetItemIterator::Selected);
  DeviceInfoSet dis(NULL, DIGCF_ALLCLASSES);
  while (*it)
  {
    QString instance_id = (*it)->data(0, ROLE_INSTANCE_ID).toString();
    if (dis.Find(instance_id))
    {
      dis.EnableDevice();
    }
    ++it;
  }
}

void MainWindow::OnUninstall()
{
  // delete the selected device(s). Unlike the Windows device manager multiple selections are allowed 
  QTreeWidgetItemIterator it(ui->the_tree, QTreeWidgetItemIterator::Selected);
  while (*it)
  {
    QString instance_id = (*it)->data(0, ROLE_INSTANCE_ID).toString();

    DeviceInfoSet dis(NULL, DIGCF_ALLCLASSES);
    if (dis.Find(instance_id))
    {
      dis.RemoveDevice(true);
    }
    ++it;
  }
  the_native_events->FakeDeviceChange(); // treat as a device change event to use common refresh logic 
}


void MainWindow::OnProperties()
{
  // launch the standard Windows device property dialog
  QTreeWidgetItemIterator it(ui->the_tree, QTreeWidgetItemIterator::Selected);
  if (*it)
  {
    QString instance_id = (*it)->data(0, ROLE_INSTANCE_ID).toString();
    QString cmd = QString("/DeviceId ") + instance_id;
    g.DeviceProperties_RunDLL((HWND)winId(), NULL, cmd.toStdString().c_str(), NULL);
  }
}

void MainWindow::OnScanHardware()
{
  DEVINST dnDevInst;
  CM_Locate_DevNode(&dnDevInst, NULL, CM_LOCATE_DEVNODE_NORMAL);
  CM_Reenumerate_DevNode(dnDevInst, 0);
}


void MainWindow::OnImport()
{
  QString selected_import_folder = the_settings.value("options/selected_import_folder", QString()).toString();

  QString fn = QFileDialog::getOpenFileName(this, tr("Open"), selected_import_folder, tr("Devine file (*.dvx)"));

  if (!fn.length())
  {
    return;
  }
  QFileInfo fi(fn);
  the_settings.setValue("options/selected_import_folder", fi.absolutePath());

  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
  {
    QMessageBox::warning(this, "Error", "Unable to open file");
    return;
  }

  if(!device::ImportAsXML(f))
  {
    QMessageBox::warning(this, "Error", "Unable to read file");
    return;
  }
  the_import_mode = true;
  QString title = tr("Devine showing imported file: %1").arg(fi.fileName());
  setWindowTitle(title);
  OnDeviceChange();
}

void MainWindow::OnExport()
{
  QString selected_export_filter = the_settings.value("options/selected_export_filter", QString()).toString();
  QString selected_export_folder = the_settings.value("options/selected_export_folder", QString()).toString();

  QString fn = QFileDialog::getSaveFileName(this, tr("Export"), selected_export_folder, tr("Devine file (*.dvx);;Text file (*.txt)"), &selected_export_filter);

  if (!fn.length())
  {
    return; 
  }
  QFileInfo fi(fn);
  the_settings.setValue("options/selected_export_filter", selected_export_filter);
  the_settings.setValue("options/selected_export_folder", fi.absolutePath());

  QFile fo(fn);
  if (!fo.open(QFile::WriteOnly))
  {
    QMessageBox::warning(this, "Error", "Unable to create export");
    return;
  }

  if (fn.right(4).toLower() == ".dvx")
  {
    device::ExportAsXML(fo);
  }
  else
  {
    vector<qsizetype> widths;
    for (int i = 0; i < ui->the_tree->columnCount(); i++)
    {
      widths.push_back(0);
    }

    // calculate the character widths for each column
    std::function<void(QTreeWidgetItem* a_item, vector<qsizetype>& a_widths, int a_level)> calc_widths;
    calc_widths = [&](QTreeWidgetItem* a_item, vector<qsizetype>& a_widths, int a_level)
    {
      if (a_level)
      {
        for (int c = 0; c < a_widths.size(); c++)
        {
          qsizetype w = a_item->text(c).length();
          if (a_level > 1) // level 0 is invisible root, level 1 is real root
          {
            w += (a_level - 1) * 2 + 1;
          }
          a_widths[c] = max(a_widths[c], w);
        }
      }
      for (int c = 0; c < a_item->childCount(); c++)
      {
        QTreeWidgetItem* child = a_item->child(c);
        if (!child->isHidden())
        {
          calc_widths(child, a_widths, a_level + 1);
        }
      }
    };
    calc_widths(ui->the_tree->invisibleRootItem(), widths, 0);
    // do the export
    std::function<void(QTreeWidgetItem* a_item, int a_level)> text_export;
    text_export = [&](QTreeWidgetItem* a_item, int a_level)
    {
      if(a_level)
      {
        for (int c = 0; c < a_item->treeWidget()->columnCount(); c++)
        {
          int tree_visualisation_width = 0;
          if (!c && a_level > 1)
          {
            fo.write("|");
            tree_visualisation_width = (a_level - 1) * 2;
            fo.write(QString().leftJustified(tree_visualisation_width - 1, '_').toLatin1());
          }
          fo.write(a_item->text(c).leftJustified((widths[c] + 1) - tree_visualisation_width).toLatin1());
        }
        fo.write("\r\n");
      }
      for (int c = 0; c < a_item->childCount(); c++)
      {
        QTreeWidgetItem* child = a_item->child(c);
        if (!child->isHidden())
        {
          text_export(child, a_level + 1);
        }
      }
      if (a_level==1)
      {
        fo.write("\r\n");
      }
    };
    text_export(ui->the_tree->invisibleRootItem(),  0);

  }
}


















