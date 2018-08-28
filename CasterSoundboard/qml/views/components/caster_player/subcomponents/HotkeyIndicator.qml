import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Item {
    id: root
    width: root.size; height: root.size
    property int size: 50
    property string hotkeyText: "?"
    signal clicked

    Button {
        id: button
        enabled: root.enabled
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

        Text {
            id: text
            enabled: root.enabled
            anchors.centerIn: parent
            color: "white"
            font.family: "Helvetica"
            font.bold: true
            font.pointSize: 28
            text: root.hotkeyText
        }

        ColorOverlay {
                anchors.fill: text; source: text
                color: (button.down ? "#000000" : "#ffffff")  // make image white
        }
    }
}

