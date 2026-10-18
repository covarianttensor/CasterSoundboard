import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var tabModel: null
    property int currentIndex: tabModel ? tabModel.currentIndex : 0
    signal addTabRequested()
    signal importDirectoryRequested()
    signal cloneTabRequested(int index)
    signal closeTabRequested(int index)
    signal renameTabRequested(int index)
    signal setTabHotkeyRequested(int index)
    signal configureTabPttRequested(int index)

    height: 48
    implicitHeight: 48
    Layout.preferredHeight: 48
    color: "#181A1B"

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        contentWidth: rowLayout.implicitWidth
        contentHeight: parent.height
        clip: true
        flickableDirection: Flickable.HorizontalFlick

        RowLayout {
            id: rowLayout
            height: parent.height
            spacing: 4

            Repeater {
                model: root.tabModel

                delegate: Rectangle {
                    id: tabItem
                    required property int index
                    required property string name
                    required property string hotkey

                    property bool isActive: root.currentIndex === index
                    property string effectiveHotkey: (hotkey && hotkey.length > 0) ? hotkey : (index < 9 ? ("Alt+" + (index + 1)) : "")
                    property string pttBadgeText: {
                        if (typeof projectController === "undefined" || !projectController.pttController) return "";
                        var rev = projectController.pttController.revision;
                        return projectController.pttController.activeBadgeForTab(tabItem.name);
                    }
                    property bool hasPttBadge: pttBadgeText.length > 0
                    readonly property bool isTransmitting: {
                        if (typeof projectController === "undefined" || !projectController.pttController) return false;
                        var tx = projectController.pttController.isTransmitting;
                        if (!tx) return false;
                        return projectController.pttController.isTransmittingForTab(tabItem.name);
                    }

                    implicitWidth: Math.max(140, tabLabel.implicitWidth + (hasPttBadge ? (pttBadge.implicitWidth + 8) : 0) + (effectiveHotkey.length > 0 ? (hotkeyBadge.implicitWidth + 10) : 0) + (showCloseBtn ? 48 : 24))
                    implicitHeight: 38
                    radius: 6
                    color: isActive ? "#2D3035" : (tabMouse.containsMouse ? "#24272B" : "#1E2023")
                    border.color: tabItem.isTransmitting ? "#2ECC71" : (isActive ? "#8E2DE2" : "#33363B")
                    border.width: (tabItem.isTransmitting || isActive) ? 2 : 1

                    // Transmitting Pulse Glow on the tab card itself (matches SoundTile style)
                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: "transparent"
                        border.color: "#2ECC71"
                        border.width: 2
                        visible: tabItem.isTransmitting
                        opacity: 0.8
                        z: 3

                        SequentialAnimation on opacity {
                            running: tabItem.isTransmitting
                            loops: Animation.Infinite
                            NumberAnimation { from: 0.3; to: 0.9; duration: 500; easing.type: Easing.InOutQuad }
                            NumberAnimation { from: 0.9; to: 0.3; duration: 500; easing.type: Easing.InOutQuad }
                        }
                    }

                    property bool showCloseBtn: root.tabModel ? root.tabModel.count > 1 : false

                    // Main Tab Click & Right-click MouseArea
                    // Restricted to not overlap the action badge, hotkey badge, or close button
                    MouseArea {
                        id: tabMouse
                        anchors.fill: parent
                        anchors.rightMargin: (tabItem.showCloseBtn ? 32 : 0) + (tabItem.effectiveHotkey.length > 0 ? (hotkeyBadge.width + 8) : 0) + (tabItem.hasPttBadge ? (pttBadge.width + 8) : 0)
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: (mouse) => {
                            if (mouse.button === Qt.LeftButton) {
                                if (root.tabModel) {
                                    root.tabModel.setCurrentIndex(tabItem.index);
                                }
                            } else if (mouse.button === Qt.RightButton) {
                                tabContextMenu.popup();
                            }
                        }
                        onDoubleClicked: root.renameTabRequested(tabItem.index)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 6
                        spacing: 6

                        Text {
                            id: tabLabel
                            text: tabItem.name
                            color: tabItem.isTransmitting ? "#A7F3D0" : (tabItem.isActive ? "#FFFFFF" : "#AAAAAA")
                            font.bold: tabItem.isActive || tabItem.isTransmitting
                            font.pointSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                        }

                        // PTT Indicator Badge (Push-to-Talk / Mic Mute) with live transmitting state and glow
                        Rectangle {
                            id: pttBadge
                            visible: tabItem.hasPttBadge
                            implicitWidth: pttBadgeLayout.implicitWidth + 14
                            implicitHeight: 22
                            Layout.preferredWidth: implicitWidth
                            Layout.preferredHeight: 22
                            Layout.alignment: Qt.AlignVCenter
                            radius: 4
                            z: 4
                            color: tabItem.isTransmitting ? "#064E3B" : (tabItem.isActive ? "#3D1E4E" : "#281D3B")
                            border.color: tabItem.isTransmitting ? "#2ECC71" : "#8E2DE2"
                            border.width: tabItem.isTransmitting ? 2 : 1

                            // Dedicated Transmitting Glow Overlay (matches SoundTile style)
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: -3
                                radius: 7
                                color: "transparent"
                                border.color: "#2ECC71"
                                border.width: 2
                                visible: tabItem.isTransmitting
                                opacity: 0.8
                                z: 5

                                SequentialAnimation on opacity {
                                    running: tabItem.isTransmitting
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 0.2; to: 1.0; duration: 400; easing.type: Easing.InOutQuad }
                                    NumberAnimation { from: 1.0; to: 0.2; duration: 400; easing.type: Easing.InOutQuad }
                                }
                            }

                            // Idle purple pulse (when NOT transmitting)
                            Rectangle {
                                anchors.fill: parent
                                radius: 4
                                color: "transparent"
                                border.color: "#D946EF"
                                border.width: 1
                                visible: !tabItem.isTransmitting
                                opacity: 0.4

                                SequentialAnimation on opacity {
                                    running: pttBadge.visible && !tabItem.isTransmitting
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 0.2; to: 0.85; duration: 1000; easing.type: Easing.InOutQuad }
                                    NumberAnimation { from: 0.85; to: 0.2; duration: 1000; easing.type: Easing.InOutQuad }
                                }
                            }

                            RowLayout {
                                id: pttBadgeLayout
                                anchors.centerIn: parent
                                spacing: 4

                                Text {
                                    text: tabItem.isTransmitting ? "🟢" : "🎙️"
                                    font.pixelSize: tabItem.isTransmitting ? 9 : 9
                                    color: tabItem.isTransmitting ? "#2ECC71" : "#D946EF"
                                }

                                Text {
                                    text: tabItem.isTransmitting 
                                          ? qsTr("TRANSMITTING (%1)").arg(tabItem.pttBadgeText.replace("PTT: ", "").replace("Mute: ", "")) 
                                          : tabItem.pttBadgeText
                                    color: tabItem.isTransmitting ? "#FFFFFF" : "#F0ABFC"
                                    font.pointSize: 8
                                    font.bold: true
                                }
                            }

                            MouseArea {
                                id: pttBadgeMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.configureTabPttRequested(tabItem.index)
                                ToolTip.visible: containsMouse
                                ToolTip.text: tabItem.isTransmitting
                                    ? qsTr("PTT key is currently held down! (%1)").arg(tabItem.pttBadgeText)
                                    : qsTr("PTT: %1 (Click to configure Tab PTT)").arg(tabItem.pttBadgeText)
                            }
                        }

                        // Hotkey Indicator Badge
                        Rectangle {
                            id: hotkeyBadge
                            visible: tabItem.effectiveHotkey.length > 0
                            implicitWidth: tabHotkeyText.implicitWidth + 10
                            implicitHeight: 20
                            Layout.preferredWidth: implicitWidth
                            Layout.preferredHeight: 20
                            Layout.alignment: Qt.AlignVCenter
                            radius: 4
                            z: 2
                            color: tabItem.isActive ? "#3D275A" : (badgeMouse.containsMouse ? "#2F343B" : "#22252A")
                            border.color: tabItem.isActive ? "#A855F7" : (badgeMouse.containsMouse ? "#6E7684" : "#3E434D")
                            border.width: 1

                            Text {
                                id: tabHotkeyText
                                anchors.centerIn: parent
                                text: tabItem.effectiveHotkey
                                color: tabItem.isActive ? "#E0C3FC" : "#9AA0A6"
                                font.pointSize: 8
                                font.bold: true
                            }

                            MouseArea {
                                id: badgeMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.setTabHotkeyRequested(tabItem.index)
                                ToolTip.visible: containsMouse
                                ToolTip.text: (tabItem.hotkey && tabItem.hotkey.length > 0)
                                    ? qsTr("Custom hotkey: %1 (Click to change)").arg(tabItem.hotkey)
                                    : qsTr("Default shortcut: %1 (Click to customize)").arg(tabItem.effectiveHotkey)
                            }
                        }

                        // Close Tab Button (X)
                        Button {
                            id: closeBtn
                            visible: tabItem.showCloseBtn
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            Layout.alignment: Qt.AlignVCenter
                            flat: true
                            z: 2
                            onClicked: root.closeTabRequested(tabItem.index)

                            background: Rectangle {
                                color: closeBtn.hovered ? "#E74C3C" : (closeBtn.pressed ? "#C0392B" : "transparent")
                                radius: 12
                            }

                            contentItem: Text {
                                text: "×"
                                color: closeBtn.hovered ? "#FFFFFF" : "#888888"
                                font.bold: true
                                font.pointSize: 15
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Delete Tab '%1'").arg(tabItem.name)
                        }
                    }

                    // Tab Context Menu (Right Click)
                    Menu {
                        id: tabContextMenu

                        MenuItem {
                            text: qsTr("Set Hotkey...")
                            onTriggered: root.setTabHotkeyRequested(tabItem.index)
                        }

                        MenuItem {
                            text: qsTr("Clear Hotkey")
                            enabled: tabItem.hotkey && tabItem.hotkey.length > 0
                            onTriggered: {
                                if (root.tabModel) {
                                    root.tabModel.setTabHotkey(tabItem.index, "");
                                }
                            }
                        }

                        MenuItem {
                            text: qsTr("Configure Tab PTT…")
                            onTriggered: root.configureTabPttRequested(tabItem.index)
                        }

                        MenuSeparator {}

                        MenuItem {
                            text: qsTr("Rename Tab...")
                            onTriggered: root.renameTabRequested(tabItem.index)
                        }

                        MenuItem {
                            text: qsTr("Clone Soundboard")
                            onTriggered: root.cloneTabRequested(tabItem.index)
                        }

                        MenuItem {
                            text: qsTr("Import Directory as Soundboard...")
                            onTriggered: root.importDirectoryRequested()
                        }

                        MenuItem {
                            text: qsTr("Delete Tab")
                            enabled: tabItem.showCloseBtn
                            onTriggered: root.closeTabRequested(tabItem.index)
                        }
                    }
                }
            }

            // Inline Add Tab Button (+)
            Button {
                implicitWidth: 38
                implicitHeight: 38
                flat: true
                onClicked: root.addTabRequested()

                background: Rectangle {
                    color: parent.hovered ? "#33373C" : "#1E2023"
                    radius: 6
                    border.color: "#33363B"
                    border.width: 1
                }

                contentItem: Text {
                    text: "+"
                    color: parent.hovered ? "#FFFFFF" : "#8E2DE2"
                    font.bold: true
                    font.pointSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add Soundboard Tab (Ctrl+N)")
            }
        }
    }
}
