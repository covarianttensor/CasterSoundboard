import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Item {
    id: root
    width: root.size; height: root.size
    property int size: 40
    property alias source: image.source
    property bool isSelected: false
    signal clicked

    Button {
        id: button
        width: root.width; height: root.height
        onClicked: root.clicked()
        background: Rectangle {
            color: "transparent"
            border.color: "white"
            border.width: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: button.down ? "white" : "#414345" }
                GradientStop { position: 1.0; color: button.down ? "white" : "#232526" }
            }
        }

        Image {
            id: image
            width: Math.floor(0.80 * root.size); height: Math.floor(0.80 * root.size)
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
        }

        ColorOverlay {
                anchors.fill: image; source: image
                color: (button.down ? "#000000" : "#ffffff")  // make image white
        }
    }
}

