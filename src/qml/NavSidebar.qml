import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property int currentIndex: 0
    property bool isCollapsed: false
    signal fullscreenToggled()

    width: isCollapsed ? 56 : 75
    color: "#1E2022"

    Behavior on width {
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // Soundboard Tab
        NavButton {
            iconSource: "icons/soundboard_tab.png"
            tooltipText: qsTr("Soundboards")
            isActive: root.currentIndex === 0
            isCollapsed: root.isCollapsed
            onClicked: root.currentIndex = 0
        }

        // Virtual Mic & Audio Routing Tab
        NavButton {
            iconSource: "icons/audio_devices.png"
            tooltipText: qsTr("Audio Devices & Virtual Mic")
            isActive: root.currentIndex === 1
            isCollapsed: root.isCollapsed
            onClicked: root.currentIndex = 1
        }

        // OSC Tab
        NavButton {
            iconSource: "icons/open_sound_control.png"
            tooltipText: qsTr("Open Sound Control (OSC)")
            isActive: root.currentIndex === 2
            isCollapsed: root.isCollapsed
            onClicked: root.currentIndex = 2
        }

        // Preferences Tab
        NavButton {
            iconSource: "icons/cog.png"
            tooltipText: qsTr("Preferences")
            isActive: root.currentIndex === 3
            isCollapsed: root.isCollapsed
            onClicked: root.currentIndex = 3
        }

        Item {
            Layout.fillHeight: true
        }

        // Fullscreen Toggle
        NavButton {
            iconSource: "icons/fullscreen.png"
            tooltipText: qsTr("Toggle Fullscreen (F11)")
            isCollapsed: root.isCollapsed
            onClicked: root.fullscreenToggled()
        }

        // Collapse / Expand Sidebar Toggle
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            flat: true
            onClicked: root.isCollapsed = !root.isCollapsed
            background: Rectangle {
                color: parent.hovered ? "#33373B" : "transparent"
            }
            contentItem: Text {
                text: root.isCollapsed ? "»" : "«"
                color: "#AAAAAA"
                font.bold: true
                font.pointSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    component NavButton : Item {
        id: btnItem
        property string iconSource
        property string tooltipText
        property bool isActive: false
        property bool isCollapsed: false
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: root.isCollapsed ? 56 : 70

        Rectangle {
            anchors.fill: parent
            color: btnItem.isActive ? "#8E2DE2" : (mouseArea.containsMouse ? "#2E3236" : "transparent")
            radius: 6
            anchors.margins: 4

            Behavior on color {
                ColorAnimation { duration: 120 }
            }

            Image {
                anchors.centerIn: parent
                sourceSize.width: 28
                sourceSize.height: 28
                width: 28
                height: 28
                source: btnItem.iconSource
                fillMode: Image.PreserveAspectFit
                opacity: btnItem.isActive ? 1.0 : (mouseArea.containsMouse ? 1.0 : 0.85)
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btnItem.clicked()
        }

        ToolTip.visible: mouseArea.containsMouse && !btnItem.isActive
        ToolTip.text: btnItem.tooltipText
        ToolTip.delay: 600
    }
}
