import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtMultimedia 5.9

import MVC_CasterPlayer 1.0

import "./subcomponents" as Subcomponent

Drawer {
    id: root
    //Component properties
    edge: Qt.RightEdge
    interactive: false
    width: parent.width * 0.66; height: parent.height
    background: Rectangle { anchors.fill: parent; color: "#00000000" }

    //Settings properties & functions
    property int playerIndex: 0
    function openSettings(player_index){
        // interaction
        root.interactive = true;
        //Set active player
        root.playerIndex = player_index;
        //Update setting components
        settingLoopSwitch.checked = soundboard1.soundboardPlayers.itemAt(playerIndex).isLooped;
        settingPlayRegionEnabledSwitch.checked = soundboard1.soundboardPlayers.itemAt(playerIndex).isPlayRegionEnabled;
        settingPlayRegionRange.regionBegin = soundboard1.soundboardPlayers.itemAt(playerIndex).playRegionBegin;
        settingPlayRegionRange.regionEnd = soundboard1.soundboardPlayers.itemAt(playerIndex).playRegionEnd;
        settingPlayRegionRange.duration = soundboard1.soundboardPlayers.itemAt(playerIndex).duration;
        settingTriggerStyle.selectedIndex = soundboard1.soundboardPlayers.itemAt(playerIndex).triggerStyle;
        //open settings drawer
        root.open();
    }
    signal settingChanged (var role, var settingValue)
    onSettingChanged: {
        switch (role) {
        case CasterPlayerModel.IsLoopedRole:
            soundboard1.soundboardPlayers.itemAt(playerIndex).isLooped = settingValue;
            break;
        case CasterPlayerModel.IsPlayingRegionEnabledRole:
            soundboard1.soundboardPlayers.itemAt(playerIndex).isPlayRegionEnabled = settingValue;
            break;
        case CasterPlayerModel.PlayRegionBeginRole:
            soundboard1.soundboardPlayers.itemAt(playerIndex).playRegionBegin = settingValue;
            break;
        case CasterPlayerModel.PlayRegionEndRole:
            soundboard1.soundboardPlayers.itemAt(playerIndex).playRegionEnd = settingValue;
            break;
        case CasterPlayerModel.TriggerStyleRole:
            soundboard1.soundboardPlayers.itemAt(playerIndex).triggerStyle = settingValue;
            break;
        }
    }

    onClosed: root.interactive = false;

    // Settings subcomponents
    Pane {
        id: header
        width: parent.width; height: 60
        background: Rectangle { color: "#F58E2DE2" }

        Text {
            anchors.centerIn: parent
            color: 'white'
            font.bold: true
            font.pointSize: 24
            text: root.width < 300 ? (root.width < 210 ? '...' : 'Player\nSettings') : 'Player Settings'
        }

        Subcomponent.BootstrapButton {
            id: closeButton
            x: parent.width - closeButton.width
            y: parent.height/2 - closeButton.height/2
            text: ' X '; type: 'danger'
            onClicked: root.close()
        }
    }

    Rectangle {
        id: settingsArea
        anchors.top: header.bottom
        width: parent.width; height: parent.height
        color: "#DF333333"

        Flickable {
            anchors.fill: parent
            contentHeight: columnView.height + 200
            clip: true
            anchors.margins: 8
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: columnView
                width: root.width;
                spacing: 8

                Item {//Action: Remove Player
                    width: parent.width; height: 50

                    Subcomponent.BootstrapButton {
                        anchors.centerIn: parent
                        text: 'Remove Player'; type: 'danger'
                        onClicked: {
                            casterPlayerController.removeItemAt(root.playerIndex);
                            root.close();
                        }
                    }
                }

                Item {//Setting: isLooped
                    width: parent.width; height:120

                    Subcomponent.FlatSwitch {
                        id: settingLoopSwitch
                        anchors.centerIn: parent
                        questionText: "Loop sound?"
                        checkedText: "YES"
                        uncheckedText: "NO"
                        onToggled: {
                            root.settingChanged(CasterPlayerModel.IsLoopedRole, checked);
                        }
                    }
                }

                Item {//Setting: isPlayRegionEnabled
                    width: parent.width; height:120

                    Subcomponent.FlatSwitch {
                        id: settingPlayRegionEnabledSwitch
                        anchors.centerIn: parent
                        questionText: "Play region:"
                        onToggled: {
                            root.settingChanged(CasterPlayerModel.IsPlayingRegionEnabledRole, checked);
                        }
                    }
                }

                Item {//Setting: play region time range
                    width: parent.width; height: 180

                    Subcomponent.TimeRangeSelector {
                        id: settingPlayRegionRange
                        anchors.centerIn: parent

                        onRegionBeginChanged: {
                            root.settingChanged(CasterPlayerModel.PlayRegionBeginRole, regionBegin);
                        }

                        onRegionEndChanged: {
                            root.settingChanged(CasterPlayerModel.PlayRegionEndRole, regionEnd);
                        }
                    }
                }

                Item {//Setting: Trigger Style
                    width: parent.width; height: 150

                    Subcomponent.StateSelector {
                        id: settingTriggerStyle
                        anchors.centerIn: parent

                        label: "Select Trigger Style:"
                        list: ListModel {
                            ListElement {
                                name: "Play/Pause"
                            }
                            ListElement {
                                name: "Play/Stop"
                            }
                            ListElement {
                                name: "Play Again"
                            }
                            ListElement {
                                name: "Sound Effect"
                            }
                        }

                        onSelectedIndexChanged: {
                            root.settingChanged(CasterPlayerModel.TriggerStyleRole, selectedIndex);
                        }
                    }
                }



            }
        }
    }


}


