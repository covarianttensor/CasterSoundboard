import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import "./" as Subcomponent

Item {
    id: root
    // Component properties
    width: parent.width; height: parent.height
    clip: true

    // Properties, functions & events
    property int regionBegin: 0
    onRegionBeginChanged: beginInput.text = root.msToProperTime(root.regionBegin)
    property int regionEnd: 0
    onRegionEndChanged: endInput.text = root.msToProperTime(root.regionEnd)
    property int duration: 0
    property string labelText: "Set play region range:"

    function msToProperTime(msTime){
        var ms = (msTime % 1000) / 1000;
        var secs = (msTime / 1000 - (msTime % 1000) / 1000) % 60;
        var mins = ((msTime / 1000 - (msTime % 1000) / 1000) - secs) / 60;
        var msStr = ("000" + ms).slice(-3);
        if(msStr.startsWith('.')){
            msStr = msStr.replace('.', '');
            if(msStr.length === 2){
                msStr = msStr + "0";
            }
        } else if(msStr.startsWith('0.')){
            msStr = msStr.replace('0.', '');
            msStr = (msStr + "00").substring(0, 3);
        }

        return ("00" + mins).slice(-2) + ':' + ("00" + secs).slice(-2) + "." + msStr;
    }

    function properTimeToMs(properTime){
        var ms = 0;
        var time1 = properTime.split(':');
        if(time1.length === 2){
            var time2 = time1[1].split('.');
            if(time2.length === 2 && 0 < time2[1].length && time2[1].length <= 3){
                ms = parseInt(time1[0], 10) * 60 * 1000 + parseInt(time2[0], 10) * 1000 + parseFloat("0." + time2[1]) * 1000;
            } else if(time2.length === 1) {
                ms = parseInt(time1[0], 10) * 60 * 1000 + parseInt(time1[1], 10) * 1000;
            }
        } else if(time1.length === 1){
            ms = parseInt(properTime, 10) * 1000;
        }

        return ms;
    }

    // Component UI

    Column {
        id: columnContainer
        width: root.width
        height: 0.8 * root.height
        spacing: 8

        Item {
            id: labelContainer
            x: (parent.width - this.width)/2
            width: label.width; height: (1/4) * parent.height

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
            id: rangeContainer1
            width: parent.width; height: (1/4) * parent.height

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
                    width: 130; height: parent.height
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
                        property string buffer: ''

                        onFocusChanged: {
                            if(beginInput.focus === true){
                                beginInput.buffer = beginInput.text;
                                beginInput.text = '';
                            } else {
                                if(beginInput.text === ''){
                                    beginInput.text = beginInput.buffer;
                                }
                                beginInput.buffer = '';
                            }
                        }

                        onAccepted: setRangeButton.submit()

                        Text {
                            anchors.centerIn: parent
                            color: '#804e4e4e'
                            font.bold: true
                            font.pointSize: 20
                            text: beginInput.buffer
                        }
                    }
                }

            }
        }

        Item {
            id: rangeContainer2
            width: parent.width; height: (1/4) * parent.height

            Row {
                anchors.centerIn: parent

                Text {
                    id: endLabel
                    color: "white"
                    font.bold: true
                    font.pointSize: 24
                    text: "END:"
                }

                Item {
                    width: 130; height: parent.height
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
                        property string buffer: ''

                        onFocusChanged: {
                            if(endInput.focus === true){
                                endInput.buffer = endInput.text;
                                endInput.text = '';
                            } else {
                                if(endInput.text === ''){
                                    endInput.text = endInput.buffer;
                                }
                                endInput.buffer = '';
                            }
                        }

                        onAccepted: setRangeButton.submit()

                        Text {
                            anchors.centerIn: parent
                            color: '#804e4e4e'
                            font.bold: true
                            font.pointSize: 20
                            text: endInput.buffer
                        }
                    }
                }
            }
        }

        Item {
            width: parent.width; height: (1/4) * parent.height
            Subcomponent.BootstrapButton {
                id: setRangeButton
                width: 100; height: parent.height
                anchors.centerIn: parent
                text: 'Set Range'; type: 'info'

                function submit(){
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

                onClicked: setRangeButton.submit()
            }
        }


    }


}
