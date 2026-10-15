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
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
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
                    Layout.preferredHeight: 140
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

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
                                Layout.preferredWidth: 380
                                Layout.preferredHeight: 40
                                model: (typeof projectController !== "undefined" && projectController.availableAudioOutputs.length > 0)
                                       ? projectController.availableAudioOutputs
                                       : [qsTr("Default System Output")]
                                currentIndex: {
                                    if (typeof projectController === "undefined" || !projectController.outputDeviceName)
                                        return 0;
                                    var idx = outputCombo.model.indexOf(projectController.outputDeviceName);
                                    return idx >= 0 ? idx : 0;
                                }
                                onActivated: (index) => {
                                    if (typeof projectController !== "undefined") {
                                        projectController.outputDeviceName = currentText;
                                    }
                                }

                                background: Rectangle {
                                    color: "#282C32"
                                    radius: 6
                                    border.color: outputCombo.activeFocus ? "#8E2DE2" : "#454B54"
                                    border.width: 1
                                }
                                contentItem: Text {
                                    leftPadding: 10
                                    text: outputCombo.displayText
                                    color: "#FFFFFF"
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }

                            Button {
                                text: qsTr("Refresh")
                                Layout.preferredHeight: 40
                                onClicked: {
                                    if (typeof projectController !== "undefined") {
                                        projectController.refreshAudioDevices();
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                    radius: 6
                                    border.color: "#454B54"
                                    border.width: 1
                                }
                            }
                        }

                        Text {
                            text: qsTr("Current Target: %1").arg(typeof projectController !== "undefined" && projectController.outputDeviceName.length > 0 ? projectController.outputDeviceName : qsTr("Default System Output"))
                            color: "#A29BFE"
                            font.bold: true
                            font.pointSize: 10
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

                        Text {
                            text: qsTr("Sink: CasterSoundboardSink | Virtual Source: CasterSoundboardMic")
                            color: "#7E8590"
                            font.pointSize: 9
                        }
                    }
                }
            }
        }
    }
}
