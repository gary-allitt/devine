#include <windows.h>
#include <QObject>
#include "nativeevents.h"

// Qt stuff to handle WM_DEVICECHANGE

NativeEvents::NativeEvents()
{
}

/*virtual*/ 
bool NativeEvents::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*) 
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
