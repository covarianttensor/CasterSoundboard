import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4

Item {
    id: root
    anchors.fill: parent
    signal clicked

    Image {
        visible: root.visible
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: '/qml/img/player_outline.png'
    }

    Button {
        id: button
        enabled: root.enabled
        anchors.centerIn: parent
        width: Math.floor(0.50 * root.width); height: Math.floor(0.50 * root.height)
        onClicked: root.clicked()
        background: Rectangle { color: "transparent" }

        Image {
            id: image
            width: Math.floor(0.50 * root.width); height: Math.floor(0.50 * root.height)
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
            source: '/qml/icons/add_player.png'
        }


        ColorOverlay {
                anchors.fill: image; source: image
                color: (button.down ? "#D0000000" : "#00000000")  // make image white
        }

    }
}
