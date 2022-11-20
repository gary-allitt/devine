#include <windows.h>

#include <cfgmgr32.h>
#include <QPainter>
#include "glue.h"

Glue::Glue()
{
  the_shell32_dll = LoadLibraryA("shell32.dll");
  if (the_shell32_dll)
  {
    GUIDFromString = (LPFN_GUIDFromString)GetProcAddress(the_shell32_dll, MAKEINTRESOURCEA(703));
  }
  HINSTANCE devmgr_dll = LoadLibraryA("devmgr.dll");
  if (devmgr_dll)
  {
    DeviceProperties_RunDLL = (LPFN_DeviceProperties_RunDLL)GetProcAddress(devmgr_dll, MAKEINTRESOURCEA(5));
  }

}

Glue::~Glue()
{
  if (the_shell32_dll)
  {
    FreeLibrary(the_shell32_dll);
  }
}

QString Glue::ExpandEnvironmentStrings(const QString& aString)
{
  WCHAR raw[1024];
  WCHAR expanded[1024] = L"";
  qsizetype l = aString.toWCharArray(raw);
  raw[l] = 0;
  ::ExpandEnvironmentStrings(raw, expanded, sizeof(expanded));
  QString e;
  e = e.fromWCharArray(expanded);
  return(e);
}


HMODULE Glue::MyGetModuleHandle(const QString& a_path)
{
  auto it = the_module_handles.find(a_path);
  if (it != the_module_handles.end())
  {
    return(it->second);
  }
  QString path = ExpandEnvironmentStrings(a_path);
  HMODULE hm = LoadLibraryA(path.toStdString().c_str());
  the_module_handles[a_path] = hm;
  return(hm);
}

void Glue::GrayImage(QImage& image)
{
  image = image.convertToFormat(QImage::Format_ARGB32);
  for (int y = 0; y < image.height(); ++y)
  {
    QRgb* scanLine = (QRgb*)image.scanLine(y);
    for (int x = 0; x < image.width(); ++x)
    {
      QRgb pixel = *scanLine;
      uint ci = uint(qGray(pixel));
      *scanLine = qRgba(ci, ci, ci, qAlpha(pixel) / 3);
      ++scanLine;
    }
  }
}

QPixmap Glue::OverlayPixmap(const QPixmap& base, const QPixmap& overlay)
{
  QPixmap result(base.width(), base.height());
  result.fill(Qt::transparent);
  {
    QPainter painter(&result);
    painter.drawPixmap(0, 0, base);
    painter.drawPixmap(0, 0, overlay);
  }
  return(result);
}

QPixmap Glue::DisabledPixmap(const QPixmap& pixmap)
{
  QPixmap px_disabled = QPixmap(":/images/disabled.png");
  return(OverlayPixmap(pixmap, px_disabled));
}

QPixmap Glue::ProblemPixmap(const QPixmap& pixmap)
{
  QPixmap px_problem = QPixmap(":/images/error.png");
  return(OverlayPixmap(pixmap, px_problem));
}

QPixmap Glue::IconFromResource(const QString& a_resource_file, int a_index, bool connected, bool disabled, bool problem)
{
  QString key = QString("%1:%2:%3:%4:%5").arg(a_resource_file).arg(a_index).arg(connected).arg(disabled).arg(problem);
  auto it = the_pixmap_cache.find(key);
  if (it != the_pixmap_cache.end())
  {
    return(it->second);
  }
  HMODULE hm = MyGetModuleHandle(a_resource_file);
  HANDLE himage = LoadImageA(hm, MAKEINTRESOURCEA(a_index), IMAGE_ICON, 16, 16, 0);
  QImage image = QImage::fromHICON((HICON)himage);
  if (!connected)
  {
    GrayImage(image);
  }
  QPixmap px = QPixmap::fromImage(image);
  if (disabled)
  {
    px = DisabledPixmap(px);
  }
  if (problem)
  {
    px = ProblemPixmap(px);
  }
  the_pixmap_cache[key] = px;
  return(px);
}

QPixmap Glue::IconFromPath(const QString& a_path, bool connected, bool disabled, bool problem)
{
  QString key = QString("%1:%2:%3:%4").arg(a_path).arg(connected).arg(disabled).arg(problem);
  QPixmap px;
  auto it = the_pixmap_cache.find(key);
  if (it != the_pixmap_cache.end())
  {
    return(it->second);
  }

  QStringList path_toks = a_path.split(",");
  if (path_toks.size() > 1)
  {
    px = IconFromResource(path_toks[0], path_toks[1].mid(1).toInt(), connected, false, false);
  }
  else
  {
    QString path = ExpandEnvironmentStrings(path_toks[0]);
    HANDLE himage = LoadImageA(NULL, path.toStdString().c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    QImage image = QImage::fromHICON((HICON)himage);
    if (!connected)
    {
      GrayImage(image);
    }

    px = QPixmap::fromImage(image);
  }
  if (disabled)
  {
    px = DisabledPixmap(px);
  }
  if (problem)
  {
    px = ProblemPixmap(px);
  }
  the_pixmap_cache[key] = px;
  return(px);
}

QString Glue::GetClassIconPath(GUID guid)
{
  DEVPROPTYPE type;
  BYTE icon_path_w[MAX_PATH];
  ULONG size = sizeof(icon_path_w);
  QPixmap px;
  if (guid != GUID_NULL
    && CM_Get_Class_Property(&guid, &DEVPKEY_DeviceClass_IconPath, &type, icon_path_w, &size, CM_CLASS_PROPERTY_INSTALLER) == CR_SUCCESS)
  {
    return(QString::fromWCharArray((WCHAR*)icon_path_w));
  }
  return("%systemroot%\\system32\\setupapi.dll,-18");
}


QString Glue::GetClassProperty(GUID guid, DEVPROPKEY Property)
{
  QString ret;
  char value[1024] = "";
  ULONG type;
  ULONG size = sizeof(value);

  if (CM_Get_Class_Property(&guid, &Property, &type, (PBYTE)value, &size, 0) == CR_SUCCESS)
  {
    if (type == DEVPROP_TYPE_STRING)
    {
      ret = QString::fromWCharArray((PWCHAR)value);
    }
  }

  return(ret);
}

QString Glue::GuidToString(GUID guid)
{
  OLECHAR  guid_s[256];
  StringFromGUID2(guid, guid_s, sizeof(guid_s));
  return(QString::fromWCharArray(guid_s));
}


