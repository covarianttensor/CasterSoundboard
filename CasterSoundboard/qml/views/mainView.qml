import QtQuick 2.7
import QtQuick.Window 2.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

ApplicationWindow {
    id: root
    visible: true
    onVisibilityChanged: {
        if(root.visibility == Window.FullScreen)
            mainSideNav_buttonsContainer_column_button_toggleFullscreen.source = '/qml/icons/exit_fullscreen.png';
        else
            mainSideNav_buttonsContainer_column_button_toggleFullscreen.source = '/qml/icons/fullscreen.png';
    }
    title: "Caster Soundboard"
    width: 750
    height: 600
    color: Qt.rgba(0.3, 0.3, 0.3)

    Rectangle {
        id: mainSideNav
        height: parent.height
        width: 75
        color: "#333333"
        property int index: 0

        Flickable {
            id: mainSideNav_buttonsContainer_flickable
            anchors.fill: parent
            contentHeight: mainSideNav_buttonsContainer_column.height
            clip: true
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: mainSideNav_buttonsContainer_column
                spacing: 2

                Button_MainSideNav {
                    id: mainSideNav_buttonsContainer_column_button_mainSoundboardsContainerTab
                    source: '/qml/icons/soundboard_tab.png'
                    size: mainSideNav.width
                    isSelected: true //Default
                    onClicked: {
                        mainSideNav.index = 0;
                        mainSideNav_buttonsContainer_column_button_mainSoundboardsContainerTab.isSelected = true;
                        mainSideNav_buttonsContainer_column_button_oscTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_settingsTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_infoTab.isSelected = false;
                    }
                }

                Button_MainSideNav {
                    id: mainSideNav_buttonsContainer_column_button_oscTab
                    source: '/qml/icons/open_sound_control.png'
                    size: mainSideNav.width
                    onClicked: {
                        mainSideNav.index = 1;
                        mainSideNav_buttonsContainer_column_button_mainSoundboardsContainerTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_oscTab.isSelected = true;
                        mainSideNav_buttonsContainer_column_button_settingsTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_infoTab.isSelected = false;
                    }
                }

                Button_MainSideNav {
                    id: mainSideNav_buttonsContainer_column_button_settingsTab
                    source: '/qml/icons/cog.png'
                    size: mainSideNav.width
                    onClicked: {
                        mainSideNav.index = 2;
                        mainSideNav_buttonsContainer_column_button_mainSoundboardsContainerTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_oscTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_settingsTab.isSelected = true;
                        mainSideNav_buttonsContainer_column_button_infoTab.isSelected = false;
                    }
                }

                Button_MainSideNav {
                    id: mainSideNav_buttonsContainer_column_button_infoTab
                    source: '/qml/icons/about.png'
                    size: mainSideNav.width
                    onClicked: {
                        mainSideNav.index = 3;
                        mainSideNav_buttonsContainer_column_button_mainSoundboardsContainerTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_oscTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_settingsTab.isSelected = false;
                        mainSideNav_buttonsContainer_column_button_infoTab.isSelected = true;
                    }
                }

                Button_MainSideNav {
                    id: mainSideNav_buttonsContainer_column_button_toggleFullscreen
                    source: '/qml/icons/fullscreen.png'
                    size: mainSideNav.width
                    onClicked: {
                        if(root.visibility == Window.Windowed)
                            root.showFullScreen();
                        else
                            root.showNormal();
                    }
                }

            }
        }


    }


    StackLayout {
        id: mainSideNav_stackLayout
        width: parent.width - mainSideNav.width
        height: parent.height
        currentIndex: mainSideNav.index
        anchors.left:  mainSideNav.right
        // Soundboards
        Item {
            id: mainSideNav_mainSoundboardsContainerTab
            Rectangle {
                id: notroot
                anchors.fill: parent
                color: Qt.rgba(0.3, 0.3, 0.3)
                //property variant colorArray: ["#00bde3", "#67c111", "#ea7025"]

                //Soundboard Menubar
                Rectangle {
                    id: mainSoundboardsContainer_menubar
                    height: 75
                    width: parent.width
                    color: "#333333"
                    property int index: 0

                    Flickable {
                        id: mainSoundboardsContainer_menubar_flickable
                        anchors.fill: parent
                        anchors.margins: 2
                        contentWidth: mainSoundboardsContainer_menubar_row.width
                        clip: true
                        flickableDirection: Flickable.HorizontalFlick

                        Row {
                            id: mainSoundboardsContainer_menubar_row
                            spacing: 2

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/newTab.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 0
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/open.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 1
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/save.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 2
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/save_as.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 2
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/rename.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 2
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/stopAll.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 2
                            }

                            Button_SoundboardsContainer_MenuBar {
                                source: '/qml/icons/duck.png'
                                size: mainSoundboardsContainer_menubar.height
                                //onClicked: mainSideNav.index = 2
                            }

                        }
                    }


                }

                //Soundboard tab buttons
                Flickable {
                    id: mainSoundboardsContainer_tabBar_flickable
                    anchors.margins: 2
                    anchors.top: mainSoundboardsContainer_menubar.bottom
                    width: parent.width
                    height: 40
                    contentHeight: mainSoundboardsContainer_tabBar.height
                    contentWidth: mainSoundboardsContainer_tabBar.width
                    clip: true
                    flickableDirection: Flickable.HorizontalFlick

                    TabBar {
                        id: mainSoundboardsContainer_tabBar
                        TabButton {
                            text: qsTr("Home")
                            width: 150
                        }
                        TabButton {
                            text: qsTr("Discover")
                            width: 150
                        }
                        TabButton {
                            text: qsTr("Activity")
                            width: 150
                        }
                        TabButton {
                            text: qsTr("Activity 2")
                            width: 150
                        }
                    }
                }



                //Soundboard tab pages
                StackLayout {
                    id: mainSoundboardsContainer_stackLayout
                    anchors.top: mainSoundboardsContainer_tabBar_flickable.bottom
                    width: parent.width; height: parent.height - (mainSoundboardsContainer_menubar.height + mainSoundboardsContainer_tabBar_flickable.height)
                    currentIndex: mainSoundboardsContainer_tabBar.currentIndex
                    Item {
                        id: homeTab
                        Soundboard {}
                    }
                    Item {
                        id: discoverTab
                        Soundboard {}
                    }
                    Item {
                        id: activityTab
                        Soundboard {}
                    }
                    Item {
                        id: activityTab2
                        Soundboard {}
                    }

                }





            }
        }
        //
        Item {
            id: mainSideNav_oscTab
            Rectangle {
                anchors.fill: parent
                color: "#FFFFFF"
            }
        }
        //
        Item {
            id: mainSideNav_settingsTab
            Rectangle {
                anchors.fill: parent
                color: "#6A49F0"
            }
        }
        //
        Item {
            id: mainSideNav_infoTab
            Rectangle {
                anchors.fill: parent
                color: "#8B72F1"
            }
        }
    }



}
