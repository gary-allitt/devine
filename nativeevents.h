#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeEvents : public QObject, public QAbstractNativeEventFilter
{
  Q_OBJECT
public:
  NativeEvents(); 
  virtual bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr*) Q_DECL_OVERRIDE;
  void FakeDeviceChange();
public slots:
  signals:
  void sigDeviceChange();
private:
};
