#include "AudioDevicesDialog.h"
#include <QComboBox>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextBrowser>
namespace Audio {
AudioDevicesDialog::AudioDevicesDialog(AudioDeviceController *c, QWidget *parent) : QDialog(parent), controller(c) {
    setWindowTitle(tr("Audio Devices")); setModal(false); resize(760, 780);
    auto rootLayout = new QVBoxLayout(this);
    setupHelp = new QTextBrowser;
    setupHelp->setObjectName("audioSetupHelp");
    setupHelp->setOpenExternalLinks(true);
    setupHelp->setMinimumHeight(260);
    rootLayout->addWidget(setupHelp, 1);
    deviceControls = new QWidget;
    deviceControls->setObjectName("audioDeviceControls");
    rootLayout->addWidget(deviceControls);
    auto layout = new QVBoxLayout(deviceControls);
    layout->setContentsMargins(0, 0, 0, 0);
    auto section = [&layout](const QString &title) {
        auto line = new QFrame;
        line->setFrameShape(QFrame::HLine); line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
        auto heading = new QLabel(title);
        auto font = heading->font(); font.setBold(true); heading->setFont(font);
        layout->addWidget(heading);
    };
    section(tr("Devices"));
    auto form = new QFormLayout; form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow); microphone = new QComboBox; systemOutput = new QComboBox; monitorOutput = new QComboBox;
    for (auto combo : {microphone, systemOutput, monitorOutput}) { combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); combo->setMinimumContentsLength(20); combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); }
    form->addRow(tr("Physical microphone"), microphone); form->addRow(tr("System default output (global)"), systemOutput);
    form->addRow(tr("Soundboard monitor output"), monitorOutput); layout->addLayout(form);
    section(tr("Application routing"));
    auto appHint = new QLabel(tr("In the receiving application, choose Default/System Default to follow the system input and output. To use the mixed input this way, enable the first and third options below, then Apply. If the application keeps using its previous device, reselect its input or restart its audio session.")); appHint->setWordWrap(true); layout->addWidget(appHint);
    mix = new QCheckBox(tr("Send microphone + soundboard to a virtual microphone"));
    monitor = new QCheckBox(tr("Monitor soundboard through chosen output (never the live microphone)"));
    defaultInput = new QCheckBox(tr("Use mixed virtual microphone as system default input (global)"));
    auto explain = [layout](QCheckBox *option, const QString &text) {
        layout->addWidget(option);
        auto hint = new QLabel(text); hint->setWordWrap(true);
        hint->setContentsMargins(22, 0, 0, 6); layout->addWidget(hint);
        return hint;
    };
    mixHint = explain(mix, tr("Combines your microphone and soundboard into one input for another application. Example: record your narration and sound effects together in a recording application."));
    monitorHint = explain(monitor, tr("Plays soundboard audio through the selected monitor output without playing your live microphone back to you. Example: hear sound effects in your headphones while sending them to another application."));
    defaultInputHint = explain(defaultInput, tr("Makes the mixed microphone the system input for applications using Default/System Default. Example: a recording application using the default input receives both your microphone and soundboard. Applications with an explicitly selected input keep that selection."));
    section(tr("Device recovery"));
    recoveryDevice = new QComboBox;
    recoveryDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    recoveryDevice->setMinimumContentsLength(20);
    recoveryDevice->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    auto recoveryForm = new QFormLayout; recoveryForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    recoveryForm->addRow(tr("Device to recover"), recoveryDevice); layout->addLayout(recoveryForm);
    recoveryHint = new QLabel(tr("Recovery can enable disabled devices or recreate missing endpoints. An idle/suspended device is not necessarily broken."));
    recoveryHint->setWordWrap(true); layout->addWidget(recoveryHint);
    // Keep recovery and retry available even when the device controls cannot load.
    layout = rootLayout;
    auto recoveryButtons = new QHBoxLayout;
    recover = new QPushButton(tr("Recover selected device…")); restart = new QPushButton(tr("Restart desktop audio…"));
    recoveryButtons->addWidget(recover); recoveryButtons->addWidget(restart); recoveryButtons->addStretch(); layout->addLayout(recoveryButtons);
    connect(recoveryDevice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { updateEnabled(); });
    connect(recover, &QPushButton::clicked, this, [this] {
        Plan p; p.metadata["recoveryEndpoint"] = recoveryDevice->currentData();
        controller->request(AudioDeviceController::RecoveryPreview, Configuration(), p);
    });
    connect(restart, &QPushButton::clicked, this, [this] { controller->request(AudioDeviceController::RestartPreview); });
    auto confirmRecovery = [this](Plan p, bool services) {
        if (!p.error.isEmpty()) { appendDiagnostics(tr("Recovery error"), p.error); return; }
        appendDiagnostics(services ? tr("Planned audio restart") : tr("Planned device recovery"), p.changes.join('\n'));
        QMessageBox box(QMessageBox::Warning, services ? tr("Restart ALL desktop audio?") : tr("Recover selected device?"), p.changes.join('\n'), QMessageBox::Cancel, this);
        auto confirm = box.addButton(services ? tr("Restart all audio") : tr("Recover device"), QMessageBox::AcceptRole);
        box.setDefaultButton(QMessageBox::Cancel); box.exec();
        if (box.clickedButton() == confirm) controller->request(services ? AudioDeviceController::Restart : AudioDeviceController::Recover, Configuration(), p);
    };
    connect(c, &AudioDeviceController::recoveryPreviewReady, this, [confirmRecovery](Plan p) { confirmRecovery(p, false); });
    connect(c, &AudioDeviceController::restartPreviewReady, this, [confirmRecovery](Plan p) { confirmRecovery(p, true); });
    section(tr("Diagnostics"));
    details = new QPlainTextEdit; details->setMinimumHeight(140); details->setReadOnly(true); details->setPlaceholderText(tr("Device IDs, planned changes and troubleshooting details")); layout->addWidget(details);
    auto buttons = new QHBoxLayout;
    refresh = new QPushButton(tr("Refresh devices"));
    refresh->setToolTip(tr("Scan again for microphones and speakers and update their status.\n"
                          "Use this after connecting or disconnecting a device. This does not apply your settings."));
    refresh->setAccessibleDescription(refresh->toolTip());
    stop = new QPushButton(tr("Stop audio sharing"));
    stop->setToolTip(tr("Turn off the microphone + soundboard mix and soundboard monitoring set up here.\n"
                       "Restore the previous default microphone if the mixed input is still selected.\n"
                       "Keep the chosen system output and device profiles. Use Apply to enable sharing again."));
    stop->setAccessibleDescription(stop->toolTip());
    auto exitButton = new QPushButton(tr("Exit"));
    exitButton->setObjectName("audioDevicesExitButton");
    exitButton->setToolTip(tr("Close this window without applying pending selections. Audio settings already applied remain active."));
    auto helpButton = new QPushButton(tr("Help"));
    helpButton->setObjectName("audioDevicesHelpButton");
    helpButton->setToolTip(tr("Show audio setup requirements, installation examples and distribution guides."));
    apply = new QPushButton(tr("Apply"));
    buttons->addWidget(refresh); buttons->addWidget(stop); buttons->addStretch();
    buttons->addWidget(apply); buttons->addWidget(exitButton); buttons->addWidget(helpButton);
    layout->addLayout(buttons);
    connect(exitButton, &QPushButton::clicked, this, &QDialog::reject);

    auto helpDialog = new QDialog(this);
    helpDialog->setObjectName("audioDevicesHelpDialog");
    helpDialog->setWindowTitle(tr("Audio Devices Help"));
    helpDialog->resize(760, 650);
    auto helpLayout = new QVBoxLayout(helpDialog);
    auto guide = new QTextBrowser;
    guide->setOpenExternalLinks(true);
    guide->setSource(QUrl("qrc:/audio/device-setup.html"));
    helpLayout->addWidget(guide);
    auto closeHelp = new QPushButton(tr("Exit"));
    helpLayout->addWidget(closeHelp, 0, Qt::AlignRight);
    connect(closeHelp, &QPushButton::clicked, helpDialog, &QDialog::reject);
    connect(this, &QDialog::finished, helpDialog, &QDialog::reject);
    connect(helpButton, &QPushButton::clicked, helpDialog, [helpDialog] {
        helpDialog->show(); helpDialog->raise(); helpDialog->activateWindow();
    });
    connect(refresh, &QPushButton::clicked, this, [this] { controller->request(AudioDeviceController::Refresh); });
    connect(stop, &QPushButton::clicked, this, [this] { controller->request(AudioDeviceController::Stop); });
    connect(apply, &QPushButton::clicked, this, [this] { controller->request(AudioDeviceController::Preview, staged()); });
    connect(mix, &QCheckBox::toggled, this, &AudioDevicesDialog::updateEnabled);
    connect(monitor, &QCheckBox::toggled, this, &AudioDevicesDialog::updateEnabled);
    connect(c, &AudioDeviceController::devicesChanged, this, &AudioDevicesDialog::populate);
    connect(c, &AudioDeviceController::busyChanged, this, [this](bool b) { busy = b; updateEnabled(); });
    connect(c, &AudioDeviceController::previewReady, this, [this](Plan plan) {
        if (!plan.error.isEmpty()) { appendDiagnostics(tr("Configuration error"), plan.error); return; }
        appendDiagnostics(tr("Planned audio changes"), plan.changes.join('\n'));
        if (QMessageBox::question(this, tr("Apply audio changes?"), plan.changes.join('\n') + tr("\n\nApply these changes now?"), QMessageBox::Apply | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Apply)
            controller->request(AudioDeviceController::Apply, plan.desired, plan);
    });
    connect(c, &AudioDeviceController::resultReady, this, [this](Result r) {
        const auto message = r.message + (r.voiceSource.isEmpty() ? QString() : tr("\nMixed input for applications: ") + r.voiceSource);
        appendDiagnostics(tr("Operation result"), message);
    });
    populate(c->devices());
}
void AudioDevicesDialog::appendDiagnostics(const QString &title, const QString &text) {
    if (text.trimmed().isEmpty()) return;
    const auto block = title + "\n----------------------------------------\n" + text.trimmed();
    if (details->toPlainText().endsWith(block)) return;
    details->appendPlainText((details->document()->isEmpty() ? QString() : QString("\n")) + block);
}
Configuration AudioDevicesDialog::staged() const {
    Configuration c; c.microphone = microphone->currentData().toString(); c.systemOutput = systemOutput->currentData().toString();
    c.monitorOutput = monitorOutput->currentData().toString(); c.mix = mix->isChecked() && capabilities.virtualSinks && capabilities.loopbacks && capabilities.streamRetargeting; c.monitor = monitor->isChecked() && capabilities.virtualSinks && capabilities.loopbacks && capabilities.streamRetargeting; c.defaultInput = defaultInput->isChecked() && capabilities.defaults; return c;
}
void AudioDevicesDialog::openSettings() {
    auto saved = controller->saved();
    auto select = [](QComboBox *box, const QString &id) {
        int i = box->findData(id); if (i < 0 && !id.isEmpty()) { box->addItem("Missing/disconnected — " + id, id); i = box->count() - 1; }
        box->setCurrentIndex(i < 0 ? 0 : i);
    };
    select(microphone, saved.microphone); select(systemOutput, saved.systemOutput); select(monitorOutput, saved.monitorOutput);
    mix->setChecked(saved.mix); monitor->setChecked(saved.monitor); defaultInput->setChecked(saved.defaultInput);
    show(); raise(); activateWindow(); controller->request(AudioDeviceController::Refresh);
}
void AudioDevicesDialog::populate(const Discovery &d) {
    capabilities = d.capabilities;
    mix->setVisible(capabilities.virtualSinks && capabilities.loopbacks && capabilities.streamRetargeting);
    monitor->setVisible(capabilities.virtualSinks && capabilities.loopbacks && capabilities.streamRetargeting);
    defaultInput->setVisible(capabilities.defaults && capabilities.virtualSinks && capabilities.loopbacks);
    mixHint->setVisible(!mix->isHidden());
    monitorHint->setVisible(!monitor->isHidden());
    defaultInputHint->setVisible(!defaultInput->isHidden());
    ready = d.error.isEmpty() && d.capabilities.defaults;
    deviceControls->setVisible(ready);
    recover->setVisible(ready);
    setupHelp->setVisible(!ready);
    if (!ready) {
        if (d.backend.isEmpty() && d.error.isEmpty()) {
            setupHelp->setHtml(tr("<h2>Checking audio devices…</h2><p>Please wait while CasterSoundboard checks the audio service.</p>"));
        } else {
#ifdef Q_OS_LINUX
            const QUrl guide("qrc:/audio/device-setup.html");
            if (setupHelp->source() != guide) setupHelp->setSource(guide);
#else
            setupHelp->setHtml(tr("<h2>Audio device management is available on Linux</h2>"
                                 "<p>This panel needs pactl and a PulseAudio-compatible audio service. "
                                 "Use the Output selector in the main window for normal playback on this system.</p>"));
#endif
        }
    }
    appendDiagnostics(tr("Audio backend"), d.backend);
    appendDiagnostics(tr("System status and devices"), d.status);
    appendDiagnostics(tr("Discovery error"), d.error);
    auto fill = [&](QComboBox *box, bool input) {
        const QString selected = box->currentData().toString(); box->clear(); box->addItem(tr("Choose a device…"), QString());
        for (auto e : d.endpoints) if (e.input == input) {
            QString suffix = !e.healthWarning.isEmpty() ? tr(" — needs attention") : !e.available ? tr(" — reported unavailable") : e.active ? tr(" — active") : tr(" — available after profile/port change");
            box->addItem(e.label + suffix, e.id); box->setItemData(box->count() - 1, e.id + (e.healthWarning.isEmpty() ? QString() : "\n" + e.healthWarning), Qt::ToolTipRole);
        }
        int index = box->findData(selected);
        if (index < 0 && !selected.isEmpty()) { box->addItem(tr("Missing/disconnected — ") + selected, selected); index = box->count() - 1; }
        box->setCurrentIndex(index < 0 ? 0 : index);
    };
    fill(microphone, true); fill(systemOutput, false); fill(monitorOutput, false);
    const auto selected = recoveryDevice->currentData().toString(); recoveryDevice->clear();
    recoveryDevice->addItem(tr("Choose a device to enable or reset…"), QString());
    for (auto e : d.endpoints) if (e.recoverySupported) {
        recoveryDevice->addItem(e.label + (e.input ? tr(" (input)") : tr(" (output)")) +
            (e.healthWarning.isEmpty() ? QString() : tr(" — needs attention")), e.id);
        recoveryDevice->setItemData(recoveryDevice->count() - 1, e.healthWarning + "\n" + e.id, Qt::ToolTipRole);
        recoveryDevice->setItemData(recoveryDevice->count() - 1, e.healthWarning, Qt::UserRole + 1);
    }
    int selectedIndex = recoveryDevice->findData(selected);
    if (selectedIndex < 0 && !selected.isEmpty()) {
        recoveryDevice->addItem(tr("Missing/disconnected — ") + selected, selected);
        selectedIndex = recoveryDevice->count() - 1;
        recoveryDevice->setItemData(selectedIndex, tr("This card is missing. Reconnect it and refresh devices before attempting recovery."), Qt::UserRole + 1);
    }
    recoveryDevice->setCurrentIndex(selectedIndex < 0 ? 0 : selectedIndex);
    for (auto box : {microphone, systemOutput, monitorOutput, recoveryDevice}) {
        int width = 0;
        for (int i = 0; i < box->count(); ++i) {
            width = qMax(width, box->fontMetrics().horizontalAdvance(box->itemText(i)) + 40);
            box->setItemData(i, box->itemText(i) + "\n" + box->itemData(i, Qt::ToolTipRole).toString(), Qt::ToolTipRole);
        }
        box->view()->setMinimumWidth(width);
    }
    restart->setVisible(capabilities.serviceRestart);
    updateEnabled();
}
void AudioDevicesDialog::updateEnabled() {
    recoveryDevice->setEnabled(ready && !busy);
    recover->setEnabled(ready && !busy && !recoveryDevice->currentData().toString().isEmpty());
    restart->setEnabled(capabilities.serviceRestart && !busy);
    const auto health = recoveryDevice->currentData(Qt::UserRole + 1).toString();
    recoveryHint->setText(health.isEmpty() ? tr("Recovery can enable disabled devices or recreate missing endpoints. An idle/suspended device is not necessarily broken.") : health);
    apply->setEnabled(ready && !busy); refresh->setEnabled(!busy); stop->setEnabled(!busy);
    systemOutput->setEnabled(ready && !busy); mix->setEnabled(ready && !busy); monitor->setEnabled(ready && !busy);
    microphone->setEnabled(ready && !busy && mix->isChecked()); monitorOutput->setEnabled(ready && !busy && monitor->isChecked());
    defaultInput->setEnabled(ready && !busy && mix->isChecked());
}
}
