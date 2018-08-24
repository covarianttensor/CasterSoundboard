import QtQuick 2.7
import QtQuick.Controls 2.4

Slider {
    id: control
    orientation: Qt.Vertical

    implicitWidth: 26
    implicitHeight: 145

    handle: Rectangle {

        x: (control.width - width) / 2
        y: control.visualPosition * (control.height - height)
        width: 50
        height: 60
        border.color: "white"
        border.width: 2
        opacity: 0.8
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#414345" }
            GradientStop { position: 1.0; color: "#232526" }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width
            height: 8
            opacity: 0.9
            color: "blue"

        }

        Text {
            anchors.centerIn: parent
            color: "white"
            font.family: "Helvetica"; font.pointSize: 18; font.bold: true
            text: Math.floor((1 - control.visualPosition) * 100) + "%"
        }
    }

    background: Rectangle {
        x: (control.width - width) / 2; y: 25
        width: 20
        height: control.height - 50
        border.color: "white"
        border.width: 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#FF0000" }//Red
            GradientStop { position: 0.3; color: "#FFF200" }//Yellow
            GradientStop { position: 1.0; color: "#1E9600" }//Green
        }
    }
}

