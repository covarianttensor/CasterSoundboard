import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    property var tile: null
    signal removeTileRequested(var tile)

    function formatTime(ms) {
        if (!ms || ms <= 0) return "00:00";
        var totalSec = Math.floor(ms / 1000);
        var min = Math.floor(totalSec / 60);
        var sec = totalSec % 60;
        return (min < 10 ? "0" + min : min) + ":" + (sec < 10 ? "0" + sec : sec);
    }

    title: qsTr("Sound Tile Properties")
    modal: true
    anchors.centerIn: parent
    width: Math.min(580, parent.width - 32)
    height: Math.min(800, parent.height - 32)

    topPadding: 12
    bottomPadding: 12
    leftPadding: 16
    rightPadding: 16

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
        implicitWidth: 100
        Layout.fillWidth: true
        Layout.minimumWidth: 50
        Layout.preferredWidth: 100
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        selectByMouse: true
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
        id: flickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: formCol.implicitHeight + 20
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ColumnLayout {
            id: formCol
            width: flickable.width - 12
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
                        Layout.preferredWidth: 100
                        Layout.minimumWidth: 50
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
                        Layout.preferredWidth: 100
                        Layout.minimumWidth: 50
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
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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

            // --- 6. Audition & Playback Scrubber ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: auditionCol.implicitHeight + 20
                radius: 8
                color: "#1D1F23"
                border.color: "#343940"
                border.width: 1

                ColumnLayout {
                    id: auditionCol
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: qsTr("Audition & Live Scrubbing:")
                            color: "#AAAAAA"
                            font.pointSize: 10
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Text {
                            text: {
                                if (!tile || tile.durationMs <= 0) return "00:00 / 00:00";
                                var cur = auditionSlider.pressed ? Math.round(auditionSlider.value) : tile.positionMs;
                                return root.formatTime(cur) + " / " + root.formatTime(tile.durationMs);
                            }
                            color: "#8E2DE2"
                            font.family: "Monospace"
                            font.pointSize: 10
                            font.bold: true
                        }
                    }

                    // Scrubber Slider
                    Slider {
                        id: auditionSlider
                        Layout.fillWidth: true
                        from: 0
                        to: (tile && tile.durationMs > 0) ? tile.durationMs : 1000
                        value: (tile && !pressed) ? tile.positionMs : value
                        enabled: tile && tile.durationMs > 0
                        onMoved: {
                            if (tile) tile.seek(Math.round(value));
                        }

                        background: Rectangle {
                            x: auditionSlider.leftPadding
                            y: auditionSlider.topPadding + auditionSlider.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 8
                            width: auditionSlider.availableWidth
                            height: implicitHeight
                            radius: 4
                            color: "#2D323A"

                            // Cue region highlight on slider
                            Rectangle {
                                visible: tile && tile.hasCueRegion && tile.durationMs > 0
                                x: tile && tile.durationMs > 0 ? (parent.width * (Math.max(0, Math.min(tile.durationMs, tile.cueBeginMs)) / tile.durationMs)) : 0
                                width: {
                                    if (!tile || tile.durationMs <= 0) return 0;
                                    var begin = Math.max(0, Math.min(tile.durationMs, tile.cueBeginMs));
                                    var end = Math.max(begin, Math.min(tile.durationMs, tile.cueEndMs));
                                    return Math.max(2, parent.width * ((end - begin) / tile.durationMs));
                                }
                                height: parent.height
                                color: "#E67E22"
                                opacity: 0.55
                                radius: 2
                            }

                            // Progress fill
                            Rectangle {
                                width: auditionSlider.visualPosition * parent.width
                                height: parent.height
                                color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#8E2DE2"
                                radius: 4
                            }
                        }

                        handle: Rectangle {
                            x: auditionSlider.leftPadding + auditionSlider.visualPosition * (auditionSlider.availableWidth - width)
                            y: auditionSlider.topPadding + auditionSlider.availableHeight / 2 - height / 2
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: 8
                            color: "#FFFFFF"
                            border.color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#8E2DE2"
                            border.width: 2
                            scale: auditionSlider.pressed ? 1.25 : (auditionSlider.hovered ? 1.1 : 1.0)
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }
                    }

                    // Transport controls row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        DarkButton {
                            text: (tile && tile.playbackState === 1) ? qsTr("⏸ Pause") : qsTr("▶ Play")
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 36
                            enabled: tile && tile.filePath && tile.filePath.length > 0
                            onClicked: {
                                if (!tile) return;
                                if (tile.playbackState === 1) {
                                    tile.pause();
                                } else {
                                    tile.play();
                                }
                            }
                        }

                        DarkButton {
                            text: qsTr("⏹ Stop")
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 36
                            enabled: tile && tile.playbackState !== 0
                            onClicked: {
                                if (tile) tile.stop();
                            }
                        }

                        Item { Layout.fillWidth: true }

                        DarkButton {
                            text: qsTr("⏮ Rewind")
                            Layout.preferredWidth: 86
                            Layout.preferredHeight: 36
                            enabled: tile && tile.durationMs > 0
                            onClicked: {
                                if (tile) tile.seek(0);
                            }
                        }
                    }
                }
            }

            // --- 7. Cue Point Region ---
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
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10
                                leftPadding: 28
                                verticalAlignment: Text.AlignVCenter
                            }
                            onToggled: {
                                if (tile) tile.hasCueRegion = checked;
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        visible: cueCheck.checked

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text { text: qsTr("Cue In (ms):"); color: "#AAAAAA"; font.pointSize: 9 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                DarkTextField {
                                    id: cueInField
                                    text: tile ? tile.cueBeginMs.toString() : "0"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                    Layout.minimumWidth: 40
                                    onTextChanged: {
                                        if (tile && activeFocus) tile.cueBeginMs = parseInt(text) || 0;
                                    }
                                }
                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 56
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
                                Layout.fillWidth: true
                                spacing: 6
                                DarkTextField {
                                    id: cueOutField
                                    text: tile ? tile.cueEndMs.toString() : "0"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                    Layout.minimumWidth: 40
                                    onTextChanged: {
                                        if (tile && activeFocus) tile.cueEndMs = parseInt(text) || 0;
                                    }
                                }
                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 56
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

            // --- 8. Danger Zone: Delete Tile ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                radius: 6
                color: "#2C1D20"
                border.color: "#8B263E"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Text {
                        text: qsTr("Remove this sound tile permanently")
                        color: "#E74C3C"
                        font.pointSize: 10
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        elide: Text.ElideRight
                    }
                    Button {
                        text: qsTr("Delete Tile")
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 34
                        onClicked: {
                            if (tile) {
                                root.removeTileRequested(tile);
                                root.close();
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 10
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.down ? "#7B1F17" : (parent.hovered ? "#C0392B" : "#962D22")
                            radius: 4
                        }
                    }
                }
            }
        }
    }
}
