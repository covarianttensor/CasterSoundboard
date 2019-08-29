import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtMultimedia 5.9

import MVC_CasterPlayer 1.0

import "./subcomponents" as Subcomponent

Rectangle {
    id: root
    //Component properties
    property int size: 210
    width: root.size; height: root.size
    color: "transparent"
    property bool shouldKeepPlayingLoop: false
    property int seekEventTriggerCount: 0 //Needed to prevent irratic loop behavior and crashing (hacky solution, sigh)
    property int seekOffset: 2000 //Needed to prevent irratic loop behavior and crashing (hacky solution, sigh)
    property string normalTrackTitleBuffer: '<DROP FILE>'
    property string soundEffectTrackTitleBuffer: '<DROP WAV FILE>'
    property int duration: player.duration

    //Player properties, functions & events
    property bool isInPlayerMode: true
    property bool isLooped: false
    property double volume: 1.0
    property bool isPlayRegionEnabled: false
    property int playRegionBegin: 0
    property int playRegionEnd: 0
    onPlayRegionEndChanged: progressBar.playRegionEnd = root.playRegionEnd
    property int triggerStyle: 0

    property alias trackTile: trackTitle.trackTitleString
    property alias trackVolume: volumeBar.value

    onIsLoopedChanged: {
        if(root.isLooped == false)
            root.shouldKeepPlayingLoop = false;
        else if (root.isLooped == true && player.playbackState == MediaPlayer.PlayingState){
            root.shouldKeepPlayingLoop = true;
        }
    }

    function baseName(url)
    {
        var filename = url.substring(url.lastIndexOf('/')+1, url.lastIndexOf('.'));
        return filename;
    }

    onTriggerStyleChanged: {
        // Player Mode Changed
        switch (root.triggerStyle){
        case CasterPlayerModel.SoundEffectTriggerStyle:
            //SFX MODE
            root.normalTrackTitleBuffer = trackTitle.trackTitleString;
            trackTitle.trackTitleString = root.soundEffectTrackTitleBuffer;
            progressBar.visible = false;
            progressBar.enabled = false;
            playerStateOverlay.visible = false;
            playerStateColorOverlay.visible = false;
            soundEffectIconColorOverlay.visible = true;
            break;
        default:
            //NORMAL MODE
            root.soundEffectTrackTitleBuffer = trackTitle.trackTitleString;
            trackTitle.trackTitleString = root.normalTrackTitleBuffer;
            progressBar.visible = true;
            progressBar.enabled = true;
            playerStateOverlay.visible = true;
            playerStateColorOverlay.visible = true;
            soundEffectIconColorOverlay.visible = false;
            break;
        }
    }

    //Player Subcomponents

    SoundEffect {
        id: sfx

        onStatusChanged: {
            switch(sfx.status){
            case SoundEffect.Error:
                trackTitle.trackTitleString = "<ERROR>";
                break;
            case SoundEffect.Ready:
                trackTitle.trackTitleString = root.baseName(source.toString());
                root.soundEffectTrackTitleBuffer = trackTitle.trackTitleString;
                break;
            }
        }
    }

    MediaPlayer {
        id: player
        volume: volumeBar.value

        onPlaybackStateChanged: {
            switch (playbackState) {
            case MediaPlayer.StoppedState:
                // Enforce Looping
                if(root.shouldKeepPlayingLoop){
                    // Loop behavior
                    player.play();
                } else {
                    // Normal behavior
                    playerStateOverlay.source = '/qml/icons/playState_stopped.png'
                    progressBar.state = "stopped";
                }
                break;
            case MediaPlayer.PausedState:
                playerStateOverlay.source = '/qml/icons/playState_paused.png'
                progressBar.state = "paused";
                break;
            case MediaPlayer.PlayingState:
            default:
                playerStateOverlay.source = '/qml/icons/playState_playing.png'
                progressBar.state = "playing";
                // Enforce Looping
                if(root.isLooped)
                    root.shouldKeepPlayingLoop = true;
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
            root.playRegionBegin = 0;
            root.playRegionEnd = player.duration;
        }

        onPositionChanged: {
            // true position buffer
            var truePosition = player.position;
            //Enforce play region and looping
            if(root.isPlayRegionEnabled){
                if( root.seekEventTriggerCount < 1 && truePosition < root.playRegionBegin - root.seekOffset){
                    root.seekEventTriggerCount += 1;
                    truePosition = root.playRegionBegin;
                    player.seek(truePosition);
                } else if(root.seekEventTriggerCount < 1 && root.playRegionEnd < truePosition){
                    root.seekEventTriggerCount += 1;
                    if(root.isLooped === false)
                        player.stop();
                    var tempTruePostition = truePosition;
                    truePosition = root.playRegionBegin;
                    player.seek(truePosition);
                } else {
                    if(root.seekEventTriggerCount < 0)
                        root.seekEventTriggerCount = 0;
                    else
                        root.seekEventTriggerCount -= 1;
                }
            }

            //Update ProgressBar
            progressBar.elapsedTime = truePosition;
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

    Image {
        id: soundEffectIconOverlay
        visible: false
        width: Math.floor(0.35 * root.size); height: Math.floor(0.35 * root.size)
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
        source: '/qml/icons/soundboard_tab.png'
    }

    ColorOverlay {
        id: soundEffectIconColorOverlay
        visible: false
        anchors.fill: soundEffectIconOverlay; source: soundEffectIconOverlay
        color: mouseArea.pressed ? 'grey' : "#ffffff"
    }

    MouseArea {
            id: mouseArea
            visible: root.isInPlayerMode
            enabled: root.isInPlayerMode
            anchors.fill: parent
            onClicked: {
                switch (root.triggerStyle){
                case CasterPlayerModel.SoundEffectTriggerStyle:
                    // Sound Effect Mode
                    sfx.play();
                    break;
                default:
                    // Normal Mode
                    switch (player.playbackState) {
                    case MediaPlayer.StoppedState:
                        switch(root.triggerStyle){
                        default:
                        case CasterPlayerModel.PlayPauseTriggerStyle:
                            player.play();
                            break;
                        case CasterPlayerModel.PlayStopTriggerStyle:
                            player.play();
                            break;
                        case CasterPlayerModel.PlayAgainTriggerStyle:
                            if(root.isPlayRegionEnabled)
                                player.seek(root.playRegionBegin);
                            else
                                player.seek(0);
                            player.play();
                            break;
                        }
                        break;
                    case MediaPlayer.PausedState:
                        switch(root.triggerStyle){
                        default:
                        case CasterPlayerModel.PlayPauseTriggerStyle:
                            player.play();
                            break;
                        case CasterPlayerModel.PlayStopTriggerStyle:
                            player.play();
                            break;
                        case CasterPlayerModel.PlayAgainTriggerStyle:
                            if(root.isPlayRegionEnabled)
                                player.seek(root.playRegionBegin);
                            else
                                player.seek(0);
                            player.play();
                            break;
                        }
                        break;
                    default:
                    case MediaPlayer.PlayingState:
                        switch(root.triggerStyle){
                        default:
                        case CasterPlayerModel.PlayPauseTriggerStyle:
                            player.pause();
                            break;
                        case CasterPlayerModel.PlayStopTriggerStyle:
                            root.shouldKeepPlayingLoop = false;
                            player.stop();
                            break;
                        case CasterPlayerModel.PlayAgainTriggerStyle:
                            if(root.isPlayRegionEnabled)
                                player.seek(root.playRegionBegin);
                            else
                                player.seek(0);
                            break;
                        }
                        break;
                    }
                }
            }

    }

    DropArea {
            id: dropArea
            visible: root.isInPlayerMode
            enabled: root.isInPlayerMode
            anchors.fill: parent
            onDropped: {
                switch (root.triggerStyle){
                case CasterPlayerModel.SoundEffectTriggerStyle:
                    sfx.source = drop.urls[0];
                    break;
                default:
                    player.source = drop.urls[0];
                    break;
                }
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
        isLooped: root.isLooped
        isPlayRegionEnabled: root.isPlayRegionEnabled
        playRegionBegin: root.playRegionBegin
        playRegionEnd: root.playRegionEnd
        x: 5 ; y: root.height - this.height - 12;
        onMoved: {
            player.seek(progressBar.value);
        }
    }

    Subcomponent.VolumeSlider {
        id: volumeBar
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: root.size - this.width - 5; y: 10;
        value: root.volume
        onMoved: {
            player.volume = volumeBar.value;//Change volume
            root.volume = volumeBar.value;//Update model since we are changing volume internally
        }
    }

    Subcomponent.HotkeyIndicator {
        id: hotkeyIndicator
        visible: root.isInPlayerMode
        enabled: root.isInPlayerMode
        x: 10; y:10
        onClicked: {
            casterPlayerSettingsDrawer.openSettings(index);
        }
    }


}
