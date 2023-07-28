#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QVariant>
#include <initguid.h>
#include "glue.h"
#include "device.h"
#include <vector>
using namespace std;

vector<device> device::the_devices;

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
    device::the_devices.push_back(d);
  }

  return(true);
}
