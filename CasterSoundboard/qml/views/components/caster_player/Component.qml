import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtMultimedia 5.9

import "./subcomponents" as Subcomponent

Rectangle {
    id: root
    property int size: 210
    width: root.size; height: root.size
    color: "transparent"

    property bool isInPlayerMode: true
    property alias trackTile: trackTitle.trackTitleString
    property alias trackVolume: volumeBar.value
    property alias isLooped: progressBar.isLooped

    function baseName(url)
    {
        var filename = url.substring(url.lastIndexOf('/')+1, url.lastIndexOf('.'));
        return filename;
    }

    MediaPlayer {
        id: player
        volume: volumeBar.value

        onPlaybackStateChanged: {
            switch (playbackState) {
            case MediaPlayer.StoppedState:
                playerStateOverlay.source = '/qml/icons/playState_stopped.png'
                break;
            case MediaPlayer.PausedState:
                playerStateOverlay.source = '/qml/icons/playState_paused.png'
                break;
            case MediaPlayer.PlayingState:
            default:
                playerStateOverlay.source = '/qml/icons/playState_playing.png'
                break;
            }
        }

        onError: {
            trackTitle.trackTitleString = "<ERROR>";
        }

        onStatusChanged: {
            if(player.status == MediaPlayer.Loaded){
                trackTitle.trackTitleString = root.baseName(source.toString());
            }
        }

        onDurationChanged: {
            progressBar.duration = player.duration;
        }

        onPositionChanged: {
            progressBar.elapsedTime = player.position;
        }
    }

    Subcomponent.AddPlayer {
        visible: !root.isInPlayerMode
        enabled: !root.isInPlayerMode
        onClicked: {
            root.isInPlayerMode = true;
            casterPlayerController.appendItem();
        }
    }

    Image {
        id: background
        visible: root.isInPlayerMode
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: '/qml/img/player_gradient.png'
    }

    VideoOutput {
        id: videoOverlay
        enabled: root.isInPlayerMode
        visible: root.isInPlayerMode
        anchors.fill: parent
        source: player
    }

    Rectangle {
        id: gradientOverlay
        visible: root.isInPlayerMode
        width: parent.width * progressBar.visualPosition; height: parent.height
        opacity: 0.8
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#667eea" }
            GradientStop { position: 1.0; color: "#764ba2" }
        }
    }

    Image {
        id: playerStateOverlay
        visible: root.isInPlayerMode
        width: Math.floor(0.35 * root.size); height: Math.floor(0.35 * root.size)
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
        source: '/qml/icons/playState_stopped.png'
    }

    ColorOverlay {
        id: playerStateColorOverlay
        visible: root.isInPlayerMode
        anchors.fill: playerStateOverlay; source: playerStateOverlay
        color: "#ffffff"
    }

    MouseArea {
            id: mouseArea
            visible: root.isInPlayerMode
            enabled: root.isInPlayerMode
            anchors.fill: parent
            onPressed:  {
                switch (player.playbackState) {
                case MediaPlayer.StoppedState:
                    player.play();
                    break;
                case MediaPlayer.PausedState:
                    player.play();
                    break;
                case MediaPlayer.PlayingState:
                default:
                    player.pause();
                    break;
                }
            }
    }

    DropArea {
            id: dropArea
            visible: root.isInPlayerMode
            enabled: root.isInPlayerMode
            anchors.fill: parent
            onDropped: {
                player.source = drop.urls[0];
            }

    }

    Subcomponent.TrackTitle {
        id: trackTitle
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: 65; y:10
    }

    Subcomponent.ProgressBar {
        id: progressBar
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: 5 ; y: root.height - this.height - 12;
        onMoved: {
            player.seek(progressBar.value);
        }
    }

    Subcomponent.VolumeSlider {
        id: volumeBar
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: root.size - 45; y: 10;
        onMoved: {
            player.volume = volumeBar.value;
        }
    }

    Subcomponent.HotkeyIndicator {
        id: hotkeyIndicator
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: 10; y:10
        onClicked: {
            casterPlayerController.removeItemAt(index);
        }
    }


}
