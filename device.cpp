#include <windows.h>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QVariant>
#include <devpkey.h>
#include <cfgmgr32.h>
#include <cfg.h>
#include <initguid.h>
#include "glue.h"
#include "device.h"
#include "deviceinfoset.h"
#include <vector>
#include <set>
using namespace std;

vector<device> device::the_devices;
map<QString, GUID> device::the_used_device_class_guids;
static Glue g;

device::device()
{
  static bool first = true;
  if (first)
  {
    the_devices.reserve(4096);
    first = false; 
  }
}

/*static*/ 
void device::ExportAsXML(QFile& a_file)
{
  QDomDocument doc;
  QDomElement root = doc.createElement("devices");
  doc.appendChild(root);

  for (auto device : the_devices)
  {
    device.ExportAsXML(root);
  }
  a_file.write(doc.toByteArray(2));
}

static void ExportNode(QDomElement& device, const QString& name, QVariant value)
{
  QDomDocument doc = device.ownerDocument();
  QDomElement e = doc.createElement(name);
  device.appendChild(e);
  QDomText t = doc.createTextNode(value.toString());
  e.appendChild(t);
}

void device::ExportAsXML(QDomElement& root)
{
  QDomDocument doc = root.ownerDocument();
  QDomElement device = doc.createElement("device");
  root.appendChild(device);
  ExportNode(device, "description", description);
  ExportNode(device, "class_guid_readable", class_guid_readable);
  ExportNode(device, "first_hardware_id", first_hardware_id);
  ExportNode(device, "instance_id", instance_id);
  ExportNode(device, "instance_id_display", instance_id_display);
  ExportNode(device, "manufacturer", manufacturer);
  ExportNode(device, "provider", provider);
  ExportNode(device, "_class", _class);
  ExportNode(device, "icon", icon);
  ExportNode(device, "parent", parent);
  ExportNode(device, "connected", connected);
  ExportNode(device, "has_children", has_children);
  ExportNode(device, "enabled", enabled);
  ExportNode(device, "problem", problem);
  ExportNode(device, "problem_code", problem_code);
  ExportNode(device, "device_type", device_type);
}

/*static*/
bool device::ImportAsXML(QFile& a_file)
{
  QDomDocument doc;
  if (!doc.setContent(&a_file))
  {
    return(false);
  }
  the_devices.clear();
  the_used_device_class_guids.clear();
  bool found_root = false; 
  QDomNode devices = doc.documentElement()/*.namedItem("devices")*/;
  for (QDomNode dn = devices.firstChild(); !dn.isNull(); dn = dn.nextSibling())
  {
    map<QString,QString> name_value;
    for (QDomElement device_attr = dn.firstChild().toElement(); !device_attr.isNull(); device_attr = device_attr.nextSibling().toElement())
    {
      name_value[device_attr.nodeName()] = device_attr.text();
    }
    device d;
    d.description = name_value["description"];
    if (d.description.indexOf("USB Serial Port") >= 0)
    {
      int i=0;
    }
    d.class_guid_readable = name_value["class_guid_readable"];
    d.first_hardware_id = name_value["first_hardware_id"];
    d.instance_id = name_value["instance_id"];
    d.instance_id_display = name_value["instance_id_display"];
    d.manufacturer = name_value["manufacturer"];
    d.device_type = name_value["device_type"];
    d.provider = name_value["provider"];
    d._class = name_value["_class"];
    d.icon = name_value["icon"];
    d.parent = name_value["parent"];
    d.connected = name_value["connected"]=="true";
    d.has_children = name_value["has_children"] == "true";
    d.enabled = name_value["enabled"] == "true";
    d.problem = name_value["problem"] == "true";
    d.problem_code = name_value["problem_code"].toULong();
    Glue g;
    g.GUIDFromString(d.class_guid_readable.toLatin1().constData(),&d.class_guid);
    the_devices.push_back(d);
    the_used_device_class_guids[d.class_guid_readable] = d.class_guid;
    if (d.parent == "")
    {
      found_root = true;
    }
  }
  // temp; original exports from W7 didn't include this 
  if (!found_root)
  {
    device d;
    d.instance_id = "htree\\root\\0";
    d.has_children = true;
    the_devices.push_back(d);
  }

  return(true);
}

/*static*/
void device::EnumDevices()
{
  // on each update we build a complete list of device instances, this is relatively quick
  // loading into the treewidget is done on demand as this is a little slower 
  the_devices.clear();

  set<QString> parents;

  DeviceInfoSet dis(NULL, DIGCF_ALLCLASSES);
  for (int i = 0; ; i++)
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
      d.problem = dis.GetDevicePropertyDW(&DEVPKEY_Device_ProblemCode) != 0;
    }
    d.icon = dis.GetDeviceProperty(&DEVPKEY_DrvPkg_Icon);
    if (!d.icon.length())
    {
      d.icon = g.GetClassIconPath(d.class_guid);
    }
    d.first_hardware_id = dis.GetDeviceProperty(&DEVPKEY_Device_HardwareIds);
    d.manufacturer = dis.GetDeviceProperty(&DEVPKEY_Device_Manufacturer);

    d.provider = dis.GetDeviceProperty(&DEVPKEY_Device_DriverProvider);
    d._class = dis.GetDeviceProperty(&DEVPKEY_Device_Class);
    d.pdo_name = dis.GetDeviceProperty(&DEVPKEY_Device_PDOName);
    d.problem_code = dis.GetDevicePropertyDW(&DEVPKEY_Device_ProblemCode);
    if (d.class_guid != GUID_NULL)
    {
      d.device_type = g.GetClassProperty(d.class_guid, DEVPKEY_DeviceClass_Name);
    }
    else
    {
      d.device_type = QObject::tr("Other devices");
    }

    parents.insert(d.parent);

    if (d.instance_id.length())
    {  // if a device goes away after creating DeviceInfoSet we get an empty entry back, ignore it
      the_devices.push_back(d);
    }
  }
  for (auto& _ : the_devices)
  {
    _.has_children = parents.find(_.instance_id) != parents.end();
  }

}

/*static*/ 
device device::GetDevice(const QString& a_instance_id)
{
  for (auto _ : the_devices)
  {
    if (_.instance_id == a_instance_id)
    {
      return(_);
    }
  }
  static device nulldevice;
  return(nulldevice);
}

