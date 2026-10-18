#ifndef UINPUTCHANNEL_H
#define UINPUTCHANNEL_H

#include <QObject>
#include <QString>
#include <QMap>

class UInputChannel : public QObject
{
    Q_OBJECT

public:
    explicit UInputChannel(QObject *parent = nullptr);
    ~UInputChannel() override;

    bool isAvailable() const { return m_isAvailable; }
    QString statusMessage() const { return m_statusMessage; }

    bool openDevice();
    void closeDevice();

    void sendKeyDown(const QString &keyName);
    void sendKeyUp(const QString &keyName);
    void sendKeyTap(const QString &keyName, int tapDurationMs = 80);

    enum class ScanCodeConvention {
        Auto,       // Detect based on QGuiApplication::platformName() (XkbOffset8 on xcb/wayland)
        XkbOffset8, // nativeScanCode is evdev + 8 (X11 / Wayland desktop convention)
        DirectEvdev // nativeScanCode is raw evdev code
    };

    static int keyCodeFromName(const QString &name);
    static QString nameFromKeyCode(int code);
    static QString nameFromQtKey(int qtKey, quint32 nativeScanCode = 0, ScanCodeConvention convention = ScanCodeConvention::Auto);
    static int evdevCodeFromScanCode(quint32 nativeScanCode, ScanCodeConvention convention = ScanCodeConvention::Auto);

private:
    bool m_isAvailable = false;
    QString m_statusMessage;
    int m_fd = -1;

    void emitRawEvent(int type, int code, int value);
    void syncEvent();
    static void initKeyMaps();
};

#endif // UINPUTCHANNEL_H
