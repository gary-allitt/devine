#include <windows.h>
#include <QObject>
#include "nativeevents.h"

// Qt stuff to handle WM_DEVICECHANGE

NativeEvents::NativeEvents()
{
}

/*virtual*/ 
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool NativeEvents::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*)
#else
bool NativeEvents::nativeEventFilter(const QByteArray& eventType, void* message, long*)
#endif

{
  if (eventType == "windows_generic_MSG")
  {
    MSG* msg = (MSG*)message;
    if (msg->message == WM_DEVICECHANGE)
    {
      emit sigDeviceChange();
    }
  }
  return false;
}

void NativeEvents::FakeDeviceChange()
{
  emit sigDeviceChange();
}
