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
            contentHeight: contentCol.implicitHeight
            clip: true

            ColumnLayout {
                id: contentCol
                width: parent.width
                spacing: 20

                // Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/audio_devices.png"
                        width: 36; height: 36
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Audio Devices & Virtual Microphone")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Configure playback output destination and voice chat virtual microphone piping.")
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

                // Audio Output Selection Box
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
                        spacing: 10

                        Text {
                            text: qsTr("Master Audio Output Device")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 12
                            ComboBox {
                                id: outputCombo
                                Layout.preferredWidth: 340
                                Layout.preferredHeight: 40
                                model: [qsTr("Default System Audio Output"), qsTr("Virtual Sink (CasterSoundboard)"), qsTr("Headphones / Line Out")]
                                currentIndex: 0
                                onCurrentTextChanged: {
                                    if (typeof projectController !== "undefined") {
                                        projectController.outputDeviceName = currentText;
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Active: %1").arg(outputCombo.currentText)
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 10
                            }
                        }
                    }
                }

                // Virtual Mic Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        RowLayout {
                            spacing: 8
                            Text {
                                text: qsTr("PipeWire / PulseAudio Virtual Mic")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 12
                            }
                            Rectangle {
                                Layout.preferredWidth: 64
                                Layout.preferredHeight: 22
                                radius: 4
                                color: "#27AE60"
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("READY")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                            }
                        }

                        Text {
                            text: qsTr("Combine microphone and soundboard audio into a dedicated virtual microphone input for Discord, OBS, or Zoom without needing physical audio cables.")
                            color: "#AAAAAA"
                            font.pointSize: 10
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
