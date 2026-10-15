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

    function formatPreciseTime(ms) {
        if (!ms || ms <= 0) return "00:00.00";
        var totalSec = Math.floor(ms / 1000);
        var min = Math.floor(totalSec / 60);
        var sec = totalSec % 60;
        var hundredths = Math.floor((ms % 1000) / 10);
        return (min < 10 ? "0" + min : min) + ":" + 
               (sec < 10 ? "0" + sec : sec) + "." + 
               (hundredths < 10 ? "0" + hundredths : hundredths);
    }

    title: qsTr("Sound Tile Properties")
    modal: true
    anchors.centerIn: parent
    width: Math.min(580, parent.width - 32)
    height: Math.min(800, parent.height - 32)

    Component.onCompleted: {
        if (typeof openHotkeyPromptOnStart !== "undefined" && openHotkeyPromptOnStart) {
            hotkeyPromptDialog.openDialog();
        }
    }

    onOpened: {
        if (typeof scrollToCueOnStart !== "undefined" && scrollToCueOnStart) {
            flickable.contentY = 320;
        }
    }

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

    // --- Hotkey Capture Prompt Dialog ---
    Dialog {
        id: hotkeyPromptDialog
        title: qsTr("Register Key Combination")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(480, (parent ? parent.width - 40 : 480))
        height: 290
        topPadding: 14
        bottomPadding: 14
        leftPadding: 18
        rightPadding: 18

        property string pendingSequence: ""
        property string pendingModifiers: ""

        function openDialog() {
            pendingSequence = "";
            pendingModifiers = "";
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.isMuted = true;
            }
            open();
        }

        onOpened: {
            captureArea.forceActiveFocus();
        }

        onClosed: {
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.isMuted = false;
            }
        }

        background: Rectangle {
            color: "#202327"
            radius: 12
            border.color: "#3E434D"
            border.width: 1
        }

        header: Rectangle {
            height: 48
            color: "#181A1D"
            radius: 12

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                Text {
                    text: "⌨"
                    font.pointSize: 14
                    color: "#8E2DE2"
                }

                Text {
                    text: qsTr("Register Key Combination")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 11
                    Layout.fillWidth: true
                }
            }
        }

        contentItem: Item {
            id: captureArea
            focus: true

            Keys.priority: Keys.BeforeItem
            Keys.enabled: true
            Keys.onPressed: (event) => {
                event.accepted = true;

                // Escape cancels dialog
                if (event.key === Qt.Key_Escape && event.modifiers === Qt.NoModifier) {
                    hotkeyPromptDialog.close();
                    return;
                }

                // Enter / Return applies sequence
                if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && event.modifiers === Qt.NoModifier) {
                    if (hotkeyPromptDialog.pendingSequence.length > 0) {
                        hotkeyPromptDialog.applySequence();
                    }
                    return;
                }

                // Modifier-only press
                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (typeof hotkeyManager !== "undefined") {
                        hotkeyPromptDialog.pendingModifiers = hotkeyManager.modifiersToString(event.modifiers);
                    }
                    return;
                }

                // Complete key combination
                if (typeof hotkeyManager !== "undefined") {
                    var seq = hotkeyManager.keySequenceToString(event.key, event.modifiers);
                    if (seq && seq.length > 0) {
                        hotkeyPromptDialog.pendingSequence = seq;
                        hotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            Keys.onReleased: (event) => {
                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (hotkeyPromptDialog.pendingSequence.length === 0) {
                        hotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: captureArea.forceActiveFocus()
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: qsTr("Press the key combination you wish to register on your keyboard:")
                    color: "#BDC3C7"
                    font.pointSize: 10
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Key Combination Display Badge
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    radius: 8
                    color: "#16181B"
                    border.color: hotkeyPromptDialog.pendingSequence.length > 0 ? "#8E2DE2" : (hotkeyPromptDialog.pendingModifiers.length > 0 ? "#A855F7" : "#2C3036")
                    border.width: hotkeyPromptDialog.pendingSequence.length > 0 ? 2 : 1

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: {
                                if (hotkeyPromptDialog.pendingSequence.length > 0) {
                                    return hotkeyPromptDialog.pendingSequence;
                                }
                                if (hotkeyPromptDialog.pendingModifiers.length > 0) {
                                    return hotkeyPromptDialog.pendingModifiers;
                                }
                                return qsTr("Listening... Press any key or combo");
                            }
                            color: hotkeyPromptDialog.pendingSequence.length > 0 ? "#FFFFFF" : (hotkeyPromptDialog.pendingModifiers.length > 0 ? "#D19CF7" : "#7E8590")
                            font.bold: true
                            font.pointSize: hotkeyPromptDialog.pendingSequence.length > 0 ? 15 : 10.5
                            font.italic: hotkeyPromptDialog.pendingSequence.length === 0 && hotkeyPromptDialog.pendingModifiers.length === 0
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: (tile && tile.hotkey && tile.hotkey.length > 0) ? qsTr("Current shortcut: %1").arg(tile.hotkey) : qsTr("No shortcut currently assigned")
                        color: "#7E8590"
                        font.pointSize: 8
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: qsTr("e.g. Ctrl+Alt+1, Space, F5, P")
                        color: "#606670"
                        font.pointSize: 8
                    }
                }
            }
        }

        footer: Rectangle {
            height: 52
            color: "#181A1D"
            radius: 12

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                DarkButton {
                    text: qsTr("Clear")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: {
                        hotkeyPromptDialog.pendingSequence = "";
                        hotkeyPromptDialog.pendingModifiers = "";
                        hotkeyField.text = "";
                        if (tile) tile.hotkey = "";
                        hotkeyPromptDialog.close();
                    }
                }

                Item { Layout.fillWidth: true }

                DarkButton {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: hotkeyPromptDialog.close()
                }

                Button {
                    id: applyBtn
                    text: qsTr("Apply")
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 34
                    enabled: hotkeyPromptDialog.pendingSequence.length > 0

                    contentItem: Text {
                        text: parent.text
                        font.bold: true
                        font.pointSize: 10
                        color: parent.enabled ? "#FFFFFF" : "#7E8590"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.enabled ? (parent.down ? "#7322B8" : (parent.hovered ? "#9D3BF0" : "#8E2DE2")) : "#2E333B"
                        radius: 6
                    }

                    onClicked: hotkeyPromptDialog.applySequence()
                }
            }
        }

        function applySequence() {
            if (pendingSequence.length > 0) {
                hotkeyField.text = pendingSequence;
                if (tile) tile.hotkey = pendingSequence;
            }
            close();
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
                        text: qsTr("Press Key...")
                        Layout.preferredWidth: 105
                        Layout.preferredHeight: 38
                        onClicked: hotkeyPromptDialog.openDialog()
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open prompt dialog to press and register a key combination")
                    }

                    DarkButton {
                        text: qsTr("Clear")
                        Layout.preferredWidth: 70
                        Layout.preferredHeight: 38
                        enabled: (tile && tile.hotkey && tile.hotkey.length > 0) || (hotkeyField.text.length > 0)
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
                Layout.preferredHeight: cueCol.implicitHeight + 24
                radius: 8
                color: "#1D1F23"
                border.color: "#343940"
                border.width: 1

                ColumnLayout {
                    id: cueCol
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    // Top Header Row: CheckBox & Action Buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

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
                                if (tile) {
                                    tile.hasCueRegion = checked;
                                    if (checked && (tile.cueEndMs <= tile.cueBeginMs || tile.cueEndMs === 0) && tile.durationMs > 0) {
                                        tile.cueEndMs = tile.durationMs;
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }

                        DarkButton {
                            text: qsTr("Reset Full")
                            visible: cueCheck.checked && tile && tile.durationMs > 0
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 86
                            onClicked: {
                                if (tile) {
                                    tile.cueBeginMs = 0;
                                    tile.cueEndMs = tile.durationMs;
                                }
                            }
                        }

                        DarkButton {
                            text: qsTr("▶ Audition Cue")
                            visible: cueCheck.checked && tile && tile.filePath && tile.filePath.length > 0
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 116
                            onClicked: {
                                if (tile) {
                                    tile.seek(tile.cueBeginMs);
                                    tile.play();
                                }
                            }
                        }
                    }

                    // --- Waveform Cue Region Visual Editor Container ---
                    Item {
                        id: cueWaveformBox
                        Layout.fillWidth: true
                        Layout.preferredHeight: 140
                        visible: (tile && tile.filePath && tile.filePath.length > 0)
                        opacity: cueCheck.checked ? 1.0 : 0.45
                        enabled: cueCheck.checked

                        readonly property real trackWidth: cueTrack.width > 0 ? cueTrack.width : 1
                        readonly property real duration: (tile && tile.durationMs > 0) ? tile.durationMs : 1000

                        readonly property real cueInX: {
                            if (!tile || duration <= 0) return 0;
                            var frac = Math.max(0.0, Math.min(1.0, tile.cueBeginMs / duration));
                            return trackWidth * frac;
                        }

                        readonly property real cueOutX: {
                            if (!tile || duration <= 0) return trackWidth;
                            var endVal = (tile.cueEndMs > 0) ? tile.cueEndMs : duration;
                            var frac = Math.max(0.0, Math.min(1.0, endVal / duration));
                            return trackWidth * frac;
                        }

                        // Top Readout Row: Floating START Label & Length
                        Item {
                            id: cueTopBar
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 22

                            // Center Cue Duration Label
                            Text {
                                anchors.centerIn: parent
                                text: {
                                    if (!tile) return "";
                                    var inMs = tile.cueBeginMs;
                                    var outMs = (tile.cueEndMs > 0) ? tile.cueEndMs : tile.durationMs;
                                    var diff = Math.max(0, outMs - inMs);
                                    return qsTr("CUE LENGTH: %1").arg(root.formatPreciseTime(diff));
                                }
                                color: "#8E9AA8"
                                font.pointSize: 8
                                font.bold: true
                            }

                            // Dynamic START Badge (Tracks Start Handle)
                            Rectangle {
                                id: startBadge
                                x: Math.max(0, Math.min(cueWaveformBox.width - width, cueWaveformBox.cueInX - width / 2))
                                anchors.verticalCenter: parent.verticalCenter
                                width: startLabel.implicitWidth + 12
                                height: 20
                                radius: 4
                                color: "#16191E"
                                border.color: startHandleMouse.pressed ? "#FFFFFF" : "#00D2FF"
                                border.width: 1

                                Text {
                                    id: startLabel
                                    anchors.centerIn: parent
                                    text: qsTr("START: %1").arg(root.formatPreciseTime(tile ? tile.cueBeginMs : 0))
                                    color: "#00D2FF"
                                    font.bold: true
                                    font.pointSize: 8
                                    font.family: "Monospace"
                                }
                            }
                        }

                        // Middle Interactive Waveform Track Container
                        Item {
                            id: cueTrack
                            anchors.top: cueTopBar.bottom
                            anchors.topMargin: 2
                            anchors.bottom: cueBottomBar.top
                            anchors.bottomMargin: 2
                            anchors.left: parent.left
                            anchors.right: parent.right

                            // Background Box
                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                                color: "#131518"
                                border.color: "#272B32"
                                border.width: 1

                                // Inner Clipped Canvas Area
                                Item {
                                    id: innerWaveArea
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    clip: true

                                    // Time Grid Guidelines
                                    Row {
                                        anchors.fill: parent
                                        Repeater {
                                            model: 4
                                            Item {
                                                width: innerWaveArea.width / 4
                                                height: innerWaveArea.height
                                                Rectangle {
                                                    anchors.right: parent.right
                                                    anchors.top: parent.top
                                                    anchors.bottom: parent.bottom
                                                    width: 1
                                                    color: "#1E232A"
                                                    visible: index < 3
                                                }
                                            }
                                        }
                                    }

                                    // Center Horizontal Midline
                                    Rectangle {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        height: 1
                                        color: "#1E232A"
                                    }

                                    // 1. Base Unselected Waveform (Muted Slate / Teal)
                                    Canvas {
                                        id: cueWaveformBase
                                        anchors.fill: parent
                                        anchors.margins: 4

                                        onPaint: {
                                            var ctx = getContext("2d");
                                            ctx.reset();
                                            var data = (tile && tile.waveform) ? tile.waveform : [];
                                            var w = width;
                                            var h = height;
                                            if (!data || data.length === 0) {
                                                ctx.fillStyle = "#272B32";
                                                ctx.fillRect(0, (h - 2) / 2, w, 2);
                                                return;
                                            }

                                            var count = data.length;
                                            var step = w / count;
                                            var barWidth = Math.max(2, step * 0.70);

                                            ctx.fillStyle = "#2D3843";
                                            for (var i = 0; i < count; ++i) {
                                                var amp = Math.max(0.06, Math.min(1.0, Number(data[i])));
                                                var barH = Math.max(4, amp * (h * 0.78));
                                                var barX = i * step + (step - barWidth) / 2;
                                                var barY = (h - barH) / 2;
                                                ctx.fillRect(barX, barY, barWidth, barH);
                                            }
                                        }

                                        onWidthChanged: requestPaint()
                                        onHeightChanged: requestPaint()
                                        Component.onCompleted: requestPaint()
                                    }

                                    // 2. Selected Active Cue Region Highlight
                                    Rectangle {
                                        id: cueActiveRegion
                                        x: cueWaveformBox.cueInX
                                        width: Math.max(0, cueWaveformBox.cueOutX - cueWaveformBox.cueInX)
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        color: Qt.rgba(0.0, 0.82, 1.0, 0.12)
                                        border.color: Qt.rgba(0.0, 0.82, 1.0, 0.45)
                                        border.width: 1
                                        clip: true

                                        // Illuminated Active Waveform inside the cue region
                                        Canvas {
                                            id: cueWaveformActive
                                            x: -cueWaveformBox.cueInX
                                            y: 4
                                            width: cueTrack.width - 2
                                            height: cueTrack.height - 2

                                            onPaint: {
                                                var ctx = getContext("2d");
                                                ctx.reset();
                                                var data = (tile && tile.waveform) ? tile.waveform : [];
                                                var w = width;
                                                var h = height;
                                                if (!data || data.length === 0) return;

                                                var count = data.length;
                                                var step = w / count;
                                                var barWidth = Math.max(2, step * 0.70);

                                                ctx.fillStyle = (tile && tile.color && tile.color !== "#3A3D40") ? tile.color : "#00D2FF";
                                                for (var i = 0; i < count; ++i) {
                                                    var amp = Math.max(0.06, Math.min(1.0, Number(data[i])));
                                                    var barH = Math.max(4, amp * (h * 0.78));
                                                    var barX = i * step + (step - barWidth) / 2;
                                                    var barY = (h - barH) / 2;
                                                    ctx.fillRect(barX, barY, barWidth, barH);
                                                }
                                            }

                                            onWidthChanged: requestPaint()
                                            onHeightChanged: requestPaint()
                                            Component.onCompleted: requestPaint()
                                        }
                                    }

                                    // 3. Playback Needle / Audition Cursor
                                    Rectangle {
                                        visible: tile && tile.durationMs > 0 && tile.playbackState !== 0
                                        x: Math.round(innerWaveArea.width * (tile ? (tile.positionMs / tile.durationMs) : 0))
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        width: 2
                                        color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#FFFFFF"
                                        opacity: 0.9
                                    }
                                }
                            }

                            // Middle Drag Area (Move whole cue window)
                            Item {
                                x: cueWaveformBox.cueInX + 13
                                width: Math.max(0, cueWaveformBox.cueOutX - cueWaveformBox.cueInX - 26)
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                visible: width > 10

                                MouseArea {
                                    id: midDragArea
                                    anchors.fill: parent
                                    cursorShape: Qt.SizeAllCursor
                                    preventStealing: true

                                    property real dragStartX: 0
                                    property int origIn: 0
                                    property int origOut: 0

                                    onPressed: (mouse) => {
                                        dragStartX = mapToItem(cueTrack, mouse.x, 0).x;
                                        origIn = tile ? tile.cueBeginMs : 0;
                                        origOut = tile ? ((tile.cueEndMs > 0) ? tile.cueEndMs : tile.durationMs) : 0;
                                    }

                                    onPositionChanged: (mouse) => {
                                        if (!pressed || !tile || tile.durationMs <= 0) return;
                                        var globalX = mapToItem(cueTrack, mouse.x, 0).x;
                                        var deltaPx = globalX - dragStartX;
                                        var deltaMs = Math.round((deltaPx / cueTrack.width) * tile.durationMs);
                                        var len = origOut - origIn;
                                        var newIn = Math.max(0, Math.min(tile.durationMs - len, origIn + deltaMs));
                                        tile.cueBeginMs = newIn;
                                        tile.cueEndMs = newIn + len;
                                    }
                                }
                            }

                            // 4. Cue In (START) Slider Handle
                            Item {
                                id: startHandle
                                x: Math.round(cueWaveformBox.cueInX - width / 2)
                                y: -4
                                width: 26
                                height: cueTrack.height + 8
                                z: 10

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 13
                                    color: startHandleMouse.pressed ? "#FFFFFF" : (startHandleMouse.containsMouse ? "#F0F4F8" : "#E2E8F0")
                                    border.color: startHandleMouse.pressed ? "#00D2FF" : (startHandleMouse.containsMouse ? "#38BDF8" : "#94A3B8")
                                    border.width: 2

                                    // Center Grip Ridges
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 2
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                width: 2
                                                height: 16
                                                radius: 1
                                                color: "#64748B"
                                            }
                                        }
                                    }
                                }

                                MouseArea {
                                    id: startHandleMouse
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.SizeHorCursor
                                    preventStealing: true

                                    onPositionChanged: (mouse) => {
                                        if (!pressed || !tile || tile.durationMs <= 0) return;
                                        var globalX = mapToItem(cueTrack, mouse.x, 0).x;
                                        var frac = Math.max(0.0, Math.min(1.0, globalX / cueTrack.width));
                                        var targetMs = Math.round(frac * tile.durationMs);
                                        var maxAllowed = (tile.cueEndMs > 50) ? tile.cueEndMs - 50 : (tile.durationMs - 50);
                                        targetMs = Math.min(targetMs, maxAllowed);
                                        targetMs = Math.max(0, targetMs);
                                        tile.cueBeginMs = targetMs;
                                    }
                                }
                            }

                            // 5. Cue Out (END) Slider Handle
                            Item {
                                id: endHandle
                                x: Math.round(cueWaveformBox.cueOutX - width / 2)
                                y: -4
                                width: 26
                                height: cueTrack.height + 8
                                z: 10

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 13
                                    color: endHandleMouse.pressed ? "#FFFFFF" : (endHandleMouse.containsMouse ? "#F0F4F8" : "#E2E8F0")
                                    border.color: endHandleMouse.pressed ? "#00D2FF" : (endHandleMouse.containsMouse ? "#38BDF8" : "#94A3B8")
                                    border.width: 2

                                    // Center Grip Ridges
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 2
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                width: 2
                                                height: 16
                                                radius: 1
                                                color: "#64748B"
                                            }
                                        }
                                    }
                                }

                                MouseArea {
                                    id: endHandleMouse
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.SizeHorCursor
                                    preventStealing: true

                                    onPositionChanged: (mouse) => {
                                        if (!pressed || !tile || tile.durationMs <= 0) return;
                                        var globalX = mapToItem(cueTrack, mouse.x, 0).x;
                                        var frac = Math.max(0.0, Math.min(1.0, globalX / cueTrack.width));
                                        var targetMs = Math.round(frac * tile.durationMs);
                                        targetMs = Math.max(targetMs, tile.cueBeginMs + 50);
                                        targetMs = Math.min(tile.durationMs, targetMs);
                                        tile.cueEndMs = targetMs;
                                    }
                                }
                            }
                        }

                        // Bottom Readout Row: Floating END Label & Time bounds
                        Item {
                            id: cueBottomBar
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 22

                            // 00:00.00 start bound
                            Text {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: "00:00.00"
                                color: "#64748B"
                                font.pointSize: 7
                                font.family: "Monospace"
                                visible: cueWaveformBox.cueInX > 75
                            }

                            // Dynamic END Badge (Tracks End Handle)
                            Rectangle {
                                id: endBadge
                                x: Math.max(0, Math.min(cueWaveformBox.width - width, cueWaveformBox.cueOutX - width / 2))
                                anchors.verticalCenter: parent.verticalCenter
                                width: endLabel.implicitWidth + 12
                                height: 20
                                radius: 4
                                color: "#16191E"
                                border.color: endHandleMouse.pressed ? "#FFFFFF" : "#00D2FF"
                                border.width: 1

                                Text {
                                    id: endLabel
                                    anchors.centerIn: parent
                                    text: qsTr("END: %1").arg(root.formatPreciseTime(tile ? (tile.cueEndMs > 0 ? tile.cueEndMs : tile.durationMs) : 0))
                                    color: "#00D2FF"
                                    font.bold: true
                                    font.pointSize: 8
                                    font.family: "Monospace"
                                }
                            }

                            // Total duration end bound
                            Text {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                text: root.formatPreciseTime(tile ? tile.durationMs : 0)
                                color: "#64748B"
                                font.pointSize: 7
                                font.family: "Monospace"
                                visible: cueWaveformBox.cueOutX < (cueWaveformBox.trackWidth - 75)
                            }
                        }
                    }

                    // Numeric Fine-Tuning Controls Row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        visible: cueCheck.checked

                        // Cue In (Start) Column
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTr("Cue In (Start):")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                font.bold: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                DarkButton {
                                    text: "-100"
                                    Layout.preferredWidth: 46
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Nudge -100ms")
                                    onClicked: {
                                        if (tile) tile.cueBeginMs = Math.max(0, tile.cueBeginMs - 100);
                                    }
                                }

                                DarkTextField {
                                    id: cueInField
                                    text: tile ? tile.cueBeginMs.toString() : "0"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                    Layout.minimumWidth: 40
                                    onTextChanged: {
                                        if (tile && activeFocus) {
                                            var val = parseInt(text) || 0;
                                            var maxIn = (tile.cueEndMs > 50) ? tile.cueEndMs - 50 : 0;
                                            tile.cueBeginMs = Math.max(0, Math.min(maxIn, val));
                                        }
                                    }
                                }

                                DarkButton {
                                    text: "+100"
                                    Layout.preferredWidth: 46
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Nudge +100ms")
                                    onClicked: {
                                        if (tile) {
                                            var maxIn = (tile.cueEndMs > 50) ? tile.cueEndMs - 50 : 0;
                                            tile.cueBeginMs = Math.min(maxIn, tile.cueBeginMs + 100);
                                        }
                                    }
                                }

                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Set Cue In to current playhead position")
                                    onClicked: {
                                        if (tile) {
                                            var maxIn = (tile.cueEndMs > 50) ? tile.cueEndMs - 50 : 0;
                                            tile.cueBeginMs = Math.min(maxIn, tile.positionMs);
                                        }
                                    }
                                }
                            }
                        }

                        // Cue Out (End) Column
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTr("Cue Out (End):")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                font.bold: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                DarkButton {
                                    text: "-100"
                                    Layout.preferredWidth: 46
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Nudge -100ms")
                                    onClicked: {
                                        if (tile) {
                                            tile.cueEndMs = Math.max(tile.cueBeginMs + 50, (tile.cueEndMs > 0 ? tile.cueEndMs : tile.durationMs) - 100);
                                        }
                                    }
                                }

                                DarkTextField {
                                    id: cueOutField
                                    text: tile ? (tile.cueEndMs > 0 ? tile.cueEndMs.toString() : (tile.durationMs > 0 ? tile.durationMs.toString() : "0")) : "0"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                    Layout.minimumWidth: 40
                                    onTextChanged: {
                                        if (tile && activeFocus) {
                                            var val = parseInt(text) || 0;
                                            tile.cueEndMs = Math.max(tile.cueBeginMs + 50, Math.min(tile.durationMs, val));
                                        }
                                    }
                                }

                                DarkButton {
                                    text: "+100"
                                    Layout.preferredWidth: 46
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Nudge +100ms")
                                    onClicked: {
                                        if (tile) {
                                            tile.cueEndMs = Math.min(tile.durationMs, (tile.cueEndMs > 0 ? tile.cueEndMs : tile.durationMs) + 100);
                                        }
                                    }
                                }

                                DarkButton {
                                    text: qsTr("Here")
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Set Cue Out to current playhead position")
                                    onClicked: {
                                        if (tile) {
                                            tile.cueEndMs = Math.max(tile.cueBeginMs + 50, tile.positionMs);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Connections {
                target: tile
                function onCueBeginMsChanged() {
                    if (cueInField && !cueInField.activeFocus && tile) {
                        cueInField.text = tile.cueBeginMs.toString();
                    }
                }
                function onCueEndMsChanged() {
                    if (cueOutField && !cueOutField.activeFocus && tile) {
                        cueOutField.text = (tile.cueEndMs > 0 ? tile.cueEndMs.toString() : (tile.durationMs > 0 ? tile.durationMs.toString() : "0"));
                    }
                }
                function onWaveformChanged() {
                    if (cueWaveformBase) cueWaveformBase.requestPaint();
                    if (cueWaveformActive) cueWaveformActive.requestPaint();
                }
                function onDurationMsChanged() {
                    if (cueWaveformBase) cueWaveformBase.requestPaint();
                    if (cueWaveformActive) cueWaveformActive.requestPaint();
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
