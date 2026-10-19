import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    signal actionHotkeyPromptRequested(string action)

    component DarkTextField : TextField {
        id: dtf
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        implicitHeight: 36
        background: Rectangle {
            color: dtf.activeFocus ? "#30353C" : (dtf.hovered ? "#2D3238" : "#282C32")
            radius: 6
            border.color: dtf.activeFocus ? "#8E2DE2" : (dtf.hovered ? "#606875" : "#454B54")
            border.width: dtf.activeFocus ? 2 : 1
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            id: prefFlickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: prefCol.implicitHeight
            clip: true

            Timer {
                interval: 50
                running: typeof scrollBottomOnStart !== "undefined" && scrollBottomOnStart
                repeat: false
                onTriggered: {
                    prefFlickable.contentY = Math.max(0, prefFlickable.contentHeight - prefFlickable.height);
                }
            }

            ColumnLayout {
                id: prefCol
                width: parent.width
                spacing: 20

                // Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/cog.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Preferences & Workspace Settings")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Caster Soundboard v2.0 - Touch-first QML Edition")
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

                // Audio Ducking Ratio Setting
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8

                        Text {
                            text: qsTr("Ducking Volume Attenuation")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 12
                            Slider {
                                id: duckSlider
                                from: 0.05
                                to: 0.75
                                value: typeof projectController !== "undefined" ? projectController.duckingVolumeRatio : 0.25
                                stepSize: 0.05
                                Layout.preferredWidth: 240
                                onMoved: {
                                    if (typeof projectController !== "undefined") {
                                        projectController.duckingVolumeRatio = value;
                                    }
                                }
                            }
                            Text {
                                text: Math.round(duckSlider.value * 100) + "%"
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 11
                            }
                        }
                    }
                }

                // Hotkey Scope Setting (Mode B Tab-Scoped vs Mode A Global)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 165
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        Text {
                            text: qsTr("Sound Tile Hotkey Scope")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        Text {
                            text: qsTr("Control whether sound tile hotkeys trigger only on the currently active tab (Bank Mode) or trigger sounds globally across all tabs.")
                            color: "#AAAAAA"
                            font.pointSize: 9.5
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 16
                            Layout.fillWidth: true

                            // Mode B (Default, TabScoped / Sound Banks)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 64
                                radius: 6
                                color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#2F2344" : "#1A1C1F"
                                border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#8E2DE2" : (modeBMouse.containsMouse ? "#4D535D" : "#33373B")
                                border.width: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? 2 : 1

                                MouseArea {
                                    id: modeBMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.hotkeyScope = 0;
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 9
                                        color: "transparent"
                                        border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#8E2DE2" : "#555A62"
                                        border.width: 2

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: "#8E2DE2"
                                            visible: typeof projectController !== "undefined" && projectController.hotkeyScope === 0
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: qsTr("Sound Banks (Tab-Scoped) — Default")
                                            color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#FFFFFF" : "#CCCCCC"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Only active tab tiles respond. Allows reusing keys on each tab.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }
                                    }
                                }
                            }

                            // Mode A (Global)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 64
                                radius: 6
                                color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#2F2344" : "#1A1C1F"
                                border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#8E2DE2" : (modeAMouse.containsMouse ? "#4D535D" : "#33373B")
                                border.width: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? 2 : 1

                                MouseArea {
                                    id: modeAMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.hotkeyScope = 1;
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 9
                                        color: "transparent"
                                        border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#8E2DE2" : "#555A62"
                                        border.width: 2

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: "#8E2DE2"
                                            visible: typeof projectController !== "undefined" && projectController.hotkeyScope === 1
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: qsTr("Global (All Tabs)")
                                            color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#FFFFFF" : "#CCCCCC"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Tile hotkeys trigger regardless of which tab is currently active.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }
                                    }
                            }
                        }
                    }
                }
            }

                // Global & Top Bar Action Shortcuts Setting
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: shortcutsCol.implicitHeight + 32
                    Layout.preferredHeight: implicitHeight
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    component ShortcutRow : RowLayout {
                        id: sr
                        property string iconSource: ""
                        property string iconColor: "#2C353D"
                        property string iconText: ""
                        property string title: ""
                        property string subtitle: ""
                        property string hotkey: ""
                        property string actionKey: ""
                        signal clearRequested()

                        Layout.fillWidth: true
                        spacing: 12

                        Rectangle {
                            width: 28
                            height: 28
                            radius: 4
                            color: sr.iconColor

                            Image {
                                anchors.centerIn: parent
                                source: sr.iconSource
                                sourceSize: Qt.size(16, 16)
                                fillMode: Image.PreserveAspectFit
                                visible: sr.iconSource.length > 0
                            }

                            Text {
                                anchors.centerIn: parent
                                text: sr.iconText
                                font.pixelSize: 13
                                visible: sr.iconText.length > 0
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: sr.title
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            Text {
                                text: sr.subtitle
                                color: "#888888"
                                font.pointSize: 8.5
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }
                        }

                        // Shortcut Badge
                        Rectangle {
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: Math.max(70, badgeText.implicitWidth + 20)
                            radius: 6
                            color: "#16181B"
                            border.color: (sr.hotkey && sr.hotkey.length > 0) ? "#8E2DE2" : "#33373B"
                            border.width: 1

                            Text {
                                id: badgeText
                                anchors.centerIn: parent
                                text: (sr.hotkey && sr.hotkey.length > 0) ? sr.hotkey : qsTr("None")
                                color: (sr.hotkey && sr.hotkey.length > 0) ? "#FFFFFF" : "#777777"
                                font.bold: (sr.hotkey && sr.hotkey.length > 0)
                                font.pointSize: 9.5
                            }
                        }

                        Button {
                            text: qsTr("Change")
                            Layout.preferredHeight: 32
                            Layout.preferredWidth: 70
                            background: Rectangle {
                                color: parent.hovered ? "#9D3BF0" : "#8E2DE2"
                                radius: 5
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 9
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: root.actionHotkeyPromptRequested(sr.actionKey)
                        }

                        Button {
                            text: qsTr("Clear")
                            Layout.preferredHeight: 32
                            Layout.preferredWidth: 60
                            visible: sr.hotkey && sr.hotkey.length > 0
                            background: Rectangle {
                                color: parent.hovered ? "#3E1E22" : "#2A181A"
                                radius: 5
                                border.color: "#8B2635"
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FF8A96"
                                font.bold: true
                                font.pointSize: 9
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: sr.clearRequested()
                        }
                    }

                    ColumnLayout {
                        id: shortcutsCol
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Text {
                            text: qsTr("Global & Top Bar Action Shortcuts")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        Text {
                            text: qsTr("Configure keyboard shortcuts for top bar actions, project management, soundboard tabs, and live production controls.")
                            color: "#AAAAAA"
                            font.pointSize: 9.5
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        // --- Production & Audio Controls ---
                        Text {
                            text: qsTr("Production & Audio Controls")
                            color: "#C084FC"
                            font.bold: true
                            font.pointSize: 9.5
                            Layout.topMargin: 4
                        }

                        ShortcutRow {
                            iconSource: "../icons/stopAll.png"
                            iconColor: "#962D22"
                            title: qsTr("Stop All Sounds (Panic Button)")
                            subtitle: qsTr("Immediately halts playback of all active sounds on every tab.")
                            hotkey: typeof projectController !== "undefined" ? projectController.stopAllHotkey : ""
                            actionKey: "stopAll"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.stopAllHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/duck.png"
                            iconColor: "#F39C12"
                            title: qsTr("Toggle Audio Ducking")
                            subtitle: qsTr("Ducks playing sound volume down to the configured attenuation ratio.")
                            hotkey: typeof projectController !== "undefined" ? projectController.duckingHotkey : ""
                            actionKey: "ducking"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.duckingHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/Key.png"
                            iconColor: "#2C353D"
                            title: qsTr("Toggle Hotkey Listening (Mute Mode)")
                            subtitle: qsTr("Temporarily mutes/unmutes sound tile shortcuts while typing notes or chat.")
                            hotkey: typeof projectController !== "undefined" ? projectController.muteHotkeysHotkey : ""
                            actionKey: "muteHotkeys"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.muteHotkeysHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconText: "🎙️"
                            iconColor: "#2F2344"
                            title: qsTr("Configure Push-to-Talk (PTT)")
                            subtitle: qsTr("Opens the Push-to-Talk and microphone automation dialog.")
                            hotkey: typeof projectController !== "undefined" ? projectController.pttDialogHotkey : ""
                            actionKey: "pttDialog"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.pttDialogHotkey = "";
                            }
                        }

                        // --- Soundboard Tab Controls ---
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                            Layout.topMargin: 4
                            Layout.bottomMargin: 2
                        }

                        Text {
                            text: qsTr("Soundboard Tab Controls")
                            color: "#C084FC"
                            font.bold: true
                            font.pointSize: 9.5
                        }

                        ShortcutRow {
                            iconSource: "../icons/newTab.png"
                            iconColor: "#2C353D"
                            title: qsTr("Add Soundboard Tab")
                            subtitle: qsTr("Creates a new soundboard bank tab in the current workspace.")
                            hotkey: typeof projectController !== "undefined" ? projectController.newTabHotkey : ""
                            actionKey: "newTab"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.newTabHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/import_directory.png"
                            iconColor: "#2C2340"
                            title: qsTr("Import Directory as Soundboard")
                            subtitle: qsTr("Discovers audio files in a folder and creates a populated soundboard tab.")
                            hotkey: typeof projectController !== "undefined" ? projectController.importDirectoryHotkey : ""
                            actionKey: "importDirectory"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.importDirectoryHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/rename.png"
                            iconColor: "#2C353D"
                            title: qsTr("Rename Current Tab")
                            subtitle: qsTr("Opens the prompt to rename the currently active soundboard tab.")
                            hotkey: typeof projectController !== "undefined" ? projectController.renameTabHotkey : ""
                            actionKey: "renameTab"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.renameTabHotkey = "";
                            }
                        }

                        // --- Project & File Controls ---
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                            Layout.topMargin: 4
                            Layout.bottomMargin: 2
                        }

                        Text {
                            text: qsTr("Project & File Controls")
                            color: "#C084FC"
                            font.bold: true
                            font.pointSize: 9.5
                        }

                        ShortcutRow {
                            iconSource: "../icons/open.png"
                            iconColor: "#2C353D"
                            title: qsTr("Open Project")
                            subtitle: qsTr("Opens an existing .caster project file from disk.")
                            hotkey: typeof projectController !== "undefined" ? projectController.openProjectHotkey : ""
                            actionKey: "openProject"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.openProjectHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/save.png"
                            iconColor: "#2C353D"
                            title: qsTr("Save Project")
                            subtitle: qsTr("Saves the active soundboard project to disk.")
                            hotkey: typeof projectController !== "undefined" ? projectController.saveProjectHotkey : ""
                            actionKey: "saveProject"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.saveProjectHotkey = "";
                            }
                        }

                        ShortcutRow {
                            iconSource: "../icons/save_as.png"
                            iconColor: "#2C353D"
                            title: qsTr("Save Project As...")
                            subtitle: qsTr("Saves the current soundboard project under a new file path.")
                            hotkey: typeof projectController !== "undefined" ? projectController.saveAsProjectHotkey : ""
                            actionKey: "saveAsProject"
                            onClearRequested: {
                                if (typeof projectController !== "undefined") projectController.saveAsProjectHotkey = "";
                            }
                        }
                    }
                }

                // Web Remote Control Server Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: webCol.implicitHeight + 32
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: webCol
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 14

                        RowLayout {
                            spacing: 12
                            Layout.fillWidth: true

                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: (typeof webRemoteController !== "undefined" && webRemoteController.isRunning) ? "#2ECC71" : "#7E8590"
                            }

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Web Remote Control Server")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 13
                                }
                                Text {
                                    text: (typeof webRemoteController !== "undefined" && webRemoteController.isRunning)
                                          ? qsTr("Active on %1 (%2 connected)").arg(webRemoteController.localUrl).arg(webRemoteController.connectedClients)
                                          : qsTr("Control soundboards, ducking, and panic-stop from any smartphone, tablet, or browser.")
                                    color: (typeof webRemoteController !== "undefined" && webRemoteController.isRunning) ? "#4EBA6F" : "#888888"
                                    font.pointSize: 9.5
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            Button {
                                text: (typeof webRemoteController !== "undefined" && webRemoteController.isRunning) ? qsTr("Stop Server") : qsTr("Start Server")
                                Layout.preferredHeight: 36
                                Layout.preferredWidth: 120
                                background: Rectangle {
                                    color: (typeof webRemoteController !== "undefined" && webRemoteController.isRunning) ? "#8B2635" : "#8E2DE2"
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9.5
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (typeof webRemoteController !== "undefined") {
                                        webRemoteController.toggleServer();
                                    }
                                }
                            }
                        }

                        // URL list & quick copy when running
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            radius: 6
                            color: "#181A1D"
                            border.color: "#33383E"
                            visible: typeof webRemoteController !== "undefined" && webRemoteController.isRunning

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 10

                                Text {
                                    text: qsTr("Connect browser to:")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                }

                                Text {
                                    text: (typeof webRemoteController !== "undefined" && webRemoteController.availableUrls.length > 0)
                                          ? webRemoteController.availableUrls[0] : (typeof webRemoteController !== "undefined" ? webRemoteController.localUrl : "")
                                    color: "#00D2FF"
                                    font.bold: true
                                    font.family: "Monospace"
                                    font.pointSize: 9.5
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Button {
                                    text: qsTr("Copy URL")
                                    Layout.preferredHeight: 28
                                    Layout.preferredWidth: 76
                                    onClicked: {
                                        if (typeof projectController !== "undefined" && typeof webRemoteController !== "undefined") {
                                            var url = webRemoteController.availableUrls.length > 0 ? webRemoteController.availableUrls[0] : webRemoteController.localUrl;
                                            projectController.copyToClipboard(url);
                                        }
                                    }
                                    background: Rectangle {
                                        color: parent.hovered ? "#383E44" : "#282C32"
                                        radius: 4
                                        border.color: "#4A525D"
                                        border.width: 1
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 8.5
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        // Form grid: Port, Bind Address, User, Password
                        GridLayout {
                            columns: 4
                            columnSpacing: 14
                            rowSpacing: 8
                            Layout.fillWidth: true

                            Text {
                                text: qsTr("Port:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof webRemoteController !== "undefined" ? String(webRemoteController.port) : "5052"
                                Layout.preferredWidth: 100
                                enabled: typeof webRemoteController !== "undefined" && !webRemoteController.isRunning
                                onEditingFinished: {
                                    if (typeof webRemoteController !== "undefined") {
                                        var p = parseInt(text);
                                        if (!isNaN(p)) webRemoteController.port = p;
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Bind Address:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof webRemoteController !== "undefined" ? webRemoteController.bindAddress : "0.0.0.0"
                                Layout.fillWidth: true
                                enabled: typeof webRemoteController !== "undefined" && !webRemoteController.isRunning
                                onEditingFinished: {
                                    if (typeof webRemoteController !== "undefined") {
                                        webRemoteController.bindAddress = text.trim();
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Username:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof webRemoteController !== "undefined" ? webRemoteController.username : "caster"
                                Layout.preferredWidth: 140
                                onEditingFinished: {
                                    if (typeof webRemoteController !== "undefined") {
                                        webRemoteController.username = text.trim();
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Password:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof webRemoteController !== "undefined" ? webRemoteController.password : "caster"
                                Layout.fillWidth: true
                                onEditingFinished: {
                                    if (typeof webRemoteController !== "undefined") {
                                        webRemoteController.password = text;
                                    }
                                }
                            }
                        }

                        // Auto-start Checkbox
                        RowLayout {
                            spacing: 8
                            CheckBox {
                                id: autoStartWebCb
                                text: qsTr("Automatically start Web Remote Server on application startup")
                                checked: typeof webRemoteController !== "undefined" ? webRemoteController.autoStart : false
                                onToggled: {
                                    if (typeof webRemoteController !== "undefined") {
                                        webRemoteController.autoStart = checked;
                                    }
                                }
                            }
                        }
                    }
                }

                // Model Context Protocol (MCP) Server Setting
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: mcpContentCol.implicitHeight + 32
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: mcpContentCol
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 14

                        RowLayout {
                            spacing: 12
                            Layout.fillWidth: true

                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: (typeof mcpServerController !== "undefined" && mcpServerController.isRunning) ? "#2ECC71" : "#7E8590"
                            }

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Model Context Protocol (MCP) AI Server")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 13
                                }
                                Text {
                                    text: (typeof mcpServerController !== "undefined" && mcpServerController.isRunning)
                                          ? qsTr("Active on %1 (%2 client%3 connected &bull; %4 tool calls)")
                                                .arg(mcpServerController.sseUrl)
                                                .arg(mcpServerController.connectedClients)
                                                .arg(mcpServerController.connectedClients === 1 ? "" : "s")
                                                .arg(mcpServerController.toolInvocationsCount)
                                          : qsTr("Exposes native AI tools for Antigravity, Claude Desktop, Cursor, and LLM assistants.")
                                    color: (typeof mcpServerController !== "undefined" && mcpServerController.isRunning) ? "#4EBA6F" : "#888888"
                                    font.pointSize: 9.5
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            Button {
                                text: (typeof mcpServerController !== "undefined" && mcpServerController.isRunning) ? qsTr("Stop Server") : qsTr("Start Server")
                                Layout.preferredHeight: 36
                                Layout.preferredWidth: 120
                                background: Rectangle {
                                    color: (typeof mcpServerController !== "undefined" && mcpServerController.isRunning) ? "#8B2635" : "#8E2DE2"
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9.5
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (typeof mcpServerController !== "undefined") {
                                        mcpServerController.toggleServer();
                                    }
                                }
                            }
                        }

                        // SSE Endpoint box & Quick Copy
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            radius: 6
                            color: "#181A1D"
                            border.color: "#33383E"
                            visible: typeof mcpServerController !== "undefined" && mcpServerController.isRunning

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 10

                                Text {
                                    text: qsTr("SSE Endpoint:")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                }

                                Text {
                                    text: typeof mcpServerController !== "undefined" ? mcpServerController.sseUrl : ""
                                    color: "#10B981"
                                    font.bold: true
                                    font.family: "Monospace"
                                    font.pointSize: 9.5
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Button {
                                    text: qsTr("Copy SSE URL")
                                    Layout.preferredHeight: 28
                                    Layout.preferredWidth: 96
                                    onClicked: {
                                        if (typeof projectController !== "undefined" && typeof mcpServerController !== "undefined") {
                                            projectController.copyToClipboard(mcpServerController.sseUrl);
                                        }
                                    }
                                    background: Rectangle {
                                        color: parent.hovered ? "#383E44" : "#282C32"
                                        radius: 4
                                        border.color: "#4A525D"
                                        border.width: 1
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 8.5
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        // Port & Bind Address inputs
                        GridLayout {
                            columns: 4
                            columnSpacing: 14
                            rowSpacing: 8
                            Layout.fillWidth: true

                            Text {
                                text: qsTr("Port:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof mcpServerController !== "undefined" ? String(mcpServerController.port) : "5055"
                                Layout.preferredWidth: 100
                                enabled: typeof mcpServerController !== "undefined" && !mcpServerController.isRunning
                                onEditingFinished: {
                                    if (typeof mcpServerController !== "undefined") {
                                        var p = parseInt(text);
                                        if (!isNaN(p)) mcpServerController.port = p;
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Bind Address:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                            }
                            DarkTextField {
                                text: typeof mcpServerController !== "undefined" ? mcpServerController.bindAddress : "127.0.0.1"
                                Layout.fillWidth: true
                                enabled: typeof mcpServerController !== "undefined" && !mcpServerController.isRunning
                                onEditingFinished: {
                                    if (typeof mcpServerController !== "undefined") {
                                        mcpServerController.bindAddress = text.trim();
                                    }
                                }
                            }
                        }

                        // Last tool invoked indicator
                        RowLayout {
                            spacing: 8
                            visible: typeof mcpServerController !== "undefined" && mcpServerController.toolInvocationsCount > 0

                            Text {
                                text: qsTr("Last Invoked Tool:")
                                color: "#8E96A0"
                                font.pointSize: 9
                            }
                            Rectangle {
                                color: "#338E2DE2"
                                border.color: "#8E2DE2"
                                radius: 4
                                implicitHeight: 22
                                implicitWidth: toolLabel.implicitWidth + 12

                                Text {
                                    id: toolLabel
                                    anchors.centerIn: parent
                                    text: typeof mcpServerController !== "undefined" ? mcpServerController.lastInvokedTool : ""
                                    color: "#D8B4FE"
                                    font.bold: true
                                    font.family: "Monospace"
                                    font.pointSize: 9
                                }
                            }
                        }

                        // Auto-start Checkbox
                        RowLayout {
                            spacing: 8
                            CheckBox {
                                id: autoStartMcpCb
                                text: qsTr("Automatically start MCP Server on application startup")
                                checked: typeof mcpServerController !== "undefined" ? mcpServerController.autoStart : false
                                onToggled: {
                                    if (typeof mcpServerController !== "undefined") {
                                        mcpServerController.autoStart = checked;
                                    }
                                }
                            }
                        }
                    }
                }

                // About card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6

                        Text {
                            text: qsTr("About Caster Soundboard")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }
                        Text {
                            text: qsTr("An open-source hotkeyed soundboard engineered for live broadcasting, podcasting, gaming, and theater productions.")
                            color: "#AAAAAA"
                            font.pointSize: 10
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        Text {
                            text: qsTr("Licensed under GNU Lesser General Public License (LGPL v3).")
                            color: "#666666"
                            font.pointSize: 9
                        }
                    }
                }
            }
        }
    }
}
