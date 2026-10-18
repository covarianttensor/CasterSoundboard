import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Item {
    id: root

    property real tileScale: typeof projectController !== "undefined" ? projectController.tileScale : 1.0
    property var currentGrid: (typeof projectController !== "undefined" && projectController.tabModel)
                              ? projectController.tabModel.currentGridModel : null

    signal configureTabPttRequested(int index)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Dynamic Tab Bar
        SoundboardTabBar {
            id: tabBar
            Layout.fillWidth: true
            tabModel: typeof projectController !== "undefined" ? projectController.tabModel : null
            onAddTabRequested: {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.addTab();
                }
            }
            onCloneTabRequested: (index) => {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.cloneTab(index);
                }
            }
            onCloseTabRequested: (index) => {
                root.requestCloseTab(index);
            }
            onRenameTabRequested: (index) => {
                renameDialog.targetIndex = index;
                if (projectController && projectController.tabModel) {
                    renameField.text = projectController.tabModel.tabNameAt(index);
                }
                renameDialog.open();
            }
            onSetTabHotkeyRequested: (index) => {
                root.requestSetTabHotkey(index);
            }
            onConfigureTabPttRequested: (index) => {
                root.configureTabPttRequested(index);
            }
        }

        // Soundboard Grid Area
        Rectangle {
            id: gridArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#181A1B"
            clip: true

            // Empty State (when grid has 0 tiles)
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 14
                visible: !root.currentGrid || root.currentGrid.count === 0

                Image {
                    source: "../icons/openMusic.png"
                    sourceSize.width: 56
                    sourceSize.height: 56
                    Layout.alignment: Qt.AlignHCenter
                    opacity: 0.6
                }

                Text {
                    text: root.currentGrid
                          ? qsTr("Tab '%1' is Empty").arg(projectController.tabModel.tabNameAt(projectController.tabModel.currentIndex))
                          : qsTr("No Soundboard Selected")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: qsTr("Drag and drop audio/video files here, or add tiles manually.")
                    color: "#7E8590"
                    font.pointSize: 11
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle {
                    Layout.preferredWidth: 220
                    Layout.preferredHeight: 44
                    Layout.alignment: Qt.AlignHCenter
                    radius: 8
                    color: addBtnMouse.containsMouse ? "#3A2B50" : "#25282B"
                    border.color: "#8E2DE2"
                    border.width: 1

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8
                        Image {
                            source: "../icons/add_player.png"
                            sourceSize.width: 22
                            sourceSize.height: 22
                            Layout.preferredWidth: 22
                            Layout.preferredHeight: 22
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: qsTr("Add Sound Tile (+)")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 11
                        }
                    }

                    MouseArea {
                        id: addBtnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (root.currentGrid) {
                                root.currentGrid.appendTile();
                            }
                        }
                    }
                }
            }

            // Populated Grid of Sound Tiles with Touch-Friendly Scrollbars
            Flickable {
                id: soundboardFlickable
                objectName: "soundboardFlickable"
                anchors.fill: parent
                anchors.margins: 14
                visible: root.currentGrid && root.currentGrid.count > 0
                clip: true
                flickableDirection: Flickable.HorizontalAndVerticalFlick

                property real cellW: Math.max(160, 230 * root.tileScale)
                property real cellH: Math.max(170, 240 * root.tileScale)
                property int totalTiles: root.currentGrid ? (root.currentGrid.count + 1) : 0
                property int minCols: totalTiles >= 2 ? 2 : 1
                // Reserve 28px for vertical scrollbar to prevent cyclic layout jitter
                property int cols: Math.max(minCols, Math.floor((soundboardFlickable.width - 28) / cellW))

                readonly property bool vNeeded: soundboardGrid.height > soundboardFlickable.height
                readonly property bool hNeeded: soundboardGrid.width > (soundboardFlickable.width - (vNeeded ? 28 : 0))

                contentWidth: Math.max(soundboardFlickable.width, soundboardGrid.width + (vNeeded ? 28 : 0))
                contentHeight: Math.max(soundboardFlickable.height, soundboardGrid.height + (hNeeded ? 28 : 0))

                // Touch-Friendly Vertical ScrollBar (Oversized 28px width)
                ScrollBar.vertical: ScrollBar {
                    id: soundboardScrollbarV
                    objectName: "soundboardScrollbarV"
                    width: 28
                    policy: ScrollBar.AsNeeded
                    visible: soundboardFlickable.vNeeded
                    interactive: true
                    active: visible
                    bottomPadding: soundboardFlickable.hNeeded ? 28 : 0

                    background: Rectangle {
                        color: "#16181B"
                        radius: 6
                        border.color: "#2C3036"
                        border.width: 1
                        anchors.fill: parent
                        anchors.bottomMargin: soundboardFlickable.hNeeded ? 28 : 0
                    }

                    contentItem: Rectangle {
                        implicitWidth: 20
                        implicitHeight: 48
                        radius: 6
                        color: soundboardScrollbarV.pressed ? "#8E2DE2" : (soundboardScrollbarV.hovered ? "#5A6270" : "#3E434D")
                        border.color: soundboardScrollbarV.pressed ? "#FFFFFF" : "#555A64"
                        border.width: 1

                        // Grip Ridges for Touch Dragging
                        Column {
                            anchors.centerIn: parent
                            spacing: 3
                            Repeater {
                                model: 3
                                Rectangle {
                                    width: 10
                                    height: 2
                                    radius: 1
                                    color: soundboardScrollbarV.pressed ? "#E0C3FC" : "#7E8590"
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }
                    }
                }

                // Touch-Friendly Horizontal ScrollBar (Oversized 28px height)
                ScrollBar.horizontal: ScrollBar {
                    id: soundboardScrollbarH
                    objectName: "soundboardScrollbarH"
                    height: 28
                    policy: ScrollBar.AsNeeded
                    visible: soundboardFlickable.hNeeded
                    interactive: true
                    active: visible
                    rightPadding: soundboardFlickable.vNeeded ? 28 : 0

                    background: Rectangle {
                        color: "#16181B"
                        radius: 6
                        border.color: "#2C3036"
                        border.width: 1
                        anchors.fill: parent
                        anchors.rightMargin: soundboardFlickable.vNeeded ? 28 : 0
                    }

                    contentItem: Rectangle {
                        implicitWidth: 48
                        implicitHeight: 20
                        radius: 6
                        color: soundboardScrollbarH.pressed ? "#8E2DE2" : (soundboardScrollbarH.hovered ? "#5A6270" : "#3E434D")
                        border.color: soundboardScrollbarH.pressed ? "#FFFFFF" : "#555A64"
                        border.width: 1

                        // Grip Ridges for Touch Dragging
                        Row {
                            anchors.centerIn: parent
                            spacing: 3
                            Repeater {
                                model: 3
                                Rectangle {
                                    width: 2
                                    height: 10
                                    radius: 1
                                    color: soundboardScrollbarH.pressed ? "#E0C3FC" : "#7E8590"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }
                }

                Grid {
                    id: soundboardGrid
                    columns: soundboardFlickable.cols

                    Repeater {
                        model: soundboardFlickable.totalTiles

                        delegate: Item {
                            id: delegateRoot
                            width: soundboardFlickable.cellW
                            height: soundboardFlickable.cellH

                            readonly property bool isAddTile: index === (root.currentGrid ? root.currentGrid.count : 0)

                            // 1. Existing Sound Tile
                            SoundTile {
                                visible: !delegateRoot.isAddTile
                                anchors.centerIn: parent
                                tile: delegateRoot.isAddTile ? null : (root.currentGrid ? root.currentGrid.tileAt(index) : null)
                                scaleFactor: root.tileScale
                                onSettingsRequested: (t) => {
                                    tileDrawer.tile = t;
                                    tileDrawer.open();
                                }
                                onExtraFilesDropped: (extraUrls) => {
                                    if (root.currentGrid) {
                                        for (var i = 0; i < extraUrls.length; ++i) {
                                            var u = extraUrls[i];
                                            var newTile = root.currentGrid.appendTile();
                                            if (newTile) {
                                                newTile.filePath = u;
                                                var fn = u.substring(u.lastIndexOf('/') + 1);
                                                var dot = fn.lastIndexOf('.');
                                                if (dot > 0) fn = fn.substring(0, dot);
                                                newTile.displayName = fn;
                                            }
                                        }
                                    }
                                }
                            }

                            // 2. Add Sound Tile Card (inline in the grid flow)
                            Rectangle {
                                visible: delegateRoot.isAddTile
                                anchors.centerIn: parent
                                width: Math.max(150, 220 * root.tileScale)
                                height: Math.max(150, 230 * root.tileScale)
                                radius: 12
                                color: addDropArea.containsDrag ? "#1E824C" : (addMouse.containsMouse ? "#2A2536" : "#1F2126")
                                border.color: addDropArea.containsDrag ? "#2ECC71" : (addMouse.containsMouse ? "#8E2DE2" : "#353942")
                                border.width: addDropArea.containsDrag ? 2 : 1

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 8
                                    Rectangle {
                                        width: 42 * root.tileScale
                                        height: 42 * root.tileScale
                                        radius: 21 * root.tileScale
                                        color: (addMouse.containsMouse || addDropArea.containsDrag) ? "#8E2DE2" : "#2E3236"
                                        border.color: (addMouse.containsMouse || addDropArea.containsDrag) ? "#FFFFFF" : "#555A64"
                                        border.width: 1
                                        Layout.alignment: Qt.AlignHCenter

                                        Text {
                                            anchors.centerIn: parent
                                            text: "+"
                                            color: "#FFFFFF"
                                            font.pixelSize: Math.round(26 * root.tileScale)
                                            font.bold: true
                                        }
                                    }
                                    Text {
                                        text: addDropArea.containsDrag ? qsTr("Drop to Add New Tiles") : qsTr("Add Sound Tile")
                                        color: (addMouse.containsMouse || addDropArea.containsDrag) ? "#FFFFFF" : "#AAAAAA"
                                        font.bold: true
                                        font.pointSize: 10 * Math.max(0.8, root.tileScale * 0.9)
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }

                                DropArea {
                                    id: addDropArea
                                    anchors.fill: parent
                                    enabled: delegateRoot.isAddTile
                                    onEntered: (drag) => {
                                        if (drag.hasUrls) drag.acceptProposedAction();
                                    }
                                    onDropped: (drop) => {
                                        if (drop.hasUrls && root.currentGrid) {
                                            for (var i = 0; i < drop.urls.length; ++i) {
                                                var url = drop.urls[i].toString();
                                                if (url.indexOf("file://") === 0) url = url.substring(7);
                                                var tile = root.currentGrid.appendTile();
                                                if (tile) {
                                                    tile.filePath = url;
                                                    var filename = url.substring(url.lastIndexOf('/') + 1);
                                                    var dotIdx = filename.lastIndexOf('.');
                                                    if (dotIdx > 0) filename = filename.substring(0, dotIdx);
                                                    tile.displayName = filename;
                                                }
                                            }
                                            drop.acceptProposedAction();
                                        }
                                    }
                                }

                                MouseArea {
                                    id: addMouse
                                    anchors.fill: parent
                                    enabled: delegateRoot.isAddTile
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (root.currentGrid) {
                                            root.currentGrid.appendTile();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Corner filler when both scrollbars are visible
                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: 28
                    height: 28
                    color: "#16181B"
                    border.color: "#2C3036"
                    border.width: 1
                    visible: soundboardScrollbarV.visible && soundboardScrollbarH.visible
                    z: 10
                }
            }

            // Background drop area (for dropping onto empty grid space behind tiles)
            DropArea {
                id: backgroundDropArea
                anchors.fill: parent
                z: -1
                onEntered: (drag) => {
                    if (drag.hasUrls) drag.acceptProposedAction();
                }
                onDropped: (drop) => {
                    if (drop.hasUrls && root.currentGrid) {
                        for (var i = 0; i < drop.urls.length; ++i) {
                            var url = drop.urls[i].toString();
                            if (url.indexOf("file://") === 0) url = url.substring(7);
                            var tile = root.currentGrid.appendTile();
                            if (tile) {
                                tile.filePath = url;
                                var filename = url.substring(url.lastIndexOf('/') + 1);
                                var dotIdx = filename.lastIndexOf('.');
                                if (dotIdx > 0) filename = filename.substring(0, dotIdx);
                                tile.displayName = filename;
                            }
                        }
                        drop.acceptProposedAction();
                    }
                }
            }
        }
    }

    // Sound Tile Properties Drawer / Modal Dialog
    SoundTileDrawer {
        id: tileDrawer
        onRemoveTileRequested: (t) => {
            if (!t || !root.currentGrid) return;
            for (var i = 0; i < root.currentGrid.count; ++i) {
                if (root.currentGrid.tileAt(i) === t) {
                    root.currentGrid.removeTile(i);
                    break;
                }
            }
        }
    }

    function requestCloseTab(index) {
        if (!projectController || !projectController.tabModel) return;
        if (projectController.tabModel.count <= 1) return;
        if (index < 0 || index >= projectController.tabModel.count) return;

        deleteTabDialog.targetIndex = index;
        deleteTabDialog.tabName = projectController.tabModel.tabNameAt(index);
        var g = projectController.tabModel.gridModelAt(index);
        deleteTabDialog.tileCount = g ? g.count : 0;
        deleteTabDialog.open();
    }

    function requestRenameCurrentTab() {
        if (!projectController || !projectController.tabModel) return;
        var idx = projectController.tabModel.currentIndex;
        if (idx >= 0 && idx < projectController.tabModel.count) {
            renameDialog.targetIndex = idx;
            renameField.text = projectController.tabModel.tabNameAt(idx);
            renameDialog.open();
        }
    }

    function requestSetTabHotkey(index) {
        if (!projectController || !projectController.tabModel) return;
        if (index < 0 || index >= projectController.tabModel.count) return;

        tabHotkeyPromptDialog.targetIndex = index;
        tabHotkeyPromptDialog.tabName = projectController.tabModel.tabNameAt(index);
        tabHotkeyPromptDialog.pendingSequence = projectController.tabModel.tabHotkeyAt(index);
        tabHotkeyPromptDialog.pendingModifiers = "";
        tabHotkeyPromptDialog.open();
    }

    // Rename Tab Dialog
    Dialog {
        id: renameDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        implicitWidth: 380
        topPadding: 16
        bottomPadding: 16
        leftPadding: 20
        rightPadding: 20

        property int targetIndex: -1

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
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Rename Soundboard Tab")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 11
            }
        }

        contentItem: ColumnLayout {
            spacing: 10
            Text {
                text: qsTr("Enter new name for tab:")
                color: "#AAAAAA"
                font.pointSize: 10
            }
            TextField {
                id: renameField
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                selectByMouse: true
                color: "#FFFFFF"
                font.pointSize: 10
                background: Rectangle {
                    color: "#282C32"
                    radius: 6
                    border.color: renameField.activeFocus ? "#8E2DE2" : "#454B54"
                    border.width: renameField.activeFocus ? 2 : 1
                }
                onAccepted: {
                    renameDialog.applyRename();
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
                anchors.margins: 10
                spacing: 8
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: renameDialog.close()
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#3A3F47" : "#282C32"
                        radius: 6
                    }
                }
                Button {
                    text: qsTr("Save")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: renameDialog.applyRename()
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#9D3BF0" : "#8E2DE2"
                        radius: 6
                    }
                }
            }
        }

        function applyRename() {
            if (targetIndex >= 0 && projectController && projectController.tabModel && renameField.text.trim().length > 0) {
                projectController.tabModel.renameTab(targetIndex, renameField.text.trim());
            }
            close();
        }
    }

    // Delete Tab Confirmation Dialog
    Dialog {
        id: deleteTabDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        implicitWidth: 420
        topPadding: 20
        bottomPadding: 20
        leftPadding: 24
        rightPadding: 24

        property int targetIndex: -1
        property string tabName: ""
        property int tileCount: 0

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
                Text {
                    text: qsTr("Delete Soundboard Tab")
                    color: "#E74C3C"
                    font.bold: true
                    font.pointSize: 11
                }
            }
        }

        contentItem: ColumnLayout {
            spacing: 12

            Text {
                text: deleteTabDialog.tileCount > 0
                    ? qsTr("Are you sure you want to delete tab \"%1\" and all %2 sound tile(s) inside it?").arg(deleteTabDialog.tabName).arg(deleteTabDialog.tileCount)
                    : qsTr("Are you sure you want to delete tab \"%1\"?").arg(deleteTabDialog.tabName)
                color: "#FFFFFF"
                font.pointSize: 10
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.maximumWidth: 370
            }

            Text {
                text: qsTr("This action cannot be undone.")
                color: "#E74C3C"
                font.pointSize: 8.5
            }
        }

        footer: Rectangle {
            implicitHeight: 52
            height: 52
            color: "#181A1D"
            radius: 10

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: deleteTabDialog.close()
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#3A3F47" : "#282C32"
                        radius: 6
                    }
                }

                Button {
                    text: qsTr("Delete Tab")
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    onClicked: {
                        if (deleteTabDialog.targetIndex >= 0 && projectController && projectController.tabModel) {
                            projectController.tabModel.removeTab(deleteTabDialog.targetIndex);
                        }
                        deleteTabDialog.close();
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#C0392B" : "#E74C3C"
                        radius: 6
                    }
                }
            }
        }
    }

    // Tab Hotkey Capture Prompt Dialog
    Dialog {
        id: tabHotkeyPromptDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        implicitWidth: 460
        topPadding: 16
        bottomPadding: 16
        leftPadding: 20
        rightPadding: 20

        property int targetIndex: -1
        property string tabName: ""
        property string pendingSequence: ""
        property string pendingModifiers: ""

        onOpened: {
            if (typeof hotkeyManager !== "undefined") {
                hotkeyManager.isMuted = true;
            }
            tabCaptureArea.forceActiveFocus();
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
                    text: qsTr("Register Shortcut for Tab \"%1\"").arg(tabHotkeyPromptDialog.tabName)
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 11
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }

        contentItem: Item {
            id: tabCaptureArea
            implicitWidth: 420
            implicitHeight: 145
            focus: true

            Keys.priority: Keys.BeforeItem
            Keys.enabled: true
            Keys.onPressed: (event) => {
                event.accepted = true;

                // Escape cancels dialog
                if (event.key === Qt.Key_Escape && event.modifiers === Qt.NoModifier) {
                    tabHotkeyPromptDialog.close();
                    return;
                }

                // Enter applies sequence
                if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && event.modifiers === Qt.NoModifier) {
                    tabHotkeyPromptDialog.applyHotkey();
                    return;
                }

                // Modifier only
                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (typeof hotkeyManager !== "undefined") {
                        tabHotkeyPromptDialog.pendingModifiers = hotkeyManager.modifiersToString(event.modifiers);
                    }
                    return;
                }

                // Complete combination
                if (typeof hotkeyManager !== "undefined") {
                    var seq = hotkeyManager.keySequenceToString(event.key, event.modifiers);
                    if (seq && seq.length > 0) {
                        tabHotkeyPromptDialog.pendingSequence = seq;
                        tabHotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            Keys.onReleased: (event) => {
                if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) {
                    if (tabHotkeyPromptDialog.pendingSequence.length === 0) {
                        tabHotkeyPromptDialog.pendingModifiers = "";
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: tabCaptureArea.forceActiveFocus()
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: qsTr("Press the key combination you wish to use to switch to this soundboard tab:")
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
                    border.color: tabHotkeyPromptDialog.pendingSequence.length > 0 ? "#8E2DE2" : (tabHotkeyPromptDialog.pendingModifiers.length > 0 ? "#A855F7" : "#2C3036")
                    border.width: tabHotkeyPromptDialog.pendingSequence.length > 0 ? 2 : 1

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: {
                                if (tabHotkeyPromptDialog.pendingSequence.length > 0) {
                                    return tabHotkeyPromptDialog.pendingSequence;
                                }
                                if (tabHotkeyPromptDialog.pendingModifiers.length > 0) {
                                    return tabHotkeyPromptDialog.pendingModifiers;
                                }
                                return qsTr("Listening... Press any key or combination");
                            }
                            color: tabHotkeyPromptDialog.pendingSequence.length > 0 ? "#FFFFFF" : (tabHotkeyPromptDialog.pendingModifiers.length > 0 ? "#D19CF7" : "#7E8590")
                            font.bold: tabHotkeyPromptDialog.pendingSequence.length > 0
                            font.pointSize: 13
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: {
                            var defKey = (tabHotkeyPromptDialog.targetIndex >= 0 && tabHotkeyPromptDialog.targetIndex < 9)
                                ? qsTr("Default shortcut: Alt+%1").arg(tabHotkeyPromptDialog.targetIndex + 1)
                                : "";
                            return defKey;
                        }
                        color: "#7E8590"
                        font.pointSize: 8.5
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: qsTr("e.g. F1, F2, Alt+1, Ctrl+1")
                        color: "#606670"
                        font.pointSize: 8.5
                    }
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
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Button {
                    text: qsTr("Clear")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: {
                        tabHotkeyPromptDialog.pendingSequence = "";
                        tabHotkeyPromptDialog.pendingModifiers = "";
                        tabHotkeyPromptDialog.applyHotkey();
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#E74C3C"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#3A2828" : "#282224"
                        radius: 6
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: tabHotkeyPromptDialog.close()
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#3A3F47" : "#282C32"
                        radius: 6
                    }
                }

                Button {
                    text: qsTr("Save")
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 34
                    onClicked: tabHotkeyPromptDialog.applyHotkey()
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#9D3BF0" : "#8E2DE2"
                        radius: 6
                    }
                }
            }
        }

        function applyHotkey() {
            if (targetIndex >= 0 && projectController && projectController.tabModel) {
                projectController.tabModel.setTabHotkey(targetIndex, pendingSequence);
            }
            close();
        }
    }

    Component.onCompleted: {
        if (typeof openDeleteTabPromptOnStart !== "undefined" && openDeleteTabPromptOnStart) {
            root.requestCloseTab(1);
        }
        if (typeof openTabHotkeyPromptOnStart !== "undefined" && openTabHotkeyPromptOnStart) {
            root.requestSetTabHotkey(0);
        }
        if (typeof openDrawerOnStart !== "undefined" && openDrawerOnStart) {
            if (root.currentGrid && root.currentGrid.count > 0) {
                var idx = (typeof drawerTileIndex !== "undefined") ? drawerTileIndex : 0;
                tileDrawer.tile = root.currentGrid.tileAt(Math.max(0, Math.min(root.currentGrid.count - 1, idx)));
                tileDrawer.open();
            }
        }
    }
}
