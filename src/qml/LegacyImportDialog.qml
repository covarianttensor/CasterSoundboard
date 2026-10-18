import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    implicitWidth: 520
    topPadding: 18
    bottomPadding: 18
    leftPadding: 22
    rightPadding: 22

    property string targetFilePath: ""
    property string targetFileName: ""

    function openForFile(filePath) {
        targetFilePath = filePath;
        var parts = filePath.split("/");
        targetFileName = parts.length > 0 ? parts[parts.length - 1] : filePath;
        open();
    }

    background: Rectangle {
        color: "#222528"
        radius: 8
        border.color: "#3A3E42"
        border.width: 1
    }

    contentItem: ColumnLayout {
        id: colLayout
        spacing: 14

        // --- Header Title with Warning Badge ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                width: 32
                height: 32
                radius: 16
                color: "#3D2B12"
                border.color: "#F59E0B"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "⚠️"
                    font.pixelSize: 14
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: qsTr("Legacy Profile Detected (Caster v1)")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 12
                }

                Text {
                    text: qsTr("File: \"%1\"").arg(root.targetFileName)
                    color: "#A0A6B2"
                    font.pointSize: 9
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#33373B"
        }

        // --- Explanation & Bullet Items ---
        Text {
            text: qsTr("This soundboard profile was created using the legacy binary format from Caster Soundboard v1.")
            color: "#D1D5DB"
            font.pointSize: 9.5
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Text {
            text: qsTr("What will be imported:")
            color: "#FFFFFF"
            font.bold: true
            font.pointSize: 9.5
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            spacing: 8

            RowLayout {
                spacing: 8
                Text { text: "🎵"; font.pixelSize: 13 }
                Text {
                    text: qsTr("<b>Sounds & Layout:</b> All 32 sound tiles, audio paths, volume levels, loop settings, and cue points.")
                    color: "#D1D5DB"
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                spacing: 8
                Text { text: "🎨"; font.pixelSize: 13 }
                Text {
                    text: qsTr("<b>Tile Colors:</b> Legacy gradient styles converted to solid colors.")
                    color: "#D1D5DB"
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                spacing: 8
                Text { text: "🎙️"; font.pixelSize: 13 }
                Text {
                    text: qsTr("<b>PTT:</b> Push-To-Talk and Mute controls imported into the PTT manager and pinned to this soundboard.")
                    color: "#D1D5DB"
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // --- Callout Box: What to do to convert ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: calloutCol.implicitHeight + 20
            radius: 6
            color: "#1E2023"
            border.color: "#2563EB"
            border.width: 1

            ColumnLayout {
                id: calloutCol
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                RowLayout {
                    spacing: 6
                    Text { text: "ℹ️"; font.pixelSize: 13 }
                    Text {
                        text: qsTr("What you must do to convert:")
                        color: "#60A5FA"
                        font.bold: true
                        font.pointSize: 9
                    }
                }

                Text {
                    text: qsTr("To preserve your original legacy file, this board will be loaded as unsaved. You must use <b>Save As</b> (Ctrl+Shift+S) to save a modern v2 JSON .caster profile. Your original binary file will remain untouched.")
                    color: "#CBD5E1"
                    font.pointSize: 8.5
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Item { Layout.preferredHeight: 4 }

        // --- Bottom Buttons ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Cancel")
                implicitHeight: 36
                implicitWidth: 90
                onClicked: root.close()

                background: Rectangle {
                    color: parent.hovered ? "#353A42" : "#282C32"
                    radius: 6
                    border.color: "#3A3E42"
                    border.width: 1
                }

                contentItem: Text {
                    text: parent.text
                    color: "#CCCCCC"
                    font.pointSize: 9
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: qsTr("⚡ Import Profile")
                implicitHeight: 36
                implicitWidth: 140
                onClicked: {
                    root.close();
                    if (projectController && root.targetFilePath.length > 0) {
                        projectController.loadProject(root.targetFilePath);
                    }
                }

                background: Rectangle {
                    color: parent.hovered ? "#9D3BE8" : "#8E2DE2"
                    radius: 6
                }

                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 9.5
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
