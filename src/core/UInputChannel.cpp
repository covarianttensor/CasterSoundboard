#include "UInputChannel.h"

#include <QDebug>
#include <QFile>
#include <QTimer>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

#ifdef __linux__
#include <linux/uinput.h>
#include <linux/input-event-codes.h>
#endif

namespace {
static QMap<QString, int> s_nameToCode;
static QMap<int, QString> s_codeToName;
static bool s_mapsInitialized = false;
}

UInputChannel::UInputChannel(QObject *parent)
    : QObject(parent)
{
    initKeyMaps();
    openDevice();
}

UInputChannel::~UInputChannel()
{
    closeDevice();
}

void UInputChannel::initKeyMaps()
{
    if (s_mapsInitialized)
        return;

#ifdef __linux__
    auto addKey = [](const QString &name, int code) {
        s_nameToCode.insert(name, code);
        s_codeToName.insert(code, name);
    };

    addKey("KEY_ESC", KEY_ESC);
    addKey("KEY_1", KEY_1);
    addKey("KEY_2", KEY_2);
    addKey("KEY_3", KEY_3);
    addKey("KEY_4", KEY_4);
    addKey("KEY_5", KEY_5);
    addKey("KEY_6", KEY_6);
    addKey("KEY_7", KEY_7);
    addKey("KEY_8", KEY_8);
    addKey("KEY_9", KEY_9);
    addKey("KEY_0", KEY_0);
    addKey("KEY_MINUS", KEY_MINUS);
    addKey("KEY_EQUAL", KEY_EQUAL);
    addKey("KEY_BACKSPACE", KEY_BACKSPACE);
    addKey("KEY_TAB", KEY_TAB);
    addKey("KEY_Q", KEY_Q);
    addKey("KEY_W", KEY_W);
    addKey("KEY_E", KEY_E);
    addKey("KEY_R", KEY_R);
    addKey("KEY_T", KEY_T);
    addKey("KEY_Y", KEY_Y);
    addKey("KEY_U", KEY_U);
    addKey("KEY_I", KEY_I);
    addKey("KEY_O", KEY_O);
    addKey("KEY_P", KEY_P);
    addKey("KEY_LEFTBRACE", KEY_LEFTBRACE);
    addKey("KEY_RIGHTBRACE", KEY_RIGHTBRACE);
    addKey("KEY_ENTER", KEY_ENTER);
    addKey("KEY_LEFTCTRL", KEY_LEFTCTRL);
    addKey("KEY_A", KEY_A);
    addKey("KEY_S", KEY_S);
    addKey("KEY_D", KEY_D);
    addKey("KEY_F", KEY_F);
    addKey("KEY_G", KEY_G);
    addKey("KEY_H", KEY_H);
    addKey("KEY_J", KEY_J);
    addKey("KEY_K", KEY_K);
    addKey("KEY_L", KEY_L);
    addKey("KEY_SEMICOLON", KEY_SEMICOLON);
    addKey("KEY_APOSTROPHE", KEY_APOSTROPHE);
    addKey("KEY_GRAVE", KEY_GRAVE);
    addKey("KEY_LEFTSHIFT", KEY_LEFTSHIFT);
    addKey("KEY_BACKSLASH", KEY_BACKSLASH);
    addKey("KEY_Z", KEY_Z);
    addKey("KEY_X", KEY_X);
    addKey("KEY_C", KEY_C);
    addKey("KEY_V", KEY_V);
    addKey("KEY_B", KEY_B);
    addKey("KEY_N", KEY_N);
    addKey("KEY_M", KEY_M);
    addKey("KEY_COMMA", KEY_COMMA);
    addKey("KEY_DOT", KEY_DOT);
    addKey("KEY_SLASH", KEY_SLASH);
    addKey("KEY_RIGHTSHIFT", KEY_RIGHTSHIFT);
    addKey("KEY_LEFTALT", KEY_LEFTALT);
    addKey("KEY_SPACE", KEY_SPACE);
    addKey("KEY_CAPSLOCK", KEY_CAPSLOCK);

    addKey("KEY_F1", KEY_F1);
    addKey("KEY_F2", KEY_F2);
    addKey("KEY_F3", KEY_F3);
    addKey("KEY_F4", KEY_F4);
    addKey("KEY_F5", KEY_F5);
    addKey("KEY_F6", KEY_F6);
    addKey("KEY_F7", KEY_F7);
    addKey("KEY_F8", KEY_F8);
    addKey("KEY_F9", KEY_F9);
    addKey("KEY_F10", KEY_F10);
    addKey("KEY_F11", KEY_F11);
    addKey("KEY_F12", KEY_F12);

    addKey("KEY_RIGHTCTRL", KEY_RIGHTCTRL);
    addKey("KEY_RIGHTALT", KEY_RIGHTALT);
    addKey("KEY_HOME", KEY_HOME);
    addKey("KEY_UP", KEY_UP);
    addKey("KEY_PAGEUP", KEY_PAGEUP);
    addKey("KEY_LEFT", KEY_LEFT);
    addKey("KEY_RIGHT", KEY_RIGHT);
    addKey("KEY_END", KEY_END);
    addKey("KEY_DOWN", KEY_DOWN);
    addKey("KEY_PAGEDOWN", KEY_PAGEDOWN);
    addKey("KEY_INSERT", KEY_INSERT);
    addKey("KEY_DELETE", KEY_DELETE);
    addKey("KEY_MUTE", KEY_MUTE);

    addKey("KEY_F13", KEY_F13);
    addKey("KEY_F14", KEY_F14);
    addKey("KEY_F15", KEY_F15);
    addKey("KEY_F16", KEY_F16);
    addKey("KEY_F17", KEY_F17);
    addKey("KEY_F18", KEY_F18);
    addKey("KEY_F19", KEY_F19);
    addKey("KEY_F20", KEY_F20);
    addKey("KEY_F21", KEY_F21);
    addKey("KEY_F22", KEY_F22);
    addKey("KEY_F23", KEY_F23);
    addKey("KEY_F24", KEY_F24);
#endif

    s_mapsInitialized = true;
}

int UInputChannel::keyCodeFromName(const QString &name)
{
    initKeyMaps();
    return s_nameToCode.value(name, 0);
}

QString UInputChannel::nameFromKeyCode(int code)
{
    initKeyMaps();
    return s_codeToName.value(code, QString("KEY_%1").arg(code));
}

QString UInputChannel::nameFromQtKey(int qtKey, quint32 nativeScanCode)
{
    initKeyMaps();

    if (nativeScanCode > 0) {
        // Under Linux X11, nativeScanCode is evdev + 8; under Wayland it is often evdev directly.
        if (s_codeToName.contains(static_cast<int>(nativeScanCode))) {
            return s_codeToName.value(static_cast<int>(nativeScanCode));
        }
        if (nativeScanCode >= 8 && s_codeToName.contains(static_cast<int>(nativeScanCode - 8))) {
            return s_codeToName.value(static_cast<int>(nativeScanCode - 8));
        }
    }

    switch (qtKey) {
    case Qt::Key_A: return "KEY_A";
    case Qt::Key_B: return "KEY_B";
    case Qt::Key_C: return "KEY_C";
    case Qt::Key_D: return "KEY_D";
    case Qt::Key_E: return "KEY_E";
    case Qt::Key_F: return "KEY_F";
    case Qt::Key_G: return "KEY_G";
    case Qt::Key_H: return "KEY_H";
    case Qt::Key_I: return "KEY_I";
    case Qt::Key_J: return "KEY_J";
    case Qt::Key_K: return "KEY_K";
    case Qt::Key_L: return "KEY_L";
    case Qt::Key_M: return "KEY_M";
    case Qt::Key_N: return "KEY_N";
    case Qt::Key_O: return "KEY_O";
    case Qt::Key_P: return "KEY_P";
    case Qt::Key_Q: return "KEY_Q";
    case Qt::Key_R: return "KEY_R";
    case Qt::Key_S: return "KEY_S";
    case Qt::Key_T: return "KEY_T";
    case Qt::Key_U: return "KEY_U";
    case Qt::Key_V: return "KEY_V";
    case Qt::Key_W: return "KEY_W";
    case Qt::Key_X: return "KEY_X";
    case Qt::Key_Y: return "KEY_Y";
    case Qt::Key_Z: return "KEY_Z";

    case Qt::Key_0: return "KEY_0";
    case Qt::Key_1: return "KEY_1";
    case Qt::Key_2: return "KEY_2";
    case Qt::Key_3: return "KEY_3";
    case Qt::Key_4: return "KEY_4";
    case Qt::Key_5: return "KEY_5";
    case Qt::Key_6: return "KEY_6";
    case Qt::Key_7: return "KEY_7";
    case Qt::Key_8: return "KEY_8";
    case Qt::Key_9: return "KEY_9";

    case Qt::Key_Space: return "KEY_SPACE";
    case Qt::Key_Period: return "KEY_DOT";
    case Qt::Key_Comma: return "KEY_COMMA";
    case Qt::Key_Slash: return "KEY_SLASH";
    case Qt::Key_Minus: return "KEY_MINUS";
    case Qt::Key_Equal: return "KEY_EQUAL";
    case Qt::Key_Semicolon: return "KEY_SEMICOLON";
    case Qt::Key_Apostrophe: return "KEY_APOSTROPHE";
    case Qt::Key_Tab: return "KEY_TAB";
    case Qt::Key_Return:
    case Qt::Key_Enter: return "KEY_ENTER";
    case Qt::Key_Escape: return "KEY_ESC";
    case Qt::Key_CapsLock: return "KEY_CAPSLOCK";

    case Qt::Key_F1: return "KEY_F1";
    case Qt::Key_F2: return "KEY_F2";
    case Qt::Key_F3: return "KEY_F3";
    case Qt::Key_F4: return "KEY_F4";
    case Qt::Key_F5: return "KEY_F5";
    case Qt::Key_F6: return "KEY_F6";
    case Qt::Key_F7: return "KEY_F7";
    case Qt::Key_F8: return "KEY_F8";
    case Qt::Key_F9: return "KEY_F9";
    case Qt::Key_F10: return "KEY_F10";
    case Qt::Key_F11: return "KEY_F11";
    case Qt::Key_F12: return "KEY_F12";
    case Qt::Key_F13: return "KEY_F13";
    case Qt::Key_F14: return "KEY_F14";
    case Qt::Key_F15: return "KEY_F15";
    case Qt::Key_F16: return "KEY_F16";
    case Qt::Key_F17: return "KEY_F17";
    case Qt::Key_F18: return "KEY_F18";
    case Qt::Key_F19: return "KEY_F19";
    case Qt::Key_F20: return "KEY_F20";
    case Qt::Key_F21: return "KEY_F21";
    case Qt::Key_F22: return "KEY_F22";
    case Qt::Key_F23: return "KEY_F23";
    case Qt::Key_F24: return "KEY_F24";

    case Qt::Key_Control: return "KEY_LEFTCTRL";
    case Qt::Key_Shift: return "KEY_LEFTSHIFT";
    case Qt::Key_Alt: return "KEY_LEFTALT";
    default:
        break;
    }

    return "KEY_V";
}

bool UInputChannel::openDevice()
{
#ifdef __linux__
    if (m_fd >= 0)
        return true;

    m_fd = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (m_fd < 0) {
        m_isAvailable = false;
        if (errno == EACCES || errno == EPERM) {
            m_statusMessage = tr("/dev/uinput permission denied. Add user to 'input' group or check uaccess permissions.");
        } else if (errno == ENOENT) {
            m_statusMessage = tr("/dev/uinput device not found.");
        } else {
            m_statusMessage = tr("Cannot open /dev/uinput (%1)").arg(QString::fromLocal8Bit(strerror(errno)));
        }
        return false;
    }

    // Configure virtual keyboard capabilities
    if (::ioctl(m_fd, UI_SET_EVBIT, EV_KEY) < 0 ||
        ::ioctl(m_fd, UI_SET_EVBIT, EV_SYN) < 0 ||
        ::ioctl(m_fd, UI_SET_EVBIT, EV_REP) < 0) {
        m_statusMessage = tr("Failed to set uinput event bits.");
        closeDevice();
        return false;
    }

    // Enable standard keyboard keys 1..248
    for (int k = 1; k < 250; ++k) {
        ::ioctl(m_fd, UI_SET_KEYBIT, k);
    }

    struct uinput_setup usetup;
    std::memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1A2B;
    usetup.id.product = 0x3C4D;
    std::strncpy(usetup.name, "Caster Soundboard Virtual Keyboard", UINPUT_MAX_NAME_SIZE - 1);

    if (::ioctl(m_fd, UI_DEV_SETUP, &usetup) < 0 ||
        ::ioctl(m_fd, UI_DEV_CREATE) < 0) {
        m_statusMessage = tr("Failed to create uinput device.");
        closeDevice();
        return false;
    }

    m_isAvailable = true;
    m_statusMessage = tr("Linux uinput virtual keyboard active.");
    return true;
#else
    m_isAvailable = false;
    m_statusMessage = tr("uinput is only supported on Linux.");
    return false;
#endif
}

void UInputChannel::closeDevice()
{
#ifdef __linux__
    if (m_fd >= 0) {
        ::ioctl(m_fd, UI_DEV_DESTROY);
        ::close(m_fd);
        m_fd = -1;
    }
#endif
    m_isAvailable = false;
}

void UInputChannel::emitRawEvent(int type, int code, int value)
{
#ifdef __linux__
    if (m_fd < 0 || code < 0)
        return;

    struct input_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.type = static_cast<__u16>(type);
    ev.code = static_cast<__u16>(code);
    ev.value = static_cast<__s32>(value);

    ssize_t n = ::write(m_fd, &ev, sizeof(ev));
    if (n < 0) {
        qWarning().noquote() << QString("[Actions] uinput write failed: %1").arg(strerror(errno));
    }
#else
    Q_UNUSED(type);
    Q_UNUSED(code);
    Q_UNUSED(value);
#endif
}

void UInputChannel::syncEvent()
{
#ifdef __linux__
    emitRawEvent(EV_SYN, SYN_REPORT, 0);
#endif
}

void UInputChannel::sendKeyDown(const QString &keyName)
{
#ifdef __linux__
    int code = keyCodeFromName(keyName);
    if (code > 0) {
        qInfo().noquote() << QString("[Actions] Virtual key DOWN: %1 (Linux code %2)").arg(keyName).arg(code);
        emitRawEvent(EV_KEY, code, 1);
        syncEvent();
    } else {
        qWarning().noquote() << QString("[Actions] Virtual key DOWN failed: unknown key '%1'").arg(keyName);
    }
#else
    Q_UNUSED(keyName);
#endif
}

void UInputChannel::sendKeyUp(const QString &keyName)
{
#ifdef __linux__
    int code = keyCodeFromName(keyName);
    if (code > 0) {
        qInfo().noquote() << QString("[Actions] Virtual key UP: %1 (Linux code %2)").arg(keyName).arg(code);
        emitRawEvent(EV_KEY, code, 0);
        syncEvent();
    }
#else
    Q_UNUSED(keyName);
#endif
}

void UInputChannel::sendKeyTap(const QString &keyName, int tapDurationMs)
{
    qInfo().noquote() << QString("[Actions] Virtual key TAP: %1 for %2 ms").arg(keyName).arg(tapDurationMs);
    sendKeyDown(keyName);
    QTimer::singleShot(tapDurationMs, this, [this, keyName]() {
        sendKeyUp(keyName);
    });
}
