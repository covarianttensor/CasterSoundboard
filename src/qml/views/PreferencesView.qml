import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

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
                        width: 36; height: 36
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
