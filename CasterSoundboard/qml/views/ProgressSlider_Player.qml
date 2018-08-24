import QtQuick 2.7
import QtQuick.Controls 2.4

Slider {
    id: control

    implicitWidth: 190
    implicitHeight: 26

    handle: Rectangle {
        x: control.visualPosition * (control.width - width)
        y: (control.height - height) / 2
        width: 40
        height: 30
        color: control.pressed ? "#f0f0f0" : "#f6f6f6"
        border.color: "white"
        border.width: 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#414345" }
            GradientStop { position: 1.0; color: "#232526" }
        }
    }

    background: Rectangle {
        y: (control.height - height) / 2
        height: 20
        border.color: "white"
        border.width: 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#cfcfcf" }
            GradientStop { position: 1.0; color: "#9e9e9e" }
        }

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            border.color: "white"
            border.width: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#414345" }
                GradientStop { position: 1.0; color: "#232526" }
            }
        }
    }
}
