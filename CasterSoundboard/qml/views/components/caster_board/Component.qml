import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import MVC_CasterPlayer 1.0

import "../caster_player" as CasterPlayer

Flickable {
    id: root
    //Component Properties & Events
    anchors.fill: parent
    anchors.margins: 8
    contentHeight: soundboardGrid.height
    clip: true
    flickableDirection: Flickable.VerticalFlick
    ScrollBar.vertical: ScrollBar {
        id: soundboardScrollbar
        width: 30
        active: true
        onActiveChanged: {
            if (!active)
                active = true;//Keep scrollbar always visible
        }
    }
    onWidthChanged: soundboardGrid.columns = soundboardGrid.computeNeededColumns(root.defaultPlayerSize, root.width, soundboardGrid.spacing, soundboardScrollbar.width)
    onDefaultPlayerSizeChanged: soundboardGrid.columns = soundboardGrid.computeNeededColumns(root.defaultPlayerSize, root.width, soundboardGrid.spacing, soundboardScrollbar.width)

    //Player Properties
    property int defaultPlayerSize: 210
    property alias soundboardPlayers: playerRepeater

    Grid {
            id: soundboardGrid
            //Properties & Functions
            spacing: 8
            columns: soundboardGrid.computeNeededColumns(root.defaultPlayerSize, parent.width, soundboardGrid.spacing, soundboardScrollbar.width)
            function computeNeededColumns(playerWidth, containerWidth, gridSpacing, scrollbarWidth){
                var neededColumnsWithoutSpacing = Math.floor(containerWidth / playerWidth);
                var adjustedGridWidth = containerWidth - ((neededColumnsWithoutSpacing - 1) * gridSpacing) - (gridSpacing + scrollbarWidth);
                var neededColumns = Math.floor(adjustedGridWidth / playerWidth);
                neededColumns = ((neededColumns <= 0) ? 1 : neededColumns);
                return neededColumns;
            }

            //Generate Players
            Repeater {
                id: playerRepeater
                model: CasterPlayerModel { list: casterPlayerController }
                delegate: CasterPlayer.Component {
                    size: root.defaultPlayerSize
                    isInPlayerMode: model.isInPlayerMode
                    isLooped: model.isLooped
                    onIsLoopedChanged: model.isLooped = isLooped
                    volume: model.volume
                    onVolumeChanged: model.volume = volume
                    isPlayRegionEnabled: model.isPlayRegionEnabled
                    onIsPlayRegionEnabledChanged: model.isPlayRegionEnabled = isPlayRegionEnabled
                    playRegionBegin: model.playRegionBegin
                    onPlayRegionBeginChanged: model.playRegionBegin = playRegionBegin
                    playRegionEnd: model.playRegionEnd
                    onPlayRegionEndChanged: model.playRegionEnd = playRegionEnd
                    triggerStyle: model.triggerStyle
                    onTriggerStyleChanged: model.triggerStyle = triggerStyle
                }
            }
    }
}
