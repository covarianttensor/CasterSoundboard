import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import "./" as Subcomponent

Item {
    id: root
    // Component properties
    width: parent.width; height: parent.height

    // Properties, functions & events
    property int regionBegin: 0
    onRegionBeginChanged: beginInput.text = root.msToProperTime(root.regionBegin)
    property int regionEnd: 0
    onRegionEndChanged: endInput.text = root.msToProperTime(root.regionEnd)
    property int duration: 0
    property string labelText: "Set play region range:"

    function msToProperTime(msTime){
        var secs = (msTime / 1000 - (msTime % 1000) / 1000) % 60;
        var mins = ((msTime / 1000 - (msTime % 1000) / 1000) - secs) / 60;
        return ("00" + mins).slice(-2) + ':' + ("00" + secs).slice(-2);
    }

    function properTimeToMs(properTime){
        var ms = 0;
        var time = properTime.split(':');
        if(time.length === 2){
            ms = parseInt(time[0], 10) * 60 * 1000 + parseInt(time[1], 10) * 1000;
        }
        return ms;
    }

    // Component UI

    Column {
        id: columnContainer
        width: root.width
        height: root.height
        spacing: 8

        Item {
            id: labelContainer
            x: (parent.width - this.width)/2
            width: label.width; height: 0.33 * parent.height

            Text {
                id: label
                y: (labelContainer.height - this.height)/2
                color: "white"
                font.bold: true
                font.pointSize: 24
                text: root.labelText
            }
        }

        Item {
            id: rangeContainer
            width: parent.width; height: 0.33 * parent.height

            Row {
                anchors.centerIn: parent

                Text {
                    id: beginLabel
                    color: "white"
                    font.bold: true
                    font.pointSize: 24
                    text: "BEGIN:"
                }

                Item {
                    width: 80; height: parent.height
                    clip: true

                    Rectangle {
                        width: parent.width; height: parent.height
                        color: "white"
                    }

                    TextInput {
                        id: beginInput
                        width: parent.width; height: parent.height
                        font.bold: true
                        font.pointSize: 20
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: root.msToProperTime(root.regionBegin)
                    }
                }

                Text {
                    id: endLabel
                    color: "white"
                    font.bold: true
                    font.pointSize: 24
                    text: "END:"
                }

                Item {
                    width: 80; height: parent.height
                    clip: true

                    Rectangle {
                        width: parent.width; height: parent.height
                        color: "white"
                    }

                    TextInput {
                        id: endInput
                        width: parent.width; height: parent.height
                        font.bold: true
                        font.pointSize: 20
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: root.msToProperTime(root.regionEnd)
                    }
                }
            }
        }

        Item {
            width: parent.width; height: 0.33 * parent.height
            Subcomponent.BootstrapButton {
                id: setRangeButton
                width: 100; height: parent.height
                anchors.centerIn: parent
                text: 'Set Range'; type: 'info'
                onClicked: {
                    var beginTemp = root.properTimeToMs(beginInput.text);
                    var endTemp = root.properTimeToMs(endInput.text);
                    if(0 <= beginTemp && beginTemp < root.duration && beginTemp < endTemp){
                        if(0 < endTemp && beginTemp < endTemp && endTemp <= root.duration){
                            root.regionBegin = beginTemp;
                            root.regionEnd = endTemp;
                        }
                    }

                    // Update Inputs
                    beginInput.text = root.msToProperTime(root.regionBegin);
                    endInput.text = root.msToProperTime(root.regionEnd);
                }
            }
        }


    }


}
