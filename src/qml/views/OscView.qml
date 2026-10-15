import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    component DarkTextField : TextField {
        id: dtf
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        implicitHeight: 44
        background: Rectangle {
            color: "#282C32"
            radius: 6
            border.color: dtf.activeFocus ? "#8E2DE2" : "#454B54"
            border.width: dtf.activeFocus ? 2 : 1
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: oscCol.implicitHeight + 40
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: oscCol
                width: flickable.width - 12
                spacing: 20

                // 1. Header
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
                            text: qsTr("Control playback wirelessly via TouchOSC, Stream Deck, tablet, or external broadcast hardware with real-time two-way synchronization.")
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

                // 2. Server Status & Quick Toggle Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 88
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Rectangle {
                            width: 16; height: 16; radius: 8
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
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            id: toggleBtn
                            text: (typeof oscController !== "undefined" && oscController.isRunning) ? qsTr("Stop Server") : qsTr("Start Server")
                            Layout.preferredWidth: 140
                            Layout.preferredHeight: 44
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

                // 3. Network Configuration Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: netCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: netCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        Text {
                            text: qsTr("OSC Network Endpoints")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            ColumnLayout {
                                spacing: 4
                                Text { text: qsTr("Inbound UDP Port:"); color: "#AAAAAA"; font.pointSize: 10 }
                                DarkTextField {
                                    id: inPortField
                                    text: typeof oscController !== "undefined" ? oscController.inboundPort.toString() : "5051"
                                    Layout.preferredWidth: 140
                                    onEditingFinished: {
                                        var p = parseInt(text);
                                        if (p > 0 && typeof oscController !== "undefined") {
                                            oscController.inboundPort = p;
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text { text: qsTr("Outbound Host (Feedback Client):"); color: "#AAAAAA"; font.pointSize: 10 }
                                DarkTextField {
                                    id: outHostField
                                    text: typeof oscController !== "undefined" ? oscController.outboundHost : "127.0.0.1"
                                    Layout.fillWidth: true
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
                                    Layout.preferredWidth: 140
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
                            text: qsTr("Standard ports: Inbound 5051 (or 8000), Outbound 9000 (TouchOSC, Bitfocus Companion, QLab)")
                            color: "#7E8590"
                            font.pointSize: 9
                        }
                    }
                }

                // 4. Automation & Two-Way Client Sync Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: autoCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: autoCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        Text {
                            text: qsTr("Automation & Two-Way Client Sync")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        // Auto-start switch
                        RowLayout {
                            spacing: 12
                            CheckBox {
                                id: autoStartCheck
                                checked: typeof oscController !== "undefined" ? oscController.autoStart : false
                                onToggled: {
                                    if (typeof oscController !== "undefined") {
                                        oscController.autoStart = checked;
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Auto-start OSC Server on Application Launch")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Text {
                                    text: qsTr("Automatically binds the UDP listener when Caster Soundboard opens (can also be invoked with --auto-start-osc).")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                }
                            }
                        }

                        // Two-way feedback switch
                        RowLayout {
                            spacing: 12
                            CheckBox {
                                id: feedbackCheck
                                checked: typeof oscController !== "undefined" ? oscController.feedbackEnabled : true
                                onToggled: {
                                    if (typeof oscController !== "undefined") {
                                        oscController.feedbackEnabled = checked;
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                Text {
                                    text: qsTr("Broadcast Feedback to Client (Two-Way Sync)")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Text {
                                    text: qsTr("Sends live play/pause/stop states, volumes, and ducking updates to TouchOSC or external control clients.")
                                    color: "#AAAAAA"
                                    font.pointSize: 9
                                }
                            }
                        }

                        // Sync Now button
                        RowLayout {
                            spacing: 12
                            Layout.leftMargin: 40
                            Button {
                                id: syncBtn
                                text: qsTr("Sync Clients Now (/glo/sync)")
                                Layout.preferredHeight: 40
                                Layout.preferredWidth: 220
                                enabled: typeof oscController !== "undefined" && oscController.isRunning
                                onClicked: {
                                    if (typeof oscController !== "undefined") {
                                        oscController.syncClients();
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
                                    color: parent.down ? "#5B18A7" : (parent.hovered ? "#A044FF" : "#8E2DE2")
                                    radius: 6
                                    opacity: syncBtn.enabled ? 1.0 : 0.5
                                }
                            }
                            Text {
                                text: qsTr("Broadcasts the complete state of all sound tiles on the active tab immediately.")
                                color: "#7E8590"
                                font.pointSize: 9
                            }
                        }
                    }
                }

                // 5. OSC Activity Monitor Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: monCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: monCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        Text {
                            text: qsTr("Live OSC Monitor")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        RowLayout {
                            spacing: 16
                            Rectangle {
                                Layout.preferredHeight: 32
                                Layout.preferredWidth: 180
                                color: "#282C32"
                                radius: 6
                                border.color: "#454B54"
                                border.width: 1
                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 8
                                    Text { text: qsTr("Packets:"); color: "#AAAAAA"; font.pointSize: 10 }
                                    Text {
                                        text: typeof oscController !== "undefined" ? oscController.packetsReceived.toString() : "0"
                                        color: "#2ECC71"
                                        font.bold: true
                                        font.pointSize: 11
                                    }
                                }
                            }
                        }

                        Text {
                            text: qsTr("Last Command: %1").arg(typeof oscController !== "undefined" && oscController.lastMessageAddress.length > 0
                                  ? (oscController.lastMessageAddress + " [" + oscController.lastMessageValue + "]")
                                  : qsTr("None"))
                            color: "#74B9FF"
                            font.family: "Monospace"
                            font.pointSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }

                // 6. Quick Reference Cheat Sheet Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: refCol.implicitHeight + 36
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: refCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 8

                        Text {
                            text: qsTr("OSC Command Reference Cheat Sheet")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 13
                        }

                        Text {
                            text: qsTr("• Stop all sounds: /stop_all or /glo/m/all_p_s/stop\n• Audio ducking: /castersoundboard/ducking or /glo/m/audio_d_s [0|1]\n• Tab navigation: /glo/m/current_tab/next or /glo/m/current_tab/prev\n• Play sound: /play/<1-based index> or /cbp/<idx>/m/p_s/play\n• Pause/Resume sound: /cbp/<idx>/m/p_s/resume_pause\n• Volume fader: /cbp/<idx>/m/vol [0.0 - 1.0]\n• Loop toggle: /cbp/<idx>/m/l_s [0|1]\n• Client sync request: /sync or /glo/sync")
                            color: "#AAAAAA"
                            font.family: "Monospace"
                            font.pointSize: 9
                            lineHeight: 1.3
                        }
                    }
                }
            }
        }
    }
}
