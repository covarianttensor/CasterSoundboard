import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property alias tileScaleValue: scaleSlider.value

    signal newTabRequested()
    signal importDirectoryRequested()
    signal openProjectRequested()
    signal saveProjectRequested()
    signal saveAsProjectRequested()
    signal renameTabRequested()
    signal openPttRequested()
    signal stopAllRequested()
    signal toggleDuckingRequested()
    signal toggleMuteHotkeysRequested()
    signal changeStopAllShortcutRequested()
    signal changeDuckingShortcutRequested()
    signal changeShortcutRequested(string action)

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
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.openProjectHotkey.length > 0)
                         ? " (" + projectController.openProjectHotkey + ")" : "";
                return qsTr("Open .caster Project%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "openProject"
            defaultKey: "Ctrl+O"
            onClicked: root.openProjectRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("openProject")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.openProjectHotkey = "Ctrl+O";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.openProjectHotkey = "";
            }
        }

        ActionIconButton {
            iconSource: "icons/save.png"
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.saveProjectHotkey.length > 0)
                         ? " (" + projectController.saveProjectHotkey + ")" : "";
                return qsTr("Save Project%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "saveProject"
            defaultKey: "Ctrl+S"
            onClicked: root.saveProjectRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("saveProject")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.saveProjectHotkey = "Ctrl+S";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.saveProjectHotkey = "";
            }
        }

        ActionIconButton {
            iconSource: "icons/save_as.png"
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.saveAsProjectHotkey.length > 0)
                         ? " (" + projectController.saveAsProjectHotkey + ")" : "";
                return qsTr("Save Project As...%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "saveAsProject"
            defaultKey: "Ctrl+Shift+S"
            onClicked: root.saveAsProjectRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("saveAsProject")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.saveAsProjectHotkey = "Ctrl+Shift+S";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.saveAsProjectHotkey = "";
            }
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
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.newTabHotkey.length > 0)
                         ? " (" + projectController.newTabHotkey + ")" : "";
                return qsTr("Add Soundboard Tab%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "newTab"
            defaultKey: "Ctrl+N"
            onClicked: root.newTabRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("newTab")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.newTabHotkey = "Ctrl+N";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.newTabHotkey = "";
            }
        }

        ActionIconButton {
            iconSource: "icons/import_directory.png"
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.importDirectoryHotkey.length > 0)
                         ? " (" + projectController.importDirectoryHotkey + ")" : "";
                return qsTr("Import Directory as Soundboard...%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "importDirectory"
            defaultKey: "Ctrl+I"
            onClicked: root.importDirectoryRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("importDirectory")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.importDirectoryHotkey = "Ctrl+I";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.importDirectoryHotkey = "";
            }
        }

        ActionIconButton {
            iconSource: "icons/rename.png"
            tooltipText: {
                var hk = (typeof projectController !== "undefined" && projectController.renameTabHotkey.length > 0)
                         ? " (" + projectController.renameTabHotkey + ")" : "";
                return qsTr("Rename Current Tab%1\nRight-click to configure shortcut").arg(hk);
            }
            actionKey: "renameTab"
            defaultKey: "F2"
            onClicked: root.renameTabRequested()
            onChangeShortcutRequested: root.changeShortcutRequested("renameTab")
            onResetShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.renameTabHotkey = "F2";
            }
            onClearShortcutRequested: {
                if (typeof projectController !== "undefined") projectController.renameTabHotkey = "";
            }
        }

        // Push-To-Talk (PTT) Button
        Button {
            id: pttBtn
            readonly property bool isTransmitting: {
                if (typeof projectController !== "undefined" && projectController.pttController) {
                    return projectController.pttController.isTransmitting;
                }
                return false;
            }
            readonly property string activeKey: {
                if (typeof projectController !== "undefined" && projectController.pttController) {
                    return projectController.pttController.activeTransmittingKey;
                }
                return "";
            }
            readonly property bool isAssigned: {
                if (typeof projectController !== "undefined" && projectController.pttController && projectController.tabModel) {
                    var rev = projectController.pttController.revision;
                    var curTab = projectController.tabModel.tabNameAt(projectController.tabModel.currentIndex);
                    return projectController.pttController.hasPttForTab(curTab);
                }
                return false;
            }

            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            onClicked: root.openPttRequested()

            background: Rectangle {
                color: pttBtn.pressed 
                       ? "#1E2022" 
                       : (pttBtn.isTransmitting ? "#064E3B" : (pttBtn.isAssigned ? "#2F2344" : (pttBtn.hovered ? "#3A3E42" : "#2E3236")))
                radius: 6
                border.color: pttBtn.isTransmitting 
                              ? "#10B981" 
                              : (pttBtn.isAssigned ? "#8E2DE2" : (pttBtn.hovered ? "#4E545F" : "#3A3E42"))
                border.width: (pttBtn.isTransmitting || pttBtn.isAssigned) ? 2 : 1

                // Pulsing glow border when transmitting
                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: "transparent"
                    border.color: "#34D399"
                    border.width: 2
                    visible: pttBtn.isTransmitting

                    SequentialAnimation on opacity {
                        running: pttBtn.isTransmitting
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.3; to: 1.0; duration: 400; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: 1.0; to: 0.3; duration: 400; easing.type: Easing.InOutQuad }
                    }
                }
            }

            contentItem: ColumnLayout {
                anchors.centerIn: parent
                spacing: 1

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 2
                    Text {
                        text: pttBtn.isTransmitting ? "🟢" : "🎙️"
                        font.pixelSize: 8
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                Text {
                    text: "PTT"
                    color: pttBtn.isTransmitting 
                           ? "#34D399" 
                           : (pttBtn.isAssigned ? "#C084FC" : (pttBtn.hovered ? "#FFFFFF" : "#CCCCCC"))
                    font.bold: true
                    font.pointSize: 9
                    font.letterSpacing: 0.5
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            ToolTip.visible: hovered
            ToolTip.text: {
                var hk = (typeof projectController !== "undefined" && projectController.pttDialogHotkey.length > 0)
                         ? " (" + projectController.pttDialogHotkey + ")" : "";
                var desc = isTransmitting
                    ? qsTr("Push-to-Talk - TRANSMITTING (%1)").arg(activeKey)
                    : (isAssigned ? qsTr("Push-to-Talk (PTT) Configured - Click to edit") : qsTr("Configure Push-to-Talk (PTT)"));
                return desc + hk + "\n" + qsTr("Right-click to configure shortcut");
            }
            ToolTip.delay: 300

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        pttContextMenu.popup();
                    }
                }
            }

            Menu {
                id: pttContextMenu
                MenuItem {
                    text: qsTr("Change Shortcut...")
                    onTriggered: root.changeShortcutRequested("pttDialog")
                }
                MenuItem {
                    text: qsTr("Reset to Default (Ctrl+Shift+P)")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.pttDialogHotkey = "Ctrl+Shift+P";
                        }
                    }
                }
                MenuItem {
                    text: qsTr("Clear Shortcut")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.pttDialogHotkey = "";
                        }
                    }
                }
            }
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
            ToolTip.text: {
                var hk = (typeof projectController !== "undefined" && projectController.muteHotkeysHotkey.length > 0)
                         ? " (" + projectController.muteHotkeysHotkey + ")" : "";
                return qsTr("Toggle Hotkey Listening: Mute hotkeys while typing in notes or chat%1\nRight-click to configure shortcut").arg(hk);
            }
            ToolTip.delay: 300

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        muteHotkeysContextMenu.popup();
                    }
                }
            }

            Menu {
                id: muteHotkeysContextMenu
                MenuItem {
                    text: qsTr("Change Shortcut...")
                    onTriggered: root.changeShortcutRequested("muteHotkeys")
                }
                MenuItem {
                    text: qsTr("Reset to Default (Ctrl+M)")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.muteHotkeysHotkey = "Ctrl+M";
                        }
                    }
                }
                MenuItem {
                    text: qsTr("Clear Shortcut")
                    onTriggered: {
                        if (typeof projectController !== "undefined") {
                            projectController.muteHotkeysHotkey = "";
                        }
                    }
                }
            }
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
        property bool isActive: false
        property bool isTransmittingActive: false
        property string actionKey: ""
        property string defaultKey: ""
        signal changeShortcutRequested()
        signal resetShortcutRequested()
        signal clearShortcutRequested()

        Layout.preferredWidth: 44
        Layout.preferredHeight: 44

        background: Rectangle {
            color: aib.pressed 
                   ? "#1E2022" 
                   : (aib.isTransmittingActive ? "#064E3B" : (aib.isActive ? "#2F2344" : (aib.hovered ? "#3A3E42" : "#2E3236")))
            radius: 6
            border.color: aib.isTransmittingActive 
                          ? "#10B981" 
                          : (aib.isActive ? "#8E2DE2" : (aib.hovered ? "#4E545F" : "#3A3E42"))
            border.width: (aib.isTransmittingActive || aib.isActive) ? 2 : 1

            // Pulsing border overlay when actively transmitting
            Rectangle {
                anchors.fill: parent
                radius: 6
                color: "transparent"
                border.color: "#34D399"
                border.width: 2
                visible: aib.isTransmittingActive

                SequentialAnimation on opacity {
                    running: aib.isTransmittingActive
                    loops: Animation.Infinite
                    NumberAnimation { from: 0.3; to: 1.0; duration: 400; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.0; to: 0.3; duration: 400; easing.type: Easing.InOutQuad }
                }
            }
        }

        contentItem: Image {
            source: aib.iconSource
            anchors.centerIn: parent
            sourceSize.width: 22
            sourceSize.height: 22
            width: 22
            height: 22
            fillMode: Image.PreserveAspectFit
            opacity: aib.hovered || aib.isActive || aib.isTransmittingActive ? 1.0 : 0.85
        }

        ToolTip.visible: hovered
        ToolTip.text: aib.tooltipText
        ToolTip.delay: 300

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            visible: aib.actionKey.length > 0
            onClicked: (mouse) => {
                if (mouse.button === Qt.RightButton) {
                    actionContextMenu.popup();
                }
            }
        }

        Menu {
            id: actionContextMenu
            MenuItem {
                text: qsTr("Change Shortcut...")
                onTriggered: aib.changeShortcutRequested()
            }
            MenuItem {
                text: qsTr("Reset to Default (%1)").arg(aib.defaultKey)
                onTriggered: aib.resetShortcutRequested()
            }
            MenuItem {
                text: qsTr("Clear Shortcut")
                onTriggered: aib.clearShortcutRequested()
            }
        }
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
