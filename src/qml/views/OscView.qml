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
            contentHeight: oscCol.implicitHeight
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
                        width: 36; height: 36
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

                // OSC Configuration Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Text {
                            text: qsTr("OSC Server Settings")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 12
                        }

                        RowLayout {
                            spacing: 16

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Inbound UDP Port:"); color: "#AAAAAA"; font.pointSize: 10 }
                                TextField {
                                    text: "9000"
                                    Layout.preferredWidth: 120
                                    color: "#FFFFFF"
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Outbound Host:"); color: "#AAAAAA"; font.pointSize: 10 }
                                TextField {
                                    text: "127.0.0.1"
                                    Layout.preferredWidth: 160
                                    color: "#FFFFFF"
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Outbound Port:"); color: "#AAAAAA"; font.pointSize: 10 }
                                TextField {
                                    text: "9001"
                                    Layout.preferredWidth: 120
                                    color: "#FFFFFF"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
