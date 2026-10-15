import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    component DarkTextField : TextField {
        id: dtf
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        implicitHeight: 44
        background: Rectangle {
            color: "#282C32"
            radius: 6
            border.color: dtf.activeFocus ? "#8E2DE2" : "#454B54"
            border.width: dtf.activeFocus ? 2 : 1
        }
    }

    component DarkComboBox : ComboBox {
        id: dcb
        implicitHeight: 44
        contentItem: Text {
            leftPadding: 12
            rightPadding: dcb.indicator.width + 12
            text: dcb.displayText
            font.pointSize: 10
            color: "#FFFFFF"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: "#282C32"
            radius: 6
            border.color: dcb.activeFocus ? "#8E2DE2" : "#454B54"
            border.width: 1
        }
        popup: Popup {
            y: dcb.height + 2
            width: dcb.width
            implicitHeight: Math.min(300, contentItem.implicitHeight + 10)
            padding: 4
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: dcb.popup.visible ? dcb.delegateModel : null
                currentIndex: dcb.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }
            background: Rectangle {
                color: "#282C32"
                radius: 6
                border.color: "#454B54"
                border.width: 1
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: contentCol.implicitHeight + 40
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contentCol
                width: flickable.width - 12
                spacing: 20

                // 1. Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/audio_devices.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Audio Devices & Virtual Microphone")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Configure playback output destination and voice chat virtual microphone piping for Discord, OBS, or Zoom.")
                            color: "#888888"
                            font.pointSize: 10
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#33373B"
                }

                // 2. Status Banner & Quick Action Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(88, statusRow.implicitHeight + 32)
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        id: statusRow
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: {
                                if (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                    return "#2ECC71"; // Active Green
                                if (typeof audioRoutingController !== "undefined" && audioRoutingController.isBusy)
                                    return "#F39C12"; // Busy Amber
                                return "#7E8590"; // Inactive Gray
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: {
                                    if (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                        return qsTr("Virtual Audio Routing Active");
                                    if (typeof audioRoutingController !== "undefined" && audioRoutingController.isBusy)
                                        return qsTr("Audio System Busy");
                                    return qsTr("Virtual Audio Routing Inactive");
                                }
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 13
                            }
                            Text {
                                text: (typeof audioRoutingController !== "undefined" && audioRoutingController.statusMessage.length > 0)
                                      ? audioRoutingController.statusMessage
                                      : qsTr("Ready — changes are applied only with Apply")
                                color: "#AAAAAA"
                                font.pointSize: 10
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            id: stopBtn
                            text: qsTr("Stop Audio Sharing")
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 44
                            visible: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                            onClicked: {
                                if (typeof audioRoutingController !== "undefined") {
                                    audioRoutingController.stopSharing();
                                }
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#922B21" : (parent.hovered ? "#C0392B" : "#A93226")
                                radius: 6
                            }
                        }

                        Button {
                            id: applyBtn
                            text: qsTr("Apply Routing")
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 44
                            enabled: typeof audioRoutingController !== "undefined" && !audioRoutingController.isBusy
                            onClicked: {
                                if (typeof audioRoutingController !== "undefined") {
                                    audioRoutingController.apply();
                                }
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#5B18A7" : (parent.hovered ? "#A044FF" : "#8E2DE2")
                                radius: 6
                                opacity: applyBtn.enabled ? 1.0 : 0.5
                            }
                        }
                    }
                }

                // 3. Section 1: Endpoints & Physical Hardware Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: endpointsCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: endpointsCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Audio Endpoints")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 13
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                text: qsTr("Refresh Devices")
                                Layout.preferredHeight: 38
                                onClicked: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.refresh();
                                    }
                                    if (typeof projectController !== "undefined") {
                                        projectController.refreshAudioDevices();
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                    radius: 6
                                    border.color: "#454B54"
                                    border.width: 1
                                }
                            }
                        }

                        // Physical microphone
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: qsTr("Physical Microphone:")
                                color: "#AAAAAA"
                                font.pointSize: 10
                            }
                            DarkComboBox {
                                id: micCombo
                                Layout.fillWidth: true
                                model: (typeof audioRoutingController !== "undefined" && audioRoutingController.inputDevices.length > 0)
                                       ? audioRoutingController.inputDevices
                                       : [qsTr("None / Disconnected")]
                                currentIndex: {
                                    if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedMicrophone)
                                        return 0;
                                    var idx = micCombo.model.indexOf(audioRoutingController.selectedMicrophone);
                                    return idx >= 0 ? idx : 0;
                                }
                                onActivated: (index) => {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.selectedMicrophone = currentText;
                                    }
                                }
                            }
                        }

                        // System default output (global)
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: qsTr("System Default Output (Global):")
                                color: "#AAAAAA"
                                font.pointSize: 10
                            }
                            DarkComboBox {
                                id: sysOutCombo
                                Layout.fillWidth: true
                                model: (typeof audioRoutingController !== "undefined" && audioRoutingController.outputDevices.length > 0)
                                       ? audioRoutingController.outputDevices
                                       : [qsTr("Default System Output")]
                                currentIndex: {
                                    if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedSystemOutput)
                                        return 0;
                                    var idx = sysOutCombo.model.indexOf(audioRoutingController.selectedSystemOutput);
                                    return idx >= 0 ? idx : 0;
                                }
                                onActivated: (index) => {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.selectedSystemOutput = currentText;
                                    }
                                }
                            }
                        }

                        // Soundboard monitor output
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: qsTr("Soundboard Monitor Output (Headphones):")
                                color: "#AAAAAA"
                                font.pointSize: 10
                            }
                            DarkComboBox {
                                id: monOutCombo
                                Layout.fillWidth: true
                                model: (typeof audioRoutingController !== "undefined" && audioRoutingController.outputDevices.length > 0)
                                       ? audioRoutingController.outputDevices
                                       : [qsTr("Default System Output")]
                                currentIndex: {
                                    if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedMonitorOutput)
                                        return 0;
                                    var idx = monOutCombo.model.indexOf(audioRoutingController.selectedMonitorOutput);
                                    return idx >= 0 ? idx : 0;
                                }
                                onActivated: (index) => {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.selectedMonitorOutput = currentText;
                                    }
                                }
                            }
                        }
                    }
                }

                // 4. Section 2: Application Routing & Virtual Mic Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: routingCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: routingCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        Text {
                            text: qsTr("Application Routing & Virtual Microphone")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        Text {
                            text: qsTr("In the receiving application (Discord, OBS, Zoom, Audacity, etc.), choose the Virtual Microphone as input. If the application keeps using its previous device, reselect its input or restart its audio session.")
                            color: "#888888"
                            font.pointSize: 9
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        // Checkbox 1: Mix physical mic with soundboard
                        RowLayout {
                            spacing: 12
                            CheckBox {
                                id: mixCheck
                                checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.mixMicrophone : true
                                onToggled: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.mixMicrophone = checked;
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Send microphone + soundboard to a virtual microphone")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Text {
                                    text: qsTr("Combines your physical microphone and soundboard audio into one virtual input for voice chat or recording.")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        // Virtual microphone name
                        RowLayout {
                            spacing: 12
                            Layout.leftMargin: 40
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Virtual microphone name:")
                                color: "#CCCCCC"
                                font.pointSize: 10
                            }
                            DarkTextField {
                                id: micNameField
                                text: typeof audioRoutingController !== "undefined" ? audioRoutingController.virtualMicName : "Caster Soundboard Mic"
                                placeholderText: qsTr("Virtual_Microphone")
                                Layout.preferredWidth: 260
                                onEditingFinished: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.virtualMicName = text;
                                    }
                                }
                            }
                        }

                        // Checkbox 2: Monitor soundboard through chosen output
                        RowLayout {
                            spacing: 12
                            CheckBox {
                                id: monCheck
                                checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.monitorSoundboard : true
                                onToggled: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.monitorSoundboard = checked;
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Monitor soundboard through chosen output (never the live microphone)")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Text {
                                    text: qsTr("Plays soundboard audio through your monitor output without playing your live voice back to your headphones.")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        // Checkbox 3: Set as system default input
                        RowLayout {
                            spacing: 12
                            CheckBox {
                                id: defInputCheck
                                checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.useAsDefaultInput : false
                                onToggled: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.useAsDefaultInput = checked;
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Use mixed virtual microphone as system default input (global)")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Text {
                                    text: qsTr("Makes the mixed microphone the system input for applications using Default/System Default.")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                // 5. Section 3: Device Recovery & Troubleshooting Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: recoveryCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: recoveryCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        Text {
                            text: qsTr("Device Recovery & Troubleshooting")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        Text {
                            text: qsTr("Recovery can enable disabled devices or recreate missing endpoints. An idle or suspended device is not necessarily broken.")
                            color: "#888888"
                            font.pointSize: 9
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            DarkComboBox {
                                id: recCombo
                                Layout.fillWidth: true
                                model: (typeof audioRoutingController !== "undefined" && audioRoutingController.recoveryDevices.length > 0)
                                       ? audioRoutingController.recoveryDevices
                                       : [qsTr("Choose a device to enable or reset...")]
                                currentIndex: {
                                    if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedRecoveryDevice)
                                        return 0;
                                    var idx = recCombo.model.indexOf(audioRoutingController.selectedRecoveryDevice);
                                    return idx >= 0 ? idx : 0;
                                }
                                onActivated: (index) => {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.selectedRecoveryDevice = currentText;
                                    }
                                }
                            }

                            Button {
                                text: qsTr("Recover Device…")
                                Layout.preferredHeight: 44
                                onClicked: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.recoverSelectedDevice();
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                    radius: 6
                                    border.color: "#454B54"
                                    border.width: 1
                                }
                            }

                            Button {
                                text: qsTr("Restart Desktop Audio…")
                                Layout.preferredHeight: 44
                                onClicked: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.restartDesktopAudio();
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                    radius: 6
                                    border.color: "#454B54"
                                    border.width: 1
                                }
                            }
                        }
                    }
                }

                // 6. Section 4: Diagnostics Log Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: diagCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: diagCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        Text {
                            text: qsTr("Diagnostics Log")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            color: "#181A1B"
                            radius: 6
                            border.color: "#33373B"
                            border.width: 1

                            ScrollView {
                                anchors.fill: parent
                                anchors.margins: 8
                                clip: true

                                TextArea {
                                    id: diagText
                                    readOnly: true
                                    color: "#74B9FF"
                                    font.family: "Monospace"
                                    font.pointSize: 9
                                    wrapMode: Text.WrapAnywhere
                                    text: typeof audioRoutingController !== "undefined" ? audioRoutingController.diagnosticsLog : qsTr("Diagnostics ready.")
                                    background: null
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
