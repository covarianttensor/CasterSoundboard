import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property alias tileScaleValue: scaleSlider.value

    signal newTabRequested()
    signal openProjectRequested()
    signal saveProjectRequested()
    signal saveAsProjectRequested()
    signal renameTabRequested()
    signal stopAllRequested()
    signal toggleDuckingRequested()
    signal toggleMuteHotkeysRequested()
    signal changeStopAllShortcutRequested()
    signal changeDuckingShortcutRequested()

    height: 60
    implicitHeight: 60
    Layout.preferredHeight: 60
    Layout.fillWidth: true
    color: "#25282B"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 8

        // --- Project / Profile File Controls ---
        ActionIconButton {
            iconSource: "icons/open.png"
            tooltipText: qsTr("Open .caster Project (Ctrl+O)")
            onClicked: root.openProjectRequested()
        }

        ActionIconButton {
            iconSource: "icons/save.png"
            tooltipText: qsTr("Save Project (Ctrl+S)")
            onClicked: root.saveProjectRequested()
        }

        ActionIconButton {
            iconSource: "icons/save_as.png"
            tooltipText: qsTr("Save Project As... (Ctrl+Shift+S)")
            onClicked: root.saveAsProjectRequested()
        }

        Rectangle {
            width: 1
            height: 32
            color: "#3A3E42"
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        // --- Soundboard Tab Controls ---
        ActionIconButton {
            iconSource: "icons/newTab.png"
            tooltipText: qsTr("Add Soundboard Tab (Ctrl+N)")
            onClicked: root.newTabRequested()
        }

        ActionIconButton {
            iconSource: "icons/rename.png"
            tooltipText: qsTr("Rename Current Tab")
            onClicked: root.renameTabRequested()
        }

        Rectangle {
            width: 1
            height: 32
            color: "#3A3E42"
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        // --- Tile Zoom & Scale Touch Slider ---
        RowLayout {
            spacing: 6

            Text {
                text: qsTr("Tile Zoom:")
                color: "#AAAAAA"
                font.pointSize: 10
            }

            Slider {
                id: scaleSlider
                from: 0.6
                to: 1.8
                value: (typeof projectController !== "undefined" && projectController.tileScale > 0) ? projectController.tileScale : 1.0
                stepSize: 0.05
                Layout.preferredWidth: 110

                background: Rectangle {
                    x: scaleSlider.leftPadding
                    y: scaleSlider.topPadding + scaleSlider.availableHeight / 2 - height / 2
                    implicitWidth: 110
                    implicitHeight: 6
                    width: scaleSlider.availableWidth
                    height: implicitHeight
                    radius: 3
                    color: "#3A3E42"

                    Rectangle {
                        width: scaleSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#8E2DE2"
                        radius: 3
                    }
                }

                handle: Rectangle {
                    x: scaleSlider.leftPadding + scaleSlider.visualPosition * (scaleSlider.availableWidth - width)
                    y: scaleSlider.topPadding + scaleSlider.availableHeight / 2 - height / 2
                    implicitWidth: 20
                    implicitHeight: 20
                    radius: 10
                    color: scaleSlider.pressed ? "#A558F0" : "#FFFFFF"
                    border.color: "#8E2DE2"
                    border.width: 2
                }
            }

            Text {
                text: Math.round(scaleSlider.value * 100) + "%"
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 10
                Layout.preferredWidth: 40
            }
        }

        Rectangle {
            width: 1
            height: 32
            color: "#3A3E42"
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        // --- Soundboard Audio Playback Output Selector ---
        RowLayout {
            spacing: 6

            Image {
                source: "icons/audio_devices.png"
                sourceSize.width: 16
                sourceSize.height: 16
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                fillMode: Image.PreserveAspectFit
                opacity: 0.85
            }

            Text {
                text: qsTr("Output:")
                color: "#AAAAAA"
                font.pointSize: 10
            }

            ComboBox {
                id: topOutputCombo
                Layout.preferredWidth: 190
                implicitHeight: 38

                model: (typeof projectController !== "undefined" && projectController.availableAudioOutputs.length > 0)
                       ? projectController.availableAudioOutputs
                       : [qsTr("Default System Output")]

                currentIndex: {
                    if (typeof projectController === "undefined" || !projectController.outputDeviceName)
                        return 0;
                    var idx = topOutputCombo.model.indexOf(projectController.outputDeviceName);
                    return idx >= 0 ? idx : 0;
                }

                enabled: typeof audioRoutingController === "undefined" || !audioRoutingController.isRouting

                onActivated: (index) => {
                    if (typeof projectController !== "undefined") {
                        projectController.outputDeviceName = currentText;
                    }
                }

                contentItem: Text {
                    leftPadding: 10
                    rightPadding: topOutputCombo.indicator.width + 10
                    text: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                          ? qsTr("Virtual Sink (Managed)")
                          : topOutputCombo.displayText
                    font.pointSize: 9
                    color: topOutputCombo.enabled ? "#FFFFFF" : "#2ECC71"
                    font.bold: !topOutputCombo.enabled
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    color: topOutputCombo.enabled ? "#2E3236" : "#1C2D24"
                    radius: 6
                    border.color: topOutputCombo.enabled ? (topOutputCombo.hovered ? "#4A4E52" : "#3A3E42") : "#2ECC71"
                    border.width: 1
                }

                popup: Popup {
                    y: topOutputCombo.height + 2
                    width: Math.max(topOutputCombo.width, 240)
                    implicitHeight: Math.min(300, contentItem.implicitHeight + 10)
                    padding: 4
                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: topOutputCombo.popup.visible ? topOutputCombo.delegateModel : null
                        currentIndex: topOutputCombo.highlightedIndex
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                    background: Rectangle {
                        color: "#282C32"
                        radius: 6
                        border.color: "#454B54"
                        border.width: 1
                    }
                }

                ToolTip.visible: hovered
                ToolTip.text: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                              ? qsTr("Audio output is currently managed by Virtual Audio Routing (piped to virtual mic and monitor)")
                              : qsTr("Select where CasterSoundboard routes its audio playback")
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // --- Production Controls ---

        // Mute Hotkeys Toggle (Issue #76)
        Button {
            id: muteHotkeysBtn
            property bool isMuted: typeof hotkeyManager !== "undefined" ? hotkeyManager.isMuted : false
            Layout.preferredHeight: 44
            Layout.preferredWidth: isMuted ? 135 : 125
            onClicked: root.toggleMuteHotkeysRequested()

            background: Rectangle {
                color: muteHotkeysBtn.isMuted ? "#D9534F" : (muteHotkeysBtn.hovered ? "#3A3E42" : "#2E3236")
                radius: 6
                border.color: muteHotkeysBtn.isMuted ? "#FF6B6B" : "#4A4E52"
                border.width: 1
            }

            contentItem: RowLayout {
                anchors.centerIn: parent
                spacing: 6
                Image {
                    source: "icons/Key.png"
                    sourceSize.width: 18
                    sourceSize.height: 18
                    Layout.preferredWidth: 18
                    Layout.preferredHeight: 18
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.9
                }
                Text {
                    text: muteHotkeysBtn.isMuted ? qsTr("Keys Muted") : qsTr("Hotkeys Active")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 9
                }
            }

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Toggle Hotkey Listening: Mute hotkeys while typing in notes or chat (Issue #76)")
        }

        // Audio Ducking Toggle
        Button {
            id: duckingBtn
            property bool isDucking: typeof projectController !== "undefined" ? projectController.isDuckingActive : false
            Layout.preferredHeight: 44
            Layout.preferredWidth: 110
            onClicked: root.toggleDuckingRequested()

            background: Rectangle {
                color: duckingBtn.isDucking ? "#F39C12" : (duckingBtn.hovered ? "#3A3E42" : "#2E3236")
                radius: 6
                border.color: duckingBtn.isDucking ? "#F1C40F" : "#4A4E52"
                border.width: 1
            }

            contentItem: RowLayout {
                anchors.centerIn: parent
                spacing: 6
                Image {
                    source: "icons/duck.png"
                    sourceSize.width: 18
                    sourceSize.height: 18
                    Layout.preferredWidth: 18
                    Layout.preferredHeight: 18
                    fillMode: Image.PreserveAspectFit
                    opacity: duckingBtn.isDucking ? 1.0 : 0.85
                }
                Text {
                    text: qsTr("Ducking")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 10
                }
            }

            ToolTip.visible: hovered
            ToolTip.text: {
                var hk = (typeof projectController !== "undefined" && projectController.duckingHotkey && projectController.duckingHotkey.length > 0)
                         ? projectController.duckingHotkey : qsTr("None");
                return qsTr("Toggle Audio Ducking (%1)\nRight-click to configure shortcut").arg(hk);
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        duckingContextMenu.popup();
                    }
                }
            }

            Menu {
                id: duckingContextMenu
                MenuItem {
                    text: qsTr("Change Shortcut...")
                    onTriggered: root.changeDuckingShortcutRequested()
                }
                MenuItem {
                    text: qsTr("Reset to Default (Ctrl+D)")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.duckingHotkey = "Ctrl+D";
                        }
                    }
                }
                MenuItem {
                    text: qsTr("Clear Shortcut")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.duckingHotkey = "";
                        }
                    }
                }
            }
        }

        // Panic Button: Stop All Sounds
        Button {
            id: stopAllBtn
            Layout.preferredHeight: 44
            Layout.preferredWidth: 115
            onClicked: root.stopAllRequested()

            background: Rectangle {
                color: stopAllBtn.pressed ? "#A93226" : (stopAllBtn.hovered ? "#C0392B" : "#962D22")
                radius: 6
                border.color: "#E74C3C"
                border.width: 1
            }

            contentItem: RowLayout {
                anchors.centerIn: parent
                spacing: 6
                Image {
                    source: "icons/stopAll.png"
                    sourceSize.width: 18
                    sourceSize.height: 18
                    Layout.preferredWidth: 18
                    Layout.preferredHeight: 18
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.95
                }
                Text {
                    text: qsTr("Stop All")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 10
                }
            }

            ToolTip.visible: hovered
            ToolTip.text: {
                var hk = (typeof projectController !== "undefined" && projectController.stopAllHotkey && projectController.stopAllHotkey.length > 0)
                         ? projectController.stopAllHotkey : qsTr("None");
                return qsTr("Panic Button: Stop all playing sounds across all tabs (%1)\nRight-click to configure shortcut").arg(hk);
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        stopAllContextMenu.popup();
                    }
                }
            }

            Menu {
                id: stopAllContextMenu
                MenuItem {
                    text: qsTr("Change Shortcut...")
                    onTriggered: root.changeStopAllShortcutRequested()
                }
                MenuItem {
                    text: qsTr("Reset to Default (Space)")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.stopAllHotkey = "Space";
                        }
                    }
                }
                MenuItem {
                    text: qsTr("Clear Shortcut")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.stopAllHotkey = "";
                        }
                    }
                }
            }
        }
    }

    component ActionIconButton : Button {
        id: aib
        property string iconSource
        property string tooltipText

        Layout.preferredWidth: 44
        Layout.preferredHeight: 44

        background: Rectangle {
            color: aib.pressed ? "#1E2022" : (aib.hovered ? "#3A3E42" : "#2E3236")
            radius: 6
            border.color: "#3A3E42"
            border.width: 1
        }

        contentItem: Image {
            source: aib.iconSource
            anchors.centerIn: parent
            sourceSize.width: 22
            sourceSize.height: 22
            width: 22
            height: 22
            fillMode: Image.PreserveAspectFit
            opacity: aib.hovered ? 1.0 : 0.85
        }

        ToolTip.visible: hovered
        ToolTip.text: aib.tooltipText
        ToolTip.delay: 300
    }

    Connections {
        target: (typeof projectController !== "undefined") ? projectController : null
        function onTileScaleChanged() {
            if (projectController && Math.abs(scaleSlider.value - projectController.tileScale) > 0.01) {
                scaleSlider.value = projectController.tileScale;
            }
        }
    }
}
