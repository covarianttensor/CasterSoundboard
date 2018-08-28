import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4

Slider {
    id: control

    implicitWidth: 200
    implicitHeight: 25
    from: 0
    to: 0
    stepSize: 1

    property bool isLooped: false
    property int elapsedTime: 0
    property int duration: 0

    onElapsedTimeChanged: {
        control.value = control.elapsedTime;
    }

    onDurationChanged: {
        control.to = control.duration;
    }


    function timeElapsed(elapsedMs){
        if (elapsedMs <= 0)
            return "+00:00";
        var elapsed = elapsedMs
        var ms = elapsed % 1000;
        elapsed = (elapsed - ms) / 1000;
        var secs = elapsed % 60;
        var mins = (elapsed - secs) / 60;

        return "+" + ("00" + mins).slice(-2) + ':' + ("00" + secs).slice(-2);
    }

    function timeRemaining(elapsedMs, durationMs) {
        if(durationMs <= 0)
            return "-00:00";
        var remaining = durationMs - elapsedMs;

        var ms = remaining % 1000;
        remaining = (remaining - ms) / 1000;
        var secs = remaining % 60;
        var mins = (remaining - secs) / 60;

        return "-" + ("00" + mins).slice(-2) + ':' + ("00" + secs).slice(-2);
    }

    handle: Rectangle {
        id: sliderHandle
        x: control.visualPosition * (control.width - width)
        y: (control.height - height) / 2
        width: 45
        height: 40
        color: control.pressed ? "#f0f0f0" : "#f6f6f6"
        border.color: "white"
        border.width: 2
        opacity: 0.87
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#414345" }
            GradientStop { position: 1.0; color: "#232526" }
        }

        Image {
            id: loopStateImage
            width: Math.floor(0.90 * control.width); height: Math.floor(0.90 * control.height)
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
            source: '/qml/icons/loop.png'
            visible: control.isLooped
        }

        ColorOverlay {
                anchors.fill: loopStateImage; source: loopStateImage
                color: "#ffffff"
                visible: control.isLooped
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

        Text {
             id: trackTimeElapsed
             x: (sliderHandle.x >= sliderHandle.width + 8 ? 5 : sliderHandle.x + sliderHandle.width + 5)
             y: Math.floor((parent.height - this.height) / 2)
             color: (sliderHandle.x >= sliderHandle.width + 8 ? "white" : "black")
             font.family: "Helvetica"
             font.bold: true
             font.pointSize: 14
             text: timeElapsed(control.elapsedTime)
        }


        Text {
             id: trackTimeRemaining
             x: (sliderHandle.x + sliderHandle.width >= parent.width - this.width - 5 ? sliderHandle.x - this.width - 5 : parent.width - this.width - 5)
             y: Math.floor((parent.height - this.height) / 2)
             color: (sliderHandle.x + sliderHandle.width >= parent.width - this.width - 5 ? "white" : "black")
             font.family: "Helvetica"
             font.bold: true
             font.pointSize: 14
             text: timeRemaining(control.elapsedTime, control.duration)
        }
    }
}
