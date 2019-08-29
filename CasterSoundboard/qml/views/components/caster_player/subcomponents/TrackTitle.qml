import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Item {
    id: root
    width: 85; height: 50
    property string trackTitleString: "<Drop File>"

    Rectangle {
        enabled: root.enabled
        anchors.fill: parent
        color: "#414345"
        border.color: "white"
        border.width: 2
        radius: 8
        opacity: 0.6

    }

    TextArea {
         id: text
         enabled: root.enabled
         clip: true
         readOnly: true
         width: parent.width; height: parent.height
         anchors.margins: 2
         anchors.centerIn: parent
         wrapMode: TextEdit.Wrap
         color: "white"
         font.family: "Helvetica"
         font.bold: true
         font.pointSize: 11

         text: root.trackTitleString
    }
}

