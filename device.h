#include <QString>

class QDomElement;
class QFile;

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
  QString pdo_name;
  bool connected;
  bool has_children;
  bool enabled;
  bool problem;

  static void ExportAsXML(QFile& a_file);
  static bool ImportAsXML(QFile& a_file);
  void ExportAsXML(QDomElement& root);

  static std::vector<device> the_devices;

};
