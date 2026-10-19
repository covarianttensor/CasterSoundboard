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
        sequence: (typeof projectController !== "undefined" && projectController.newTabHotkey && projectController.newTabHotkey.length > 0)
                  ? projectController.newTabHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController && projectController.tabModel) {
                projectController.tabModel.addTab();
            }
        }
    }

    Shortcut {
        sequence: (typeof projectController !== "undefined" && projectController.importDirectoryHotkey && projectController.importDirectoryHotkey.length > 0)
                  ? projectController.importDirectoryHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: importDirectoryDialog.openDialog()
    }

    Shortcut {
        sequence: (typeof projectController !== "undefined" && projectController.renameTabHotkey && projectController.renameTabHotkey.length > 0)
                  ? projectController.renameTabHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: {
            if (sidebar.currentIndex === 0) {
                soundboardsView.requestRenameCurrentTab();
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
        sequence: (typeof projectController !== "undefined" && projectController.openProjectHotkey && projectController.openProjectHotkey.length > 0)
                  ? projectController.openProjectHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: openDialog.open()
    }

    Shortcut {
        sequence: (typeof projectController !== "undefined" && projectController.saveProjectHotkey && projectController.saveProjectHotkey.length > 0)
                  ? projectController.saveProjectHotkey : ""
        enabled: sequence.length > 0
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
        sequence: (typeof projectController !== "undefined" && projectController.saveAsProjectHotkey && projectController.saveAsProjectHotkey.length > 0)
                  ? projectController.saveAsProjectHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: saveAsDialog.open()
    }

    Shortcut {
        sequence: (typeof projectController !== "undefined" && projectController.pttDialogHotkey && projectController.pttDialogHotkey.length > 0)
                  ? projectController.pttDialogHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: pttDialog.open()
    }

    Shortcut {
        sequence: (typeof projectController !== "undefined" && projectController.muteHotkeysHotkey && projectController.muteHotkeysHotkey.length > 0)
                  ? projectController.muteHotkeysHotkey : ""
        enabled: sequence.length > 0
        context: Qt.ApplicationShortcut
        onActivated: {
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.toggleMute();
            }
        }
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
                if (projectController.isLegacyFile(path)) {
                    legacyImportDialog.openForFile(path);
                } else {
                    projectController.loadProject(path);
                }
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
        implicitWidth: 520
        width: Math.min(520, parent ? parent.width - 40 : 520)
        topPadding: 16
        bottomPadding: 16
        leftPadding: 24
        rightPadding: 24

        property string actionKey: ""
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
            } else if (action === "openProject") {
                actionTitle = qsTr("Open Project (.caster)");
                defaultKey = "Ctrl+O";
                pendingSequence = (projectController && projectController.openProjectHotkey) ? projectController.openProjectHotkey : "Ctrl+O";
            } else if (action === "saveProject") {
                actionTitle = qsTr("Save Project");
                defaultKey = "Ctrl+S";
                pendingSequence = (projectController && projectController.saveProjectHotkey) ? projectController.saveProjectHotkey : "Ctrl+S";
            } else if (action === "saveAsProject") {
                actionTitle = qsTr("Save Project As...");
                defaultKey = "Ctrl+Shift+S";
                pendingSequence = (projectController && projectController.saveAsProjectHotkey) ? projectController.saveAsProjectHotkey : "Ctrl+Shift+S";
            } else if (action === "newTab") {
                actionTitle = qsTr("Add Soundboard Tab");
                defaultKey = "Ctrl+N";
                pendingSequence = (projectController && projectController.newTabHotkey) ? projectController.newTabHotkey : "Ctrl+N";
            } else if (action === "importDirectory") {
                actionTitle = qsTr("Import Directory as Soundboard");
                defaultKey = "Ctrl+I";
                pendingSequence = (projectController && projectController.importDirectoryHotkey) ? projectController.importDirectoryHotkey : "Ctrl+I";
            } else if (action === "renameTab") {
                actionTitle = qsTr("Rename Current Tab");
                defaultKey = "F2";
                pendingSequence = (projectController && projectController.renameTabHotkey) ? projectController.renameTabHotkey : "F2";
            } else if (action === "pttDialog") {
                actionTitle = qsTr("Configure Push-to-Talk (PTT)");
                defaultKey = "Ctrl+Shift+P";
                pendingSequence = (projectController && projectController.pttDialogHotkey) ? projectController.pttDialogHotkey : "Ctrl+Shift+P";
            } else if (action === "muteHotkeys") {
                actionTitle = qsTr("Toggle Hotkey Mute (Active / Muted)");
                defaultKey = "Ctrl+M";
                pendingSequence = (projectController && projectController.muteHotkeysHotkey) ? projectController.muteHotkeysHotkey : "Ctrl+M";
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
            } else if (actionKey === "openProject") {
                projectController.openProjectHotkey = pendingSequence;
            } else if (actionKey === "saveProject") {
                projectController.saveProjectHotkey = pendingSequence;
            } else if (actionKey === "saveAsProject") {
                projectController.saveAsProjectHotkey = pendingSequence;
            } else if (actionKey === "newTab") {
                projectController.newTabHotkey = pendingSequence;
            } else if (actionKey === "importDirectory") {
                projectController.importDirectoryHotkey = pendingSequence;
            } else if (actionKey === "renameTab") {
                projectController.renameTabHotkey = pendingSequence;
            } else if (actionKey === "pttDialog") {
                projectController.pttDialogHotkey = pendingSequence;
            } else if (actionKey === "muteHotkeys") {
                projectController.muteHotkeysHotkey = pendingSequence;
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
            border.color: "#3B424D"
            border.width: 1
        }

        header: Rectangle {
            implicitHeight: 48
            color: "#181A1D"
            radius: 10
            border.color: "#3B424D"
            border.width: 1

            // Flatten bottom corners to seamlessly merge with dialog body
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 10
                color: "#181A1D"
            }

            // Bottom border divider
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#2E333B"
            }

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
            implicitWidth: 472
            implicitHeight: actionCol.implicitHeight
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
                id: actionCol
                anchors.fill: parent
                spacing: 12

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
                            elide: Text.ElideRight
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
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        footer: Rectangle {
            implicitHeight: 54
            color: "#181A1D"
            radius: 10
            border.color: "#3B424D"
            border.width: 1

            // Flatten top corners to seamlessly merge with dialog body
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 10
                color: "#181A1D"
            }

            // Top border divider
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#2E333B"
            }

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

    // Push-to-Talk (PTT) Automation Dialog (Issue #24)
    PttDialog {
        id: pttDialog
    }

    // Legacy Profile Import Dialog (Issue #23)
    LegacyImportDialog {
        id: legacyImportDialog
    }

    // Directory Import Dialog (Issue #26)
    ImportDirectoryDialog {
        id: importDirectoryDialog
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
                onImportDirectoryRequested: importDirectoryDialog.openDialog()
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
                onOpenPttRequested: pttDialog.open()
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
                onChangeShortcutRequested: (action) => openActionHotkeyPrompt(action)
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
                    onImportDirectoryRequested: importDirectoryDialog.openDialog()
                    onConfigureTabPttRequested: (index) => {
                        if (projectController && projectController.tabModel) {
                            pttDialog.openForTab(projectController.tabModel.tabNameAt(index));
                        } else {
                            pttDialog.open();
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

                Views.AboutView {
                    id: aboutView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onOpenDocumentationRequested: sidebar.currentIndex = 4
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
        if (typeof legacyStartupFile !== "undefined" && legacyStartupFile.length > 0) {
            legacyImportDialog.openForFile(legacyStartupFile);
        } else if (typeof openLegacyImportPromptOnStart !== "undefined" && openLegacyImportPromptOnStart) {
            legacyImportDialog.openForFile("retro_sounds.caster");
        } else if (typeof openPttPromptOnStart !== "undefined" && openPttPromptOnStart) {
            pttDialog.open();
        } else if (typeof openImportDirPromptOnStart !== "undefined" && openImportDirPromptOnStart) {
            importDirectoryDialog.openDialog(typeof importDirInitialPath !== "undefined" ? importDirInitialPath : "");
        }
    }
}
