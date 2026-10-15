import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Item {
    id: root

    property real tileScale: typeof projectController !== "undefined" ? projectController.tileScale : 1.0
    property var currentGrid: (typeof projectController !== "undefined" && projectController.tabModel)
                              ? projectController.tabModel.currentGridModel : null

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Dynamic Tab Bar
        SoundboardTabBar {
            Layout.fillWidth: true
            tabModel: typeof projectController !== "undefined" ? projectController.tabModel : null
            onAddTabRequested: {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.addTab();
                }
            }
            onCloseTabRequested: (index) => {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.removeTab(index);
                }
            }
            onRenameTabRequested: (index) => {
                renameDialog.targetIndex = index;
                if (projectController && projectController.tabModel) {
                    renameField.text = projectController.tabModel.tabNameAt(index);
                }
                renameDialog.open();
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
                property real cellH: Math.max(160, 230 * root.tileScale)
                property int totalTiles: root.currentGrid ? (root.currentGrid.count + 1) : 0
                property int minCols: totalTiles >= 2 ? 2 : 1
                property real availW: soundboardFlickable.width - (soundboardScrollbarV.visible ? soundboardScrollbarV.width : 0)
                property real availH: soundboardFlickable.height - (soundboardScrollbarH.visible ? soundboardScrollbarH.height : 0)
                property int cols: Math.max(minCols, Math.floor(availW / cellW))

                contentWidth: Math.max(availW, soundboardGrid.width)
                contentHeight: Math.max(availH, soundboardGrid.height)

                // Touch-Friendly Vertical ScrollBar (Oversized 28px width)
                ScrollBar.vertical: ScrollBar {
                    id: soundboardScrollbarV
                    objectName: "soundboardScrollbarV"
                    width: 28
                    policy: ScrollBar.AsNeeded
                    visible: size < 1.0
                    interactive: true
                    active: size < 1.0
                    onActiveChanged: {
                        if (!active && size < 1.0) active = true;
                    }
                    bottomPadding: soundboardScrollbarH.visible ? soundboardScrollbarH.height : 0

                    background: Rectangle {
                        color: "#16181B"
                        radius: 6
                        border.color: "#2C3036"
                        border.width: 1
                        anchors.fill: parent
                        anchors.bottomMargin: soundboardScrollbarH.visible ? soundboardScrollbarH.height : 0
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
                    visible: size < 1.0
                    interactive: true
                    active: size < 1.0
                    onActiveChanged: {
                        if (!active && size < 1.0) active = true;
                    }
                    rightPadding: soundboardScrollbarV.visible ? soundboardScrollbarV.width : 0

                    background: Rectangle {
                        color: "#16181B"
                        radius: 6
                        border.color: "#2C3036"
                        border.width: 1
                        anchors.fill: parent
                        anchors.rightMargin: soundboardScrollbarV.visible ? soundboardScrollbarV.width : 0
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
                                width: Math.max(150, 210 * root.tileScale)
                                height: Math.max(150, 210 * root.tileScale)
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

    // Rename Tab Dialog
    Dialog {
        id: renameDialog
        title: qsTr("Rename Soundboard Tab")
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        property int targetIndex: -1

        ColumnLayout {
            spacing: 8
            Text {
                text: qsTr("Enter new name for tab:")
                color: "#FFFFFF"
            }
            TextField {
                id: renameField
                Layout.preferredWidth: 260
                selectByMouse: true
                color: "#FFFFFF"
            }
        }

        onAccepted: {
            if (targetIndex >= 0 && projectController && projectController.tabModel) {
                projectController.tabModel.renameTab(targetIndex, renameField.text);
            }
        }
    }

    Component.onCompleted: {
        if (typeof openDrawerOnStart !== "undefined" && openDrawerOnStart) {
            if (root.currentGrid && root.currentGrid.count > 0) {
                var idx = (typeof drawerTileIndex !== "undefined") ? drawerTileIndex : 0;
                tileDrawer.tile = root.currentGrid.tileAt(Math.max(0, Math.min(root.currentGrid.count - 1, idx)));
                tileDrawer.open();
            }
        }
    }
}
