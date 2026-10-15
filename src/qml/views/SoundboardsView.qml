import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Dynamic Tab Bar
        SoundboardTabBar {
            Layout.fillWidth: true
            tabModel: typeof projectController !== "undefined" ? projectController.tabModel : null
            onAddTabRequested: {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.addTab();
                }
            }
            onCloseTabRequested: (index) => {
                if (projectController && projectController.tabModel) {
                    projectController.tabModel.removeTab(index);
                }
            }
            onRenameTabRequested: (index) => {
                renameDialog.targetIndex = index;
                if (projectController && projectController.tabModel) {
                    renameField.text = projectController.tabModel.tabNameAt(index);
                }
                renameDialog.open();
            }
        }

        // Soundboard Grid Area (Phase 2 Container)
        Rectangle {
            id: gridArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#181A1B"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    id: statusHeader
                    text: typeof projectController !== "undefined" && projectController.tabModel && projectController.tabModel.currentGridModel
                          ? qsTr("Active Tab: %1").arg(projectController.tabModel.tabNameAt(projectController.tabModel.currentIndex))
                          : qsTr("Soundboard Workspace")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 18
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    text: qsTr("Tiles in active set: %1 | Zoom Scale: %2%").arg(
                              typeof projectController !== "undefined" && projectController.tabModel && projectController.tabModel.currentGridModel
                              ? projectController.tabModel.currentGridModel.count : 0
                          ).arg(Math.round((typeof projectController !== "undefined" ? projectController.tileScale : 1.0) * 100))
                    color: "#888888"
                    font.pointSize: 12
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle {
                    Layout.preferredWidth: 260
                    Layout.preferredHeight: 44
                    radius: 8
                    color: "#25282B"
                    border.color: "#8E2DE2"
                    border.width: 1

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8
                        Image {
                            source: "../icons/add_player.png"
                            sourceSize.width: 22
                            sourceSize.height: 22
                            Layout.preferredWidth: 22
                            Layout.preferredHeight: 22
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: qsTr("Add Sound Tile (+)")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 11
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (projectController && projectController.tabModel && projectController.tabModel.currentGridModel) {
                                projectController.tabModel.currentGridModel.appendTile();
                            }
                        }
                    }
                }
            }
        }
    }

    // Rename Tab Dialog
    Dialog {
        id: renameDialog
        title: qsTr("Rename Soundboard Tab")
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        property int targetIndex: -1

        ColumnLayout {
            spacing: 8
            Text {
                text: qsTr("Enter new name for tab:")
                color: "#FFFFFF"
            }
            TextField {
                id: renameField
                Layout.preferredWidth: 260
                selectByMouse: true
                color: "#FFFFFF"
            }
        }

        onAccepted: {
            if (targetIndex >= 0 && projectController && projectController.tabModel) {
                projectController.tabModel.renameTab(targetIndex, renameField.text);
            }
        }
    }
}
