import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Rectangle {
    id: root
    property variant colorArray: ["#00bde3", "#67c111", "#ea7025"]
    width: 200; height: 200
    property int colorIndex: Math.floor(Math.random()*3)
    color: root.colorArray[colorIndex]
    border.color: "white"
    border.width: 2

    Button_Player {
        x: 5; y: root.height - 2*this.height - 10;
        source: '/qml/icons/cog.png'
    }

    ProgressSlider_Player {
        x: 5 ; y: root.height - this.height - 10;
    }

    VolumeSlider_Player {
        x: 155; y: 5;
    }
}
