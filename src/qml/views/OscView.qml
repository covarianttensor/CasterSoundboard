import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    component DarkTextField : TextField {
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        background: Rectangle {
            color: "#282C32"
            radius: 6
            border.color: parent.activeFocus ? "#8E2DE2" : "#454B54"
            border.width: parent.activeFocus ? 2 : 1
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: oscCol.implicitHeight + 20
            clip: true

            ColumnLayout {
                id: oscCol
                width: parent.width
                spacing: 20

                // Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/open_sound_control.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Open Sound Control (OSC)")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Control playback wirelessly via TouchOSC, Stream Deck, tablet, or external broadcast hardware.")
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

                // Server Status & Control
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 90
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Rectangle {
                            width: 14; height: 14; radius: 7
                            color: (typeof oscController !== "undefined" && oscController.isRunning) ? "#2ECC71" : "#7E8590"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: (typeof oscController !== "undefined" && oscController.isRunning)
                                      ? qsTr("OSC Server Active")
                                      : qsTr("OSC Server Inactive")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 13
                            }
                            Text {
                                text: typeof oscController !== "undefined" ? oscController.statusMessage : qsTr("Server stopped")
                                color: "#AAAAAA"
                                font.pointSize: 10
                            }
                        }

                        Button {
                            text: (typeof oscController !== "undefined" && oscController.isRunning) ? qsTr("Stop Server") : qsTr("Start Server")
                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 40
                            onClicked: {
                                if (typeof oscController !== "undefined") {
                                    oscController.toggleServer();
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
                                color: (typeof oscController !== "undefined" && oscController.isRunning)
                                       ? (parent.hovered ? "#C0392B" : "#962D22")
                                       : (parent.hovered ? "#27AE60" : "#1E8449")
                                radius: 6
                            }
                        }
                    }
                }

                // OSC Configuration Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 14

                        Text {
                            text: qsTr("OSC Network Endpoints")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 16

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Inbound UDP Port:"); color: "#AAAAAA"; font.pointSize: 10 }
                                DarkTextField {
                                    id: inPortField
                                    text: typeof oscController !== "undefined" ? oscController.inboundPort.toString() : "5051"
                                    Layout.preferredWidth: 130
                                    Layout.preferredHeight: 38
                                    onEditingFinished: {
                                        var p = parseInt(text);
                                        if (p > 0 && typeof oscController !== "undefined") {
                                            oscController.inboundPort = p;
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Outbound Host:"); color: "#AAAAAA"; font.pointSize: 10 }
                                DarkTextField {
                                    id: outHostField
                                    text: typeof oscController !== "undefined" ? oscController.outboundHost : "127.0.0.1"
                                    Layout.preferredWidth: 160
                                    Layout.preferredHeight: 38
                                    onEditingFinished: {
                                        if (typeof oscController !== "undefined") {
                                            oscController.outboundHost = text;
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Outbound Port:"); color: "#AAAAAA"; font.pointSize: 10 }
                                DarkTextField {
                                    id: outPortField
                                    text: typeof oscController !== "undefined" ? oscController.outboundPort.toString() : "9000"
                                    Layout.preferredWidth: 130
                                    Layout.preferredHeight: 38
                                    onEditingFinished: {
                                        var p = parseInt(text);
                                        if (p > 0 && typeof oscController !== "undefined") {
                                            oscController.outboundPort = p;
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            text: qsTr("Standard ports: Inbound 5051 (or 8000), Outbound 9000 (TouchOSC / QLab)")
                            color: "#7E8590"
                            font.pointSize: 9
                        }
                    }
                }

                // OSC Activity Monitor Card
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

                        Text {
                            text: qsTr("Live OSC Monitor")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 16
                            Text {
                                text: qsTr("Packets Received: %1").arg(typeof oscController !== "undefined" ? oscController.packetsReceived : 0)
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 11
                            }
                        }

                        Text {
                            text: qsTr("Last Command: %1").arg(typeof oscController !== "undefined" && oscController.lastMessageAddress.length > 0
                                  ? (oscController.lastMessageAddress + " [" + oscController.lastMessageValue + "]")
                                  : qsTr("None"))
                            color: "#AAAAAA"
                            font.pointSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
