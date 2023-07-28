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
  QString device_type;
  bool connected;
  bool has_children;
  bool enabled;
  bool problem;
  uint32_t problem_code;

  static void ExportAsXML(QFile& a_file);
  static bool ImportAsXML(QFile& a_file);
  void ExportAsXML(QDomElement& root);

  static std::vector<device> the_devices;

  static device GetDevice(const QString& a_instance_id)
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

};
