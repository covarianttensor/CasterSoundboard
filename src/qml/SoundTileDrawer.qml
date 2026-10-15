import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    property var tile: null
    signal removeTileRequested(var tile)

    title: qsTr("Sound Tile Properties")
    modal: true
    anchors.centerIn: parent
    width: Math.min(540, parent.width - 32)
    height: Math.min(620, parent.height - 32)

    footer: Rectangle {
        height: 54
        color: "#1A1C20"
        radius: 10
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 16
            Item { Layout.fillWidth: true }
            DarkButton {
                text: qsTr("Close")
                Layout.preferredWidth: 96
                Layout.preferredHeight: 38
                onClicked: root.close()
            }
        }
    }

    background: Rectangle {
        color: "#22252A"
        radius: 10
        border.color: "#3B4048"
        border.width: 1
    }

    header: Rectangle {
        height: 52
        color: "#1A1C20"
        radius: 10

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 10

            Rectangle {
                width: 14; height: 14; radius: 7
                color: (tile && tile.color && tile.color.length > 0) ? tile.color : "#8E2DE2"
            }

            Text {
                text: tile ? (tile.displayName.length > 0 ? tile.displayName : qsTr("Sound Tile Settings")) : qsTr("Tile Settings")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 13
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }
    }

    component DarkTextField : TextField {
        Layout.minimumWidth: 50
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        background: Rectangle {
            color: "#282C32"
            radius: 6
            border.color: parent.activeFocus ? "#8E2DE2" : "#454B54"
            border.width: parent.activeFocus ? 2 : 1
        }
    }

    component DarkButton : Button {
        font.pointSize: 10
        contentItem: Text {
            text: parent.text
            font: parent.font
            color: parent.enabled ? "#FFFFFF" : "#7E8590"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
            radius: 6
            border.color: parent.down ? "#8E2DE2" : "#454B54"
            border.width: 1
        }
    }

    FileDialog {
        id: mediaFileDialog
        title: qsTr("Select Audio or Video File")
        nameFilters: [
            "Media Files (*.wav *.mp3 *.ogg *.flac *.aac *.m4a *.mp4 *.mkv *.webm *.avi)",
            "Audio Files (*.wav *.mp3 *.ogg *.flac *.aac *.m4a)",
            "Video Files (*.mp4 *.mkv *.webm *.avi)",
            "All Files (*)"
        ]
        onAccepted: {
            if (tile) {
                var path = selectedFile.toString();
                if (path.indexOf("file://") === 0) {
                    path = path.substring(7);
                }
                tile.filePath = path;
                if (nameField.text.length === 0 || nameField.text === "Empty Sound") {
                    var filename = path.substring(path.lastIndexOf('/') + 1);
                    var dotIdx = filename.lastIndexOf('.');
                    if (dotIdx > 0) filename = filename.substring(0, dotIdx);
                    tile.displayName = filename;
                    nameField.text = filename;
                }
            }
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: formCol.implicitHeight + 20
        clip: true
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ColumnLayout {
            id: formCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 8
            spacing: 14

            // --- 1. Sound Name ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: qsTr("Display Title:"); color: "#AAAAAA"; font.pointSize: 10; font.bold: true }
                DarkTextField {
                    id: nameField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    text: tile ? tile.displayName : ""
                    placeholderText: qsTr("e.g. Applause, Intro Theme, Rimshot")
                    onTextChanged: {
                        if (tile && activeFocus) tile.displayName = text;
                    }
                }
            }

            // --- 2. File Path ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: qsTr("Audio / Video Media Source:"); color: "#AAAAAA"; font.pointSize: 10; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    DarkTextField {
                        id: pathField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        text: tile ? tile.filePath : ""
                        readOnly: true
                        placeholderText: qsTr("No file selected...")
                    }
                    DarkButton {
                        text: qsTr("Browse...")
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 38
                        onClicked: mediaFileDialog.open()
                    }
                }
            }

            // --- 3. Hotkey Binding (Issue #76) ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: qsTr("Keyboard Shortcut Hotkey:"); color: "#AAAAAA"; font.pointSize: 10; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    DarkTextField {
                        id: hotkeyField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        text: tile ? tile.hotkey : ""
                        placeholderText: qsTr("e.g. Ctrl+Alt+1, Space, F5, or letter key")
                        onTextChanged: {
                            if (tile && activeFocus) tile.hotkey = text;
                        }
                    }

                    DarkButton {
                        text: qsTr("Clear")
                        Layout.preferredWidth: 70
                        Layout.preferredHeight: 38
                        enabled: tile && tile.hotkey && tile.hotkey.length > 0
                        onClicked: {
                            if (tile) {
                                tile.hotkey = "";
                                hotkeyField.text = "";
                            }
                        }
                    }
                }
                Text {
                    text: qsTr("Supports full modifiers (Ctrl, Alt, Shift) or standalone keys. Hotkeys can be muted during shows via Top Bar.")
                    color: "#7E8590"
                    font.pointSize: 8
                }
            }

            // --- 4. Tile Accent Color ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Text { text: qsTr("Tile Color Accent:"); color: "#AAAAAA"; font.pointSize: 10; font.bold: true }

                RowLayout {
                    spacing: 8
                    Repeater {
                        model: ["#8E2DE2", "#E74C3C", "#2ECC71", "#3498DB", "#E67E22", "#1ABC9C", "#E91E63", "#95A5A6"]
                        delegate: Rectangle {
                            required property string modelData
                            width: 32
                            height: 32
                            radius: 16
                            color: modelData
                            border.color: (tile && tile.color === modelData) ? "#FFFFFF" : "#444444"
                            border.width: (tile && tile.color === modelData) ? 3 : 1

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                color: "#FFFFFF"
                                font.bold: true
                                visible: tile && tile.color === modelData
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (tile) tile.color = modelData;
                                }
                            }
                        }
                    }
                }
            }

            // --- 5. Trigger Style ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: qsTr("Playback Trigger Mode:"); color: "#AAAAAA"; font.pointSize: 10; font.bold: true }
                ComboBox {
                    id: triggerCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    model: [
                        qsTr("Retrigger (Play from start on each tap)"),
                        qsTr("Restart (Stop current and restart)"),
                        qsTr("Play / Pause (Toggle pause)"),
                        qsTr("Play / Stop (Toggle stop)")
                    ]
                    currentIndex: tile ? tile.triggerStyle : 0
                    onActivated: (index) => {
                        if (tile) tile.triggerStyle = index;
                    }
                    background: Rectangle {
                        color: "#282C32"
                        radius: 6
                        border.color: parent.activeFocus ? "#8E2DE2" : "#454B54"
                        border.width: 1
                    }
                    contentItem: Text {
                        leftPadding: 10
                        text: triggerCombo.displayText
                        color: "#FFFFFF"
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            // --- 6. Cue Point Region ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: cueCol.implicitHeight + 16
                radius: 8
                color: "#1D1F23"
                border.color: "#343940"
                border.width: 1

                ColumnLayout {
                    id: cueCol
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            id: cueCheck
                            checked: tile ? tile.hasCueRegion : false
                            text: qsTr("Enable Cue Region (In / Out Trimming)")
                            onToggled: {
                                if (tile) tile.hasCueRegion = checked;
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        enabled: cueCheck.checked
                        opacity: cueCheck.checked ? 1.0 : 0.4
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text { text: qsTr("Cue In (ms):"); color: "#AAAAAA"; font.pointSize: 9 }
                            RowLayout {
                                DarkTextField {
                                    id: cueInField
                                    text: tile ? tile.cueBeginMs.toString() : "0"
                                    Layout.fillWidth: true
                                    onTextChanged: {
                                        if (tile && activeFocus) tile.cueBeginMs = parseInt(text) || 0;
                                    }
                                }
                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 64
                                    Layout.preferredHeight: 38
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Set to current playback position")
                                    onClicked: {
                                        if (tile) {
                                            tile.cueBeginMs = tile.positionMs;
                                            cueInField.text = tile.positionMs.toString();
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text { text: qsTr("Cue Out (ms):"); color: "#AAAAAA"; font.pointSize: 9 }
                            RowLayout {
                                DarkTextField {
                                    id: cueOutField
                                    text: tile ? tile.cueEndMs.toString() : "0"
                                    Layout.fillWidth: true
                                    onTextChanged: {
                                        if (tile && activeFocus) tile.cueEndMs = parseInt(text) || 0;
                                    }
                                }
                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 64
                                    Layout.preferredHeight: 38
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Set to current playback position")
                                    onClicked: {
                                        if (tile) {
                                            tile.cueEndMs = tile.positionMs;
                                            cueOutField.text = tile.positionMs.toString();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // --- 7. Danger Zone: Delete Tile ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                radius: 6
                color: "#2C1D20"
                border.color: "#8B263E"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    Text {
                        text: qsTr("Remove this sound tile permanently")
                        color: "#E74C3C"
                        font.pointSize: 10
                        Layout.fillWidth: true
                    }
                    Button {
                        text: qsTr("Delete Tile")
                        Layout.preferredHeight: 32
                        onClicked: {
                            if (tile) {
                                root.removeTileRequested(tile);
                                root.close();
                            }
                        }
                        background: Rectangle {
                            color: parent.hovered ? "#C0392B" : "#962D22"
                            radius: 4
                        }
                    }
                }
            }
        }
    }
}
