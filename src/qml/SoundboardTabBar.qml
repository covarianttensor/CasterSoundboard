import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var tabModel: null
    property int currentIndex: tabModel ? tabModel.currentIndex : 0
    signal addTabRequested()
    signal closeTabRequested(int index)
    signal renameTabRequested(int index)
    signal setTabHotkeyRequested(int index)

    height: 48
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

                    implicitWidth: Math.max(140, tabLabel.implicitWidth + (effectiveHotkey.length > 0 ? (hotkeyBadge.implicitWidth + 10) : 0) + (showCloseBtn ? 48 : 24))
                    implicitHeight: 38
                    radius: 6
                    color: isActive ? "#2D3035" : (tabMouse.containsMouse ? "#24272B" : "#1E2023")
                    border.color: isActive ? "#8E2DE2" : "#33363B"
                    border.width: isActive ? 2 : 1

                    property bool showCloseBtn: root.tabModel ? root.tabModel.count > 1 : false

                    // Main Tab Click & Right-click MouseArea
                    // Restricted to not overlap the hotkey badge or close button
                    MouseArea {
                        id: tabMouse
                        anchors.fill: parent
                        anchors.rightMargin: (tabItem.showCloseBtn ? 32 : 0) + (tabItem.effectiveHotkey.length > 0 ? (hotkeyBadge.width + 8) : 0)
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
                            color: tabItem.isActive ? "#FFFFFF" : "#AAAAAA"
                            font.bold: tabItem.isActive
                            font.pointSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
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

                        MenuSeparator {}

                        MenuItem {
                            text: qsTr("Rename Tab...")
                            onTriggered: root.renameTabRequested(tabItem.index)
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
