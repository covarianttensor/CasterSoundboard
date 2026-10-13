#ifndef CASTER_AUDIO_DIALOG_H
#define CASTER_AUDIO_DIALOG_H
#include "AudioDeviceController.h"
#include <QDialog>
class QComboBox; class QCheckBox; class QLabel; class QPlainTextEdit; class QPushButton;
class QTextBrowser;
namespace Audio {
class AudioDevicesDialog : public QDialog {
    Q_OBJECT
public:
    explicit AudioDevicesDialog(AudioDeviceController *, QWidget *parent = nullptr);
    void openSettings();
private:
    AudioDeviceController *controller;
    QComboBox *microphone, *systemOutput, *monitorOutput, *recoveryDevice;
    QCheckBox *mix, *monitor, *defaultInput;
    QLabel *recoveryHint, *mixHint, *monitorHint, *defaultInputHint;
    QPlainTextEdit *details;
    QWidget *deviceControls;
    QTextBrowser *setupHelp;
    QPushButton *refresh, *stop, *apply, *recover, *restart;
    bool ready = false, busy = false;
    Capabilities capabilities;
    Configuration staged() const;
    void populate(const Discovery &);
    void updateEnabled();
    void appendDiagnostics(const QString &title, const QString &text);
};
}
#endif
