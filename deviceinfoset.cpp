#include <windows.h>
#include <initguid.h>
#include <devpkey.h>
#include <memory>
using namespace std;
#include "deviceinfoset.h"
#include "cfgmgr32.h"
#include <QDebug>

// OO wrapper for Windows SetupDiXXX stuff
// makes the client code more readable 

DeviceInfoSet::DeviceInfoSet(const GUID* a_guid, DWORD a_flags)
{
  the_info_set = SetupDiGetClassDevsA(a_guid, 0, 0, a_flags);
}

DeviceInfoSet::~DeviceInfoSet()
{
  SetupDiDestroyDeviceInfoList(the_info_set);
}

bool DeviceInfoSet::Enumerate(int i)
{
  the_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
  return(SetupDiEnumDeviceInfo(the_info_set, i, &the_info_data));
}

bool DeviceInfoSet::Find(const QString& a_instance_id)
{
  for (int i = 0; ; i++)
  {
    if (!Enumerate(i))
    {
      return(false);
    }
    QString instance_id = GetDeviceProperty(&DEVPKEY_Device_InstanceId).toLower();
    if (instance_id == a_instance_id.toLower())
    {
      return(true);
    }
  }
}

bool DeviceInfoSet::FindLocation(const QString& a_location)
{
  for (int i = 0; ; i++)
  {
    if (!Enumerate(i))
    {
      return(false);
    }
    QStringList _ = GetDeviceProperty(&DEVPKEY_Device_InstanceId).toLower().split("\\");
    QString location = _[_.count() - 1];
    if (location == a_location.toLower())
    {
      return(true);
    }
  }
}

void DeviceInfoSet::Dump()
{
  for (int i = 0; ; i++)
  {
    if (!Enumerate(i))
    {
      return;
    }
    QString instance_id = GetDeviceProperty(&DEVPKEY_Device_InstanceId).toLower();
    qDebug() << instance_id;
  }
}


QString DeviceInfoSet::GetDeviceProperty(const DEVPROPKEY* a_property_key)
{
  WCHAR value[1024] = L"";
  ULONG type;

  SetupDiGetDeviceProperty(the_info_set, &the_info_data, a_property_key, &type, (PBYTE)value, sizeof(value), NULL, 0);
  return(QString::fromWCharArray(value));
}

DWORD DeviceInfoSet::GetDevicePropertyDW(const DEVPROPKEY* a_property_key)
{
  DWORD value;
  ULONG type;

  SetupDiGetDeviceProperty(the_info_set, &the_info_data, a_property_key, &type, (PBYTE)&value, sizeof(value), NULL, 0);
  return(value);
}

QString DeviceInfoSet::GetDeviceRegistryProperty(DWORD a_property)
{
  WCHAR value[1024] = L"";
  SetupDiGetDeviceRegistryProperty(the_info_set, &the_info_data, a_property, NULL, (PBYTE)value, sizeof(value), NULL);
  return(QString::fromWCharArray(value));
}

DWORD DeviceInfoSet::GetDeviceRegistryPropertyDW(DWORD a_property)
{
  DWORD value;
  SetupDiGetDeviceRegistryPropertyA(the_info_set, &the_info_data, a_property, NULL, (PBYTE)&value, sizeof(value), NULL);
  return(value);
}

void DeviceInfoSet::EnableDevice()
{
  ChangeDeviceState(DICS_ENABLE);
}

void DeviceInfoSet::DisableDevice()
{
  ChangeDeviceState(DICS_DISABLE);
}

void DeviceInfoSet::StartDevice()
{
  ChangeDeviceState(DICS_START);
}

void DeviceInfoSet::StopDevice()
{
  ChangeDeviceState(DICS_STOP);
}

void DeviceInfoSet::RemoveDevice(bool a_remove_oem_package)
{
  QString inf_path = GetDeviceProperty(&DEVPKEY_Device_DriverInfPath).toLower();
  SetupDiRemoveDevice(the_info_set, &the_info_data);
  if (a_remove_oem_package 
    && inf_path.left(3) == "oem")
  {
    SetupUninstallOEMInfA(inf_path.toLatin1().constData(), 0, NULL);
  }
}


void DeviceInfoSet::ChangeDeviceState(int aCode)
{
  SP_PROPCHANGE_PARAMS params;
  memset(&params, 0, sizeof(SP_PROPCHANGE_PARAMS));
  params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
  params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

  params.StateChange = aCode;
  params.Scope = DICS_FLAG_CONFIGSPECIFIC;
  params.HwProfile = 0;

  SetupDiSetClassInstallParamsA(the_info_set, &the_info_data, (PSP_CLASSINSTALL_HEADER)&params, sizeof(SP_PROPCHANGE_PARAMS));
  SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, the_info_set, &the_info_data);
}


bool DeviceInfoSet::SetDeviceRegistryProperty(DWORD a_property, const QStringList& a_values)
{
  int l = 1;
  for (auto v : a_values)
  {
    l += v.length() + 1;
  }
  unique_ptr<char> buf(new char[l]);
  char* p = buf.get();

  for (auto v : a_values)
  {
    strcpy(p, v.toStdString().c_str());
    p += v.length() + 1;
  }
  *p = '\0';
  return(SetupDiSetDeviceRegistryPropertyA(the_info_set, &the_info_data, a_property, (PBYTE)buf.get(), (DWORD)l));

}

/*static*/
void DeviceInfoSet::Reenumerate()
{
  DEVINST dnDevInst;
  CM_Locate_DevNode(&dnDevInst, NULL, CM_LOCATE_DEVNODE_NORMAL);
  CM_Reenumerate_DevNode(dnDevInst, 0);
}



