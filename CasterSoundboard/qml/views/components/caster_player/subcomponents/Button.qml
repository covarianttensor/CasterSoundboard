import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Item {
    id: root
    width: 40; height: 40
    property alias source: image.source
    property bool isTextButton: false
    property string buttonText: "Click Me"
    property alias font: text.font
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
            width: Math.floor(0.80 * root.width); height: Math.floor(0.80 * root.height)
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
            visible: !root.isTextButton
        }

        Text {
            id: text
            anchors.centerIn: parent
            color: "white"
            font.family: "Helvetica"
            font.bold: true
            font.pointSize: 12
            text: root.buttonText
            visible: root.isTextButton
        }

        ColorOverlay {
                anchors.fill: (root.isTextButton ? text : image); source: (root.isTextButton ? text : image)
                color: (button.down ? "#000000" : "#ffffff")  // make image white
        }
    }
}

