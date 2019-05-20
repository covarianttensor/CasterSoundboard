import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Item {
    id: root
    // Component properties
    width: parent.width; height: parent.height
    state: "unchecked"

    // Switch Properties & events
    property bool checked: false
    property string questionText: "State:"
    property alias questionTextFont: question.font
    property string checkedText: "ON"
    property string uncheckedText: "OFF"
    signal toggled
    onToggled: {
        switch(root.checked){
        case false:
            root.checked = true;
            break;
        case true:
            root.checked = false;
            break;
        }
    }
    onCheckedChanged: {
        switch(root.checked){
        case false:
            root.state = "unchecked";
            break;
        case true:
            root.state = "checked";
            break;
        }
    }

    states: [
        State {
            name: "unchecked"
            PropertyChanges { target: background; color: "#434343" }
        },
        State {
            name: "checked"
            PropertyChanges { target: background; color: "#764ba2" }
        }
    ]

    transitions: [
        Transition {
            from: "unchecked"; to: "checked"
            NumberAnimation { target: handle; properties: "x"; from: 0; to: switchContainer.width - handle.width; duration: 70 }
            ColorAnimation {
                target: background
                properties: "color"
                from: "#434343"
                to: "#764ba2"
                duration: 150
            }
        },
        Transition {
            from: "checked"; to: "unchecked"
            NumberAnimation { target: handle; properties: "x"; from: switchContainer.width - handle.width; to: 0; duration: 70 }
            ColorAnimation {
                target: background
                properties: "color"
                from: "#764ba2"
                to: "#434343"
                duration: 150
            }
        }
    ]

    Column {
        id: gridContainer
        width: root.width
        height: 0.8 * root.height
        spacing: 8

        Item {
            id: questionContainer
            x: (parent.width - this.width)/2
            width: question.width; height: 0.5 * parent.height

            Text {
                id: question
                y: (questionContainer.height - this.height)/2
                color: "white"
                font.bold: true
                font.pointSize: 24
                text: root.questionText
            }
        }

        Item {
            id: switchContainer
            x: (parent.width - this.width)/2
            width: 120; height: 0.5 * parent.height

            Rectangle {
                id: background
                width: parent.width; height: parent.height
                radius: parent.height
                border.color: "white"
                border.width: 2
            }

            Text {
                x: root.checked ? (switchContainer.width - handle.width - this.width) : handle.width; y: (switchContainer.height - this.height)/2
                color: "white"
                font.bold: true
                font.pointSize: 24
                text: root.checked ? qsTr(root.checkedText) : qsTr(root.uncheckedText)
            }

            Rectangle {
                id: handle
                width: parent.height; height: parent.height
                radius: parent.height
                border.color: "white"
                border.width: 2
                color: "#667eea"
            }

            MouseArea {
                width: parent.width; height: parent.height
                onClicked: root.toggled()
            }
        }
    }


}
