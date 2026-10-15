import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./views" as Views

ApplicationWindow {
    id: window
    visible: true

    width: typeof projectController !== "undefined" ? projectController.windowWidth : 1024
    height: typeof projectController !== "undefined" ? projectController.windowHeight : 720
    minimumWidth: 640
    minimumHeight: 480

    title: {
        var base = (typeof projectController !== "undefined" && projectController.filePath.length > 0)
                   ? projectController.filePath : qsTr("Unsaved Project");
        var dirty = (typeof projectController !== "undefined" && projectController.isDirty) ? " *" : "";
        return base + dirty + " - Caster Soundboard";
    }

    color: "#181A1B"

    onWidthChanged: {
        if (typeof projectController !== "undefined") {
            projectController.windowWidth = width;
        }
    }
    onHeightChanged: {
        if (typeof projectController !== "undefined") {
            projectController.windowHeight = height;
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
        sequence: "Space"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController) {
                projectController.stopAllSounds();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+D"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (projectController) {
                projectController.toggleAudioDucking();
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
            window.showNormal();
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
                        // Forward rename trigger to current tab
                    }
                }
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
                }

                Views.AudioRoutingView {
                    id: audioRoutingView
                }

                Views.OscView {
                    id: oscView
                }

                Views.PreferencesView {
                    id: preferencesView
                }
            }
        }
    }
}
