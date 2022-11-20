#include <windows.h>
#include <devpkey.h>
#include <map>
//#include <vector>
//#include <set>
using namespace std;

#include <QString>
#include <QPixmap>

typedef void (WINAPI* LPFN_DeviceProperties_RunDLL)(HWND, HINSTANCE, LPCSTR, int);
typedef BOOL(WINAPI* LPFN_GUIDFromString)(LPCSTR, LPGUID);

class Glue
{
public:
  Glue();
  ~Glue();

  QPixmap IconFromPath(const QString& a_path, bool connected, bool disabled, bool problem);
  QString GetClassProperty(GUID guid, DEVPROPKEY Property);
  QString GetClassIconPath(GUID guid);

  QString GuidToString(GUID guid);

  LPFN_GUIDFromString GUIDFromString;
  LPFN_DeviceProperties_RunDLL DeviceProperties_RunDLL;
private:
  map<QString, HMODULE> the_module_handles;

  HINSTANCE the_shell32_dll;

  QString ExpandEnvironmentStrings(const QString& aString);
  HMODULE MyGetModuleHandle(const QString& a_path);
  QPixmap IconFromResource(const QString& a_resource_file, int a_index, bool connected, bool disabled, bool problem);
  map<QString, QPixmap> the_pixmap_cache;

  void GrayImage(QImage& image);
  QPixmap OverlayPixmap(const QPixmap& base, const QPixmap& overlay);
  QPixmap DisabledPixmap(const QPixmap& pixmap);
  QPixmap ProblemPixmap(const QPixmap& pixmap);



};