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

                    property bool isActive: root.currentIndex === index

                    implicitWidth: Math.max(140, tabLabel.implicitWidth + (showCloseBtn ? 48 : 24))
                    implicitHeight: 38
                    radius: 6
                    color: isActive ? "#2D3035" : (tabMouse.containsMouse ? "#24272B" : "#1E2023")
                    border.color: isActive ? "#8E2DE2" : "#33363B"
                    border.width: isActive ? 2 : 1

                    property bool showCloseBtn: root.tabModel ? root.tabModel.count > 1 : false

                    // Main Tab Click & Right-click MouseArea
                    // Restricted to not overlap the close button on the right
                    MouseArea {
                        id: tabMouse
                        anchors.fill: parent
                        anchors.rightMargin: tabItem.showCloseBtn ? 32 : 0
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
