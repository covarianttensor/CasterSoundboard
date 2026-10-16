import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    signal actionHotkeyPromptRequested(string action)

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: prefCol.implicitHeight
            clip: true

            ColumnLayout {
                id: prefCol
                width: parent.width
                spacing: 20

                // Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/cog.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Preferences & Workspace Settings")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Caster Soundboard v2.0 - Touch-first QML Edition")
                            color: "#888888"
                            font.pointSize: 10
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#33373B"
                }

                // Audio Ducking Ratio Setting
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8

                        Text {
                            text: qsTr("Ducking Volume Attenuation")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 12
                            Slider {
                                id: duckSlider
                                from: 0.05
                                to: 0.75
                                value: typeof projectController !== "undefined" ? projectController.duckingVolumeRatio : 0.25
                                stepSize: 0.05
                                Layout.preferredWidth: 240
                                onMoved: {
                                    if (typeof projectController !== "undefined") {
                                        projectController.duckingVolumeRatio = value;
                                    }
                                }
                            }
                            Text {
                                text: Math.round(duckSlider.value * 100) + "%"
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 11
                            }
                        }
                    }
                }

                // Hotkey Scope Setting (Mode B Tab-Scoped vs Mode A Global)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 165
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        Text {
                            text: qsTr("Sound Tile Hotkey Scope")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        Text {
                            text: qsTr("Control whether sound tile hotkeys trigger only on the currently active tab (Bank Mode) or trigger sounds globally across all tabs.")
                            color: "#AAAAAA"
                            font.pointSize: 9.5
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 16
                            Layout.fillWidth: true

                            // Mode B (Default, TabScoped / Sound Banks)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 64
                                radius: 6
                                color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#2F2344" : "#1A1C1F"
                                border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#8E2DE2" : (modeBMouse.containsMouse ? "#4D535D" : "#33373B")
                                border.width: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? 2 : 1

                                MouseArea {
                                    id: modeBMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.hotkeyScope = 0;
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 9
                                        color: "transparent"
                                        border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#8E2DE2" : "#555A62"
                                        border.width: 2

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: "#8E2DE2"
                                            visible: typeof projectController !== "undefined" && projectController.hotkeyScope === 0
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: qsTr("Sound Banks (Tab-Scoped) — Default")
                                            color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 0) ? "#FFFFFF" : "#CCCCCC"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Only active tab tiles respond. Allows reusing keys on each tab.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }
                                    }
                                }
                            }

                            // Mode A (Global)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 64
                                radius: 6
                                color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#2F2344" : "#1A1C1F"
                                border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#8E2DE2" : (modeAMouse.containsMouse ? "#4D535D" : "#33373B")
                                border.width: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? 2 : 1

                                MouseArea {
                                    id: modeAMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.hotkeyScope = 1;
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 9
                                        color: "transparent"
                                        border.color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#8E2DE2" : "#555A62"
                                        border.width: 2

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: "#8E2DE2"
                                            visible: typeof projectController !== "undefined" && projectController.hotkeyScope === 1
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: qsTr("Global (All Tabs)")
                                            color: (typeof projectController !== "undefined" && projectController.hotkeyScope === 1) ? "#FFFFFF" : "#CCCCCC"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Tile hotkeys trigger regardless of which tab is currently active.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }
                                    }
                            }
                        }
                    }
                }
            }

            // Global Action Shortcuts Setting (Panic / Stop All & Ducking)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 185
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Text {
                            text: qsTr("Global Action Shortcuts")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        Text {
                            text: qsTr("Assign custom keyboard shortcuts to instantly panic-stop all sounds or toggle audio ducking during production.")
                            color: "#AAAAAA"
                            font.pointSize: 9.5
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        // Stop All sounds row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 4
                                color: "#962D22"
                                Image {
                                    source: "../icons/stopAll.png"
                                    anchors.centerIn: parent
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: qsTr("Stop All Sounds (Panic Button)")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 10
                                }
                                Text {
                                    text: qsTr("Immediately halts playback of all active sounds on every tab.")
                                    color: "#888888"
                                    font.pointSize: 8.5
                                }
                            }

                            // Shortcut Badge
                            Rectangle {
                                Layout.preferredHeight: 30
                                Layout.preferredWidth: Math.max(70, stopAllText.implicitWidth + 20)
                                radius: 6
                                color: "#16181B"
                                border.color: (typeof projectController !== "undefined" && projectController.stopAllHotkey.length > 0) ? "#8E2DE2" : "#33373B"
                                border.width: 1

                                Text {
                                    id: stopAllText
                                    anchors.centerIn: parent
                                    text: (typeof projectController !== "undefined" && projectController.stopAllHotkey.length > 0)
                                          ? projectController.stopAllHotkey : qsTr("None")
                                    color: (typeof projectController !== "undefined" && projectController.stopAllHotkey.length > 0) ? "#FFFFFF" : "#777777"
                                    font.bold: (typeof projectController !== "undefined" && projectController.stopAllHotkey.length > 0)
                                    font.pointSize: 9.5
                                }
                            }

                            Button {
                                text: qsTr("Change")
                                Layout.preferredHeight: 32
                                Layout.preferredWidth: 70
                                background: Rectangle {
                                    color: parent.hovered ? "#9D3BF0" : "#8E2DE2"
                                    radius: 5
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: root.actionHotkeyPromptRequested("stopAll")
                            }

                            Button {
                                text: qsTr("Clear")
                                Layout.preferredHeight: 32
                                Layout.preferredWidth: 60
                                visible: typeof projectController !== "undefined" && projectController.stopAllHotkey.length > 0
                                background: Rectangle {
                                    color: parent.hovered ? "#3E1E22" : "#2A181A"
                                    radius: 5
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
                                    if (typeof projectController !== "undefined") {
                                        projectController.stopAllHotkey = "";
                                    }
                                }
                            }
                        }

                        // Ducking row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 4
                                color: "#F39C12"
                                Image {
                                    source: "../icons/duck.png"
                                    anchors.centerIn: parent
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: qsTr("Toggle Audio Ducking")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 10
                                }
                                Text {
                                    text: qsTr("Ducks playing sound volume down to the configured attenuation ratio.")
                                    color: "#888888"
                                    font.pointSize: 8.5
                                }
                            }

                            // Shortcut Badge
                            Rectangle {
                                Layout.preferredHeight: 30
                                Layout.preferredWidth: Math.max(70, duckingText.implicitWidth + 20)
                                radius: 6
                                color: "#16181B"
                                border.color: (typeof projectController !== "undefined" && projectController.duckingHotkey.length > 0) ? "#8E2DE2" : "#33373B"
                                border.width: 1

                                Text {
                                    id: duckingText
                                    anchors.centerIn: parent
                                    text: (typeof projectController !== "undefined" && projectController.duckingHotkey.length > 0)
                                          ? projectController.duckingHotkey : qsTr("None")
                                    color: (typeof projectController !== "undefined" && projectController.duckingHotkey.length > 0) ? "#FFFFFF" : "#777777"
                                    font.bold: (typeof projectController !== "undefined" && projectController.duckingHotkey.length > 0)
                                    font.pointSize: 9.5
                                }
                            }

                            Button {
                                text: qsTr("Change")
                                Layout.preferredHeight: 32
                                Layout.preferredWidth: 70
                                background: Rectangle {
                                    color: parent.hovered ? "#9D3BF0" : "#8E2DE2"
                                    radius: 5
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: root.actionHotkeyPromptRequested("ducking")
                            }

                            Button {
                                text: qsTr("Clear")
                                Layout.preferredHeight: 32
                                Layout.preferredWidth: 60
                                visible: typeof projectController !== "undefined" && projectController.duckingHotkey.length > 0
                                background: Rectangle {
                                    color: parent.hovered ? "#3E1E22" : "#2A181A"
                                    radius: 5
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
                                    if (typeof projectController !== "undefined") {
                                        projectController.duckingHotkey = "";
                                    }
                                }
                            }
                        }
                    }
                }

                // About card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6

                        Text {
                            text: qsTr("About Caster Soundboard")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }
                        Text {
                            text: qsTr("An open-source hotkeyed soundboard engineered for live broadcasting, podcasting, gaming, and theater productions.")
                            color: "#AAAAAA"
                            font.pointSize: 10
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        Text {
                            text: qsTr("Licensed under GNU Lesser General Public License (LGPL v3).")
                            color: "#666666"
                            font.pointSize: 9
                        }
                    }
                }
            }
        }
    }
}
