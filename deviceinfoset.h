#include <windows.h>
#include <QString.h>
#include <QStringList.h>
#include <propsys.h>
#ifndef _SETUPAPI_VER 
#define _SETUPAPI_VER _WIN32_WINNT_WIN10
#endif
#include <setupapi.h>


class DeviceInfoSet
{
public:
  DeviceInfoSet(const GUID* a_guid, DWORD a_flags);
  ~DeviceInfoSet();
  bool Enumerate(int i);
  bool Find(const QString& a_instance_id);
  bool FindLocation(const QString& a_location);
  QString GetDeviceProperty(const DEVPROPKEY* a_property_key);
  DWORD GetDevicePropertyDW(const DEVPROPKEY* a_property_key);
  QString GetDeviceRegistryProperty(DWORD a_property);
  DWORD GetDeviceRegistryPropertyDW(DWORD a_property);
  void EnableDevice();
  void DisableDevice();
  void StopDevice();
  void StartDevice();
  void Dump();
  void RemoveDevice(bool a_remove_oem_package);
  bool SetDeviceRegistryProperty(DWORD a_property, const QStringList& a_values);
  static void Reenumerate();

private:
  HDEVINFO the_info_set;
  SP_DEVINFO_DATA the_info_data;

  void ChangeDeviceState(int aCode);
  
};
