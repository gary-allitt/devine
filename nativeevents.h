#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeEvents : public QObject, public QAbstractNativeEventFilter
{
  Q_OBJECT
public:
  NativeEvents(); 
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  virtual bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) Q_DECL_OVERRIDE;
#else
  virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
#endif
    void FakeDeviceChange();
public slots:
  signals:
  void sigDeviceChange();
private:
};
