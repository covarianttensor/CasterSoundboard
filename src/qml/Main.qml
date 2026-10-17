import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./views" as Views

ApplicationWindow {
    id: window
    visible: true

    width: (typeof projectController !== "undefined" && projectController.windowWidth > 0) ? projectController.windowWidth : 1024
    height: (typeof projectController !== "undefined" && projectController.windowHeight > 0) ? projectController.windowHeight : 720
    minimumWidth: 640
    minimumHeight: 480

    visibility: {
        if ((typeof fullscreenOnStart !== "undefined" && fullscreenOnStart) ||
            (typeof projectController !== "undefined" && projectController.isFullScreen)) {
            return Window.FullScreen;
        }
        if (typeof projectController !== "undefined" && projectController.isMaximized) {
            return Window.Maximized;
        }
        return Window.Windowed;
    }

    title: {
        var base = (typeof projectController !== "undefined" && projectController.filePath.length > 0)
                   ? projectController.filePath : qsTr("Unsaved Project");
        var dirty = (typeof projectController !== "undefined" && projectController.isDirty) ? " *" : "";
        return base + dirty + " - Caster Soundboard";
    }

    color: "#181A1B"

    onWidthChanged: {
        if (typeof projectController !== "undefined" && window.visibility === Window.Windowed) {
            projectController.windowWidth = width;
        }
    }
    onHeightChanged: {
        if (typeof projectController !== "undefined" && window.visibility === Window.Windowed) {
            projectController.windowHeight = height;
        }
    }
    onVisibilityChanged: {
        if (typeof projectController !== "undefined") {
            if (window.visibility === Window.Maximized) {
                projectController.isMaximized = true;
                projectController.isFullScreen = false;
            } else if (window.visibility === Window.FullScreen) {
                projectController.isFullScreen = true;
                projectController.isMaximized = false;
            } else if (window.visibility === Window.Windowed) {
                projectController.isMaximized = false;
                projectController.isFullScreen = false;
            }
        }
    }

    // --- Keyboard Shortcuts (Issues #67 & #76) ---
    Shortcut {
        sequence: "Ctrl+Tab"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController && projectController.tabModel) {
                projectController.tabModel.nextTab();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+Tab"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController && projectController.tabModel) {
                projectController.tabModel.previousTab();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+N"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController && projectController.tabModel) {
                projectController.tabModel.addTab();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+W"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController && projectController.tabModel && projectController.tabModel.count > 1) {
                soundboardsView.requestCloseTab(projectController.tabModel.currentIndex);
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+O"
        context: Qt.ApplicationShortcut
        onActivated: openDialog.open()
    }

    Shortcut {
        sequence: "Ctrl+S"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController) {
                if (projectController.filePath.length > 0) {
                    projectController.save();
                } else {
                    saveAsDialog.open();
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+S"
        context: Qt.ApplicationShortcut
        onActivated: saveAsDialog.open()
    }

    Shortcut {
        sequence: "F11"
        context: Qt.ApplicationShortcut
        onActivated: toggleFullscreen()
    }

    function toggleFullscreen() {
        if (window.visibility === Window.FullScreen) {
            if (projectController && projectController.isMaximized) {
                window.showMaximized();
            } else {
                window.showNormal();
            }
            if (projectController) projectController.isFullScreen = false;
        } else {
            window.showFullScreen();
            if (projectController) projectController.isFullScreen = true;
        }
    }

    // --- File Dialogs ---
    FileDialog {
        id: openDialog
        title: qsTr("Open Caster Soundboard Project")
        nameFilters: ["Caster Soundboard Projects (*.caster)", "All Files (*)"]
        onAccepted: {
            if (projectController) {
                var path = selectedFile.toString();
                if (path.indexOf("file://") === 0) {
                    path = path.substring(7);
                }
                projectController.loadProject(path);
            }
        }
    }

    FileDialog {
        id: saveAsDialog
        title: qsTr("Save Caster Soundboard Project")
        fileMode: FileDialog.SaveFile
        nameFilters: ["Caster Soundboard Projects (*.caster)", "All Files (*)"]
        onAccepted: {
            if (projectController) {
                var path = selectedFile.toString();
                if (path.indexOf("file://") === 0) {
                    path = path.substring(7);
                }
                if (path.indexOf(".caster") === -1) {
                    path += ".caster";
                }
                projectController.saveProject(path);
            }
        }
    }

    function openActionHotkeyPrompt(action) {
        actionHotkeyPromptDialog.openForAction(action);
    }

    // Global Action Hotkey Prompt Dialog (Stop All / Ducking)
    Dialog {
        id: actionHotkeyPromptDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        implicitWidth: 460
        topPadding: 16
        bottomPadding: 16
        leftPadding: 20
        rightPadding: 20

        property string actionKey: "" // "stopAll" or "ducking"
        property string actionTitle: ""
        property string defaultKey: ""
        property string pendingSequence: ""
        property string pendingModifiers: ""

        function openForAction(action) {
            actionKey = action;
            if (action === "stopAll") {
                actionTitle = qsTr("Stop All Sounds (Panic Button)");
                defaultKey = "Space";
                pendingSequence = (projectController && projectController.stopAllHotkey) ? projectController.stopAllHotkey : "Space";
            } else if (action === "ducking") {
                actionTitle = qsTr("Toggle Audio Ducking");
                defaultKey = "Ctrl+D";
                pendingSequence = (projectController && projectController.duckingHotkey) ? projectController.duckingHotkey : "Ctrl+D";
            }
            pendingModifiers = "";
            open();
        }

        function applyHotkey() {
            if (!projectController) {
                close();
                return;
            }
            if (actionKey === "stopAll") {
                projectController.stopAllHotkey = pendingSequence;
            } else if (actionKey === "ducking") {
                projectController.duckingHotkey = pendingSequence;
            }
            close();
        }

        onOpened: {
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.isMuted = true;
            }
            actionCaptureArea.forceActiveFocus();
        }

        onClosed: {
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.isMuted = false;
            }
        }

        background: Rectangle {
            color: "#202327"
            radius: 10
            border.color: "#343940"
            border.width: 1
        }

        header: Rectangle {
            implicitHeight: 46
            height: 46
            color: "#181A1D"
            radius: 10

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                Text {
                    text: "⌨"
                    font.pointSize: 13
                    color: "#8E2DE2"
                }

                Text {
                    text: qsTr("Register Shortcut for %1").arg(actionHotkeyPromptDialog.actionTitle)
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 11
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }

        contentItem: Item {
            id: actionCaptureArea
            implicitWidth: 420
            implicitHeight: 145
            focus: true

            Keys.priority: Keys.BeforeItem
            Keys.enabled: true
            Keys.onPressed: (event) => {
                event.accepted = true;

                if (event.key === Qt.Key_Escape && event.modifiers === Qt.NoModifier) {
                    actionHotkeyPromptDialog.close();
                    return;
                }

                if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && event.modifiers === Qt.NoModifier) {
                    actionHotkeyPromptDialog.applyHotkey();
                    return;
                }

                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (typeof hotkeyManager !== "undefined") {
                        actionHotkeyPromptDialog.pendingModifiers = hotkeyManager.modifiersToString(event.modifiers);
                    }
                    return;
                }

                if (typeof hotkeyManager !== "undefined") {
                    var seq = hotkeyManager.keySequenceToString(event.key, event.modifiers);
                    if (seq && seq.length > 0) {
                        actionHotkeyPromptDialog.pendingSequence = seq;
                        actionHotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            Keys.onReleased: (event) => {
                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (actionHotkeyPromptDialog.pendingSequence.length === 0) {
                        actionHotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: actionCaptureArea.forceActiveFocus()
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: qsTr("Press the key combination you wish to use for this global action:")
                    color: "#AAAAAA"
                    font.pointSize: 9.5
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Key Combination Display Badge
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    radius: 8
                    color: "#16181B"
                    border.color: actionHotkeyPromptDialog.pendingSequence.length > 0 ? "#8E2DE2" : (actionHotkeyPromptDialog.pendingModifiers.length > 0 ? "#A855F7" : "#2C3036")
                    border.width: actionHotkeyPromptDialog.pendingSequence.length > 0 ? 2 : 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        Text {
                            text: {
                                if (actionHotkeyPromptDialog.pendingSequence.length > 0) {
                                    return actionHotkeyPromptDialog.pendingSequence;
                                }
                                if (actionHotkeyPromptDialog.pendingModifiers.length > 0) {
                                    return actionHotkeyPromptDialog.pendingModifiers;
                                }
                                return qsTr("(Press any key or key combination...)");
                            }
                            color: actionHotkeyPromptDialog.pendingSequence.length > 0 ? "#FFFFFF" : (actionHotkeyPromptDialog.pendingModifiers.length > 0 ? "#D19CF7" : "#7E8590")
                            font.bold: actionHotkeyPromptDialog.pendingSequence.length > 0
                            font.pointSize: actionHotkeyPromptDialog.pendingSequence.length > 0 ? 14 : 11
                            Layout.fillWidth: true
                        }

                        Text {
                            text: qsTr("Default: %1").arg(actionHotkeyPromptDialog.defaultKey)
                            color: "#6B7280"
                            font.pointSize: 9
                        }
                    }
                }

                Text {
                    text: qsTr("Tip: You can use single keys (like Space, F1..F12) or modifier combos (Ctrl+D, Alt+Space, etc.)")
                    color: "#7E8590"
                    font.pointSize: 8.5
                    font.italic: true
                }
            }
        }

        footer: Rectangle {
            implicitHeight: 52
            height: 52
            color: "#181A1D"
            radius: 10

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 10

                Button {
                    text: qsTr("Clear")
                    implicitHeight: 34
                    background: Rectangle {
                        color: parent.hovered ? "#3E1E22" : "#2A181A"
                        radius: 6
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
                    onClicked: {
                        actionHotkeyPromptDialog.pendingSequence = "";
                        actionHotkeyPromptDialog.pendingModifiers = "";
                        actionHotkeyPromptDialog.applyHotkey();
                    }
                }

                Button {
                    text: qsTr("Reset Default")
                    implicitHeight: 34
                    background: Rectangle {
                        color: parent.hovered ? "#33373B" : "#24272B"
                        radius: 6
                        border.color: "#4A4E54"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#B0B5BD"
                        font.pointSize: 9
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        actionHotkeyPromptDialog.pendingSequence = actionHotkeyPromptDialog.defaultKey;
                        actionHotkeyPromptDialog.pendingModifiers = "";
                        actionHotkeyPromptDialog.applyHotkey();
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")
                    implicitHeight: 34
                    background: Rectangle {
                        color: parent.hovered ? "#33373B" : "#24272B"
                        radius: 6
                        border.color: "#4A4E54"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pointSize: 9.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: actionHotkeyPromptDialog.close()
                }

                Button {
                    text: qsTr("Set Shortcut")
                    implicitHeight: 34
                    enabled: actionHotkeyPromptDialog.pendingSequence.length > 0
                    background: Rectangle {
                        color: parent.enabled ? (parent.hovered ? "#9D3BF0" : "#8E2DE2") : "#33373B"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: parent.enabled ? "#FFFFFF" : "#666666"
                        font.bold: true
                        font.pointSize: 9.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: actionHotkeyPromptDialog.applyHotkey()
                }
            }
        }
    }

    // Soundboard Actions Dialog (Push-To-Talk & Mute) (Issue #24)
    ActionsDialog {
        id: actionsDialog
    }

    // --- Main Shell Layout ---
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Vertical Touch Navigation Sidebar
        NavSidebar {
            id: sidebar
            Layout.fillHeight: true
            onFullscreenToggled: window.toggleFullscreen()
        }

        // Main Content Area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Top Action Bar
            TopActionBar {
                id: topBar
                Layout.fillWidth: true
                onNewTabRequested: {
                    if (projectController && projectController.tabModel) {
                        projectController.tabModel.addTab();
                    }
                }
                onOpenProjectRequested: openDialog.open()
                onSaveProjectRequested: {
                    if (projectController) {
                        if (projectController.filePath.length > 0) {
                            projectController.save();
                        } else {
                            saveAsDialog.open();
                        }
                    }
                }
                onSaveAsProjectRequested: saveAsDialog.open()
                onRenameTabRequested: {
                    if (sidebar.currentIndex === 0) {
                        soundboardsView.requestRenameCurrentTab();
                    }
                }
                onOpenActionsRequested: actionsDialog.open()
                onStopAllRequested: {
                    if (projectController) {
                        projectController.stopAllSounds();
                    }
                }
                onToggleDuckingRequested: {
                    if (projectController) {
                        projectController.toggleAudioDucking();
                    }
                }
                onToggleMuteHotkeysRequested: {
                    if (typeof hotkeyManager !== "undefined") {
                        hotkeyManager.toggleMute();
                    }
                }
                onChangeStopAllShortcutRequested: openActionHotkeyPrompt("stopAll")
                onChangeDuckingShortcutRequested: openActionHotkeyPrompt("ducking")
                onTileScaleValueChanged: {
                    if (projectController) {
                        projectController.tileScale = tileScaleValue;
                    }
                }
            }

            // Stack of Views
            StackLayout {
                id: mainStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: sidebar.currentIndex

                Views.SoundboardsView {
                    id: soundboardsView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onConfigureTabActionsRequested: (index) => {
                        if (projectController && projectController.tabModel) {
                            actionsDialog.openForTab(projectController.tabModel.tabNameAt(index));
                        } else {
                            actionsDialog.open();
                        }
                    }
                }

                Views.AudioRoutingView {
                    id: audioRoutingView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Views.OscView {
                    id: oscView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Views.PreferencesView {
                    id: preferencesView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onActionHotkeyPromptRequested: (action) => openActionHotkeyPrompt(action)
                }

                Views.DocumentationView {
                    id: documentationView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    Connections {
        target: (typeof projectController !== "undefined") ? projectController : null
        function onProjectLoaded() {
            if (!projectController) return;
            if (typeof fullscreenOnStart !== "undefined" && fullscreenOnStart) {
                window.showFullScreen();
                projectController.isFullScreen = true;
                return;
            }
            if (projectController.isFullScreen) {
                window.showFullScreen();
            } else if (projectController.isMaximized) {
                window.showMaximized();
            } else {
                window.showNormal();
                if (projectController.windowWidth >= window.minimumWidth) {
                    window.width = projectController.windowWidth;
                }
                if (projectController.windowHeight >= window.minimumHeight) {
                    window.height = projectController.windowHeight;
                }
            }
        }
        function onWindowGeometryChanged() {
            if (!projectController || window.visibility !== Window.Windowed) return;
            if (Math.abs(window.width - projectController.windowWidth) > 2) {
                window.width = projectController.windowWidth;
            }
            if (Math.abs(window.height - projectController.windowHeight) > 2) {
                window.height = projectController.windowHeight;
            }
        }
    }

    Component.onCompleted: {
        var startFullscreen = (typeof fullscreenOnStart !== "undefined" && fullscreenOnStart) ||
                              (typeof projectController !== "undefined" && projectController.isFullScreen);
        if (startFullscreen) {
            window.showFullScreen();
            if (typeof projectController !== "undefined") {
                projectController.isFullScreen = true;
            }
        } else if (typeof projectController !== "undefined" && projectController.isMaximized) {
            window.showMaximized();
        } else if (typeof projectController !== "undefined") {
            if (projectController.windowWidth >= window.minimumWidth) {
                window.width = projectController.windowWidth;
            }
            if (projectController.windowHeight >= window.minimumHeight) {
                window.height = projectController.windowHeight;
            }
        }
        if (typeof initialViewIndex !== "undefined" && initialViewIndex > 0) {
            sidebar.currentIndex = initialViewIndex;
        }
        if (typeof openActionHotkeyPromptOnStart !== "undefined" && openActionHotkeyPromptOnStart.length > 0) {
            openActionHotkeyPrompt(openActionHotkeyPromptOnStart);
        }
    }
}
