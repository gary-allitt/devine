#include <QString>
#include <initguid.h>

class QDomElement;
class QFile;

struct device
{
  device();
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
  QString device_type;
  bool connected;
  bool has_children;
  bool enabled;
  bool problem;
  uint32_t problem_code;

  static void ExportAsXML(QFile& a_file);
  static bool ImportAsXML(QFile& a_file);
  void ExportAsXML(QDomElement& root);
  static void EnumDevices();

  static device GetDevice(const QString& a_instance_id);
  static const std::vector<device>& GetDevices() { return(the_devices); };
  static const std::map<QString, GUID>& GetUsedClasses() { return(the_used_device_class_guids); }
private:
  static std::vector<device> the_devices;
  static std::map<QString, GUID> the_used_device_class_guids;

};
