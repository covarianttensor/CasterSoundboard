import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtMultimedia 5.9

import "./subcomponents" as Subcomponent

Drawer {
    id: root
    edge: Qt.RightEdge
    interactive: false
    width: parent.width * 0.66; height: parent.height
    background: Rectangle { anchors.fill: parent; color: "#00000000" }

    property int playerIndex: 0
    function openSettings(player_index){
        root.playerIndex = player_index
        root.open();
    }

    Pane {
        id: header
        width: parent.width; height: 60
        background: Rectangle { color: "#F58E2DE2" }

        Text {
            anchors.centerIn: parent
            color: 'white'
            font.bold: true
            font.pointSize: 24
            text: root.width < 300 ? (root.width < 210 ? '...' : 'Player\nSettings') : 'Player Settings'
        }

        Subcomponent.BootstrapButton {
            id: closeButton
            x: parent.width - closeButton.width
            y: parent.height/2 - closeButton.height/2
            text: ' X '; type: 'danger'
            onClicked: root.close()
        }
    }

    Rectangle {
        anchors.top: header.bottom
        width: parent.width; height: parent.height
        color: "#DF333333"

        Flickable {
            anchors.fill: parent
            contentHeight: columnView.height
            clip: true
            anchors.margins: 8
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: columnView
                anchors.fill: parent
                spacing: 8

                Item {
                    width: parent.width; height: 50

                    Subcomponent.BootstrapButton {
                        anchors.centerIn: parent
                        text: 'Delete Player'; type: 'danger'
                        onClicked: {
                            casterPlayerController.removeItemAt(root.playerIndex);
                            root.close();
                        }
                    }
                }



            }
        }
    }


}


