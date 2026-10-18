import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property int currentIndex: 0
    signal fullscreenToggled()

    width: 68
    implicitWidth: 68
    Layout.preferredWidth: 68
    Layout.fillHeight: true
    color: "#1E2022"

    ColumnLayout {
        anchors.fill: parent
        anchors.bottomMargin: 4
        spacing: 4

        // Soundboard Tab
        NavButton {
            iconSource: "icons/soundboard_tab.png"
            tooltipText: qsTr("Soundboards")
            isActive: root.currentIndex === 0
            onClicked: root.currentIndex = 0
        }

        // Virtual Mic & Audio Routing Tab
        NavButton {
            iconSource: "icons/audio_devices.png"
            tooltipText: qsTr("Audio Devices & Virtual Mic")
            isActive: root.currentIndex === 1
            onClicked: root.currentIndex = 1
        }

        // OSC Tab
        NavButton {
            iconSource: "icons/open_sound_control.png"
            tooltipText: qsTr("Open Sound Control (OSC)")
            isActive: root.currentIndex === 2
            onClicked: root.currentIndex = 2
        }

        // Preferences Tab
        NavButton {
            iconSource: "icons/cog.png"
            tooltipText: qsTr("Preferences")
            isActive: root.currentIndex === 3
            onClicked: root.currentIndex = 3
        }

        Item {
            Layout.fillHeight: true
        }

        // App Documentation (floated to bottom above fullscreen toggle)
        NavButton {
            iconSource: "icons/book.png"
            tooltipText: qsTr("Documentation & Terminal CLI Guide")
            isActive: root.currentIndex === 4
            onClicked: root.currentIndex = 4
        }

        // About & System Info
        NavButton {
            iconSource: "icons/about.png"
            tooltipText: qsTr("About Caster Soundboard")
            isActive: root.currentIndex === 5
            onClicked: root.currentIndex = 5
        }

        // Fullscreen Toggle
        NavButton {
            iconSource: "icons/fullscreen.png"
            tooltipText: qsTr("Toggle Fullscreen (F11)")
            onClicked: root.fullscreenToggled()
        }
    }

    component NavButton : Item {
        id: btnItem
        property string iconSource
        property string tooltipText
        property bool isActive: false
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 64

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
