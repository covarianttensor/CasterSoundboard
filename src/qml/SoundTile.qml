import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: root

    property var tile: null
    property real scaleFactor: (typeof projectController !== "undefined") ? projectController.tileScale : 1.0

    signal settingsRequested(var tile)
    signal removeRequested(var tile)
    signal extraFilesDropped(var urls)

    implicitWidth: 220 * scaleFactor
    implicitHeight: 220 * scaleFactor
    radius: 12
    color: "#202327"

    // Outline accent matches tile custom color, or green when playing, or purple by default
    border.color: {
        if (tile && tile.playbackState === 1) return "#2ECC71"; // playing = vibrant green
        if (tile && tile.color && tile.color.length > 0) return tile.color;
        return mouseArea.containsMouse ? "#8E2DE2" : "#32363D";
    }
    border.width: (tile && tile.playbackState === 1) ? 2 : (mouseArea.containsMouse ? 2 : 1)

    function formatTime(ms) {
        if (!ms || ms <= 0) return "00:00";
        var totalSec = Math.floor(ms / 1000);
        var min = Math.floor(totalSec / 60);
        var sec = totalSec % 60;
        return (min < 10 ? "0" + min : min) + ":" + (sec < 10 ? "0" + sec : sec);
    }

    // Video Output container for video files
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        anchors.margins: 2
        visible: tile && tile.hasVideo
        opacity: (tile && tile.playbackState === 1) ? 0.65 : 0.35
        fillMode: VideoOutput.PreserveAspectCrop
        clip: true

        Component.onCompleted: {
            if (tile) {
                tile.setVideoOutput(videoOutput);
            }
        }
    }

    // Dark tint overlay on top of video so controls remain clearly legible
    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        radius: 10
        color: "#181A1D"
        opacity: (tile && tile.hasVideo) ? 0.45 : 1.0
    }

    // Playing Pulse Glow
    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: "transparent"
        border.color: "#2ECC71"
        border.width: 2
        visible: tile && tile.playbackState === 1
        opacity: 0.7

        SequentialAnimation on opacity {
            running: tile && tile.playbackState === 1
            loops: Animation.Infinite
            NumberAnimation { from: 0.3; to: 0.9; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 0.9; to: 0.3; duration: 800; easing.type: Easing.InOutQuad }
        }
    }

    // Main Tile Content Layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 4

        // --- Header: Title, Hotkey Badge, Settings Button ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            // Tile Color Dot
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: (tile && tile.color && tile.color.length > 0) ? tile.color : "#8E2DE2"
            }

            // Display Title
            Text {
                id: titleText
                text: tile ? (tile.displayName.length > 0 ? tile.displayName : qsTr("Empty Sound")) : qsTr("Empty")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 10 * Math.max(0.8, scaleFactor * 0.9)
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Hotkey Badge
            Rectangle {
                visible: tile && tile.hotkey && tile.hotkey.length > 0
                Layout.preferredHeight: 20
                Layout.preferredWidth: hotkeyLabel.implicitWidth + 12
                radius: 4
                color: "#2C313A"
                border.color: "#8E2DE2"
                border.width: 1

                Text {
                    id: hotkeyLabel
                    anchors.centerIn: parent
                    text: tile ? tile.hotkey : ""
                    color: "#D19CF7"
                    font.bold: true
                    font.pointSize: 8
                }
            }

            // Settings Cog Button
            Button {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                flat: true
                onClicked: root.settingsRequested(tile)

                background: Rectangle {
                    color: parent.hovered ? "#3A3F47" : "transparent"
                    radius: 4
                }

                contentItem: Image {
                    source: "icons/cog.png"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    anchors.centerIn: parent
                    opacity: parent.hovered ? 1.0 : 0.6
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Tile Settings & Hotkeys")
            }
        }

        // --- Center: Touch Trigger Button ---
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                id: triggerBtn
                anchors.centerIn: parent
                width: Math.max(50, 64 * scaleFactor)
                height: width
                radius: width / 2
                color: {
                    if (tile && tile.playbackState === 1) return "#27AE60"; // Green playing
                    if (tile && tile.playbackState === 2) return "#F39C12"; // Orange paused
                    return triggerMouse.containsMouse ? "#8E2DE2" : "#2E333C"; // Idle
                }
                border.color: (tile && tile.playbackState === 1) ? "#2ECC71" : (triggerMouse.containsMouse ? "#A855F7" : "#454B54")
                border.width: 2

                Text {
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: (tile && tile.playbackState === 1) ? 0 : 2
                    text: {
                        if (tile && tile.playbackState === 1) return "❚❚";
                        return "▶";
                    }
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: (tile && tile.playbackState === 1) ? 14 * Math.max(0.8, scaleFactor) : 18 * Math.max(0.8, scaleFactor)
                }

                MouseArea {
                    id: triggerMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (tile) {
                            if (!tile.filePath || tile.filePath.length === 0) {
                                root.settingsRequested(tile);
                            } else {
                                tile.trigger();
                            }
                        }
                    }
                }
            }

            // Prompt when tile has no file loaded
            Text {
                anchors.top: triggerBtn.bottom
                anchors.topMargin: 4
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !tile || !tile.filePath || tile.filePath.length === 0
                text: qsTr("Drop file or click to setup")
                color: "#7E8590"
                font.pointSize: 8
            }
        }

        // --- Progress & Time Bar ---
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            // Time Readouts
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: root.formatTime(tile ? tile.positionMs : 0)
                    color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#AAAAAA"
                    font.pointSize: 8
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: root.formatTime(tile ? tile.durationMs : 0)
                    color: "#888888"
                    font.pointSize: 8
                }
            }

            // Interactive Scrubbing Track
            Item {
                id: scrubContainer
                Layout.fillWidth: true
                Layout.preferredHeight: 18

                property bool isScrubbing: false
                property real scrubFraction: 0.0

                Rectangle {
                    id: progressTrack
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: (scrubArea.containsMouse || scrubContainer.isScrubbing) ? 8 : 6
                    radius: height / 2
                    color: "#2D323A"
                    Behavior on height { NumberAnimation { duration: 120 } }

                    // Cue region marker (if enabled)
                    Rectangle {
                        id: cueRegionMarker
                        visible: tile && tile.hasCueRegion && tile.durationMs > 0
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        x: tile && tile.durationMs > 0 ? (progressTrack.width * (Math.max(0, Math.min(tile.durationMs, tile.cueBeginMs)) / tile.durationMs)) : 0
                        width: {
                            if (!tile || tile.durationMs <= 0) return 0;
                            var begin = Math.max(0, Math.min(tile.durationMs, tile.cueBeginMs));
                            var end = Math.max(begin, Math.min(tile.durationMs, tile.cueEndMs));
                            return Math.max(2, progressTrack.width * ((end - begin) / tile.durationMs));
                        }
                        color: "#E67E22"
                        opacity: 0.45
                        radius: 2
                    }

                    // Progress fill
                    Rectangle {
                        id: progressFill
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        radius: parent.radius
                        color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#8E2DE2"
                        width: {
                            if (!tile || tile.durationMs <= 0) return 0;
                            if (scrubContainer.isScrubbing) {
                                return Math.min(progressTrack.width, Math.max(0, progressTrack.width * scrubContainer.scrubFraction));
                            }
                            return Math.min(progressTrack.width, progressTrack.width * (tile.positionMs / tile.durationMs));
                        }
                    }

                    // Draggable Scrub Thumb / Handle
                    Rectangle {
                        id: scrubThumb
                        width: 14
                        height: 14
                        radius: 7
                        color: "#FFFFFF"
                        border.color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#8E2DE2"
                        border.width: 2
                        anchors.verticalCenter: parent.verticalCenter
                        x: Math.max(0, Math.min(progressTrack.width - width, progressFill.width - (width / 2)))
                        visible: (scrubArea.containsMouse || scrubContainer.isScrubbing || (tile && tile.playbackState === 1)) && (tile && tile.durationMs > 0)
                        scale: scrubContainer.isScrubbing ? 1.25 : (scrubArea.containsMouse ? 1.1 : 1.0)
                        Behavior on scale { NumberAnimation { duration: 100 } }
                    }
                }

                // Floating Scrub Tooltip
                Rectangle {
                    id: scrubTooltip
                    visible: scrubContainer.isScrubbing
                    anchors.bottom: progressTrack.top
                    anchors.bottomMargin: 6
                    x: Math.max(0, Math.min(scrubContainer.width - width, progressFill.width - (width / 2)))
                    width: scrubTooltipText.implicitWidth + 12
                    height: 22
                    radius: 4
                    color: "#111315"
                    border.color: "#8E2DE2"
                    border.width: 1

                    Text {
                        id: scrubTooltipText
                        anchors.centerIn: parent
                        text: {
                            if (!tile || tile.durationMs <= 0) return "00:00";
                            var ms = Math.round(scrubContainer.scrubFraction * tile.durationMs);
                            return root.formatTime(ms);
                        }
                        color: "#FFFFFF"
                        font.bold: true
                        font.pointSize: 8
                    }
                }

                MouseArea {
                    id: scrubArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    function updateScrub(mouseX) {
                        if (!tile || tile.durationMs <= 0) return;
                        var fraction = Math.max(0.0, Math.min(1.0, mouseX / progressTrack.width));
                        scrubContainer.scrubFraction = fraction;
                        var targetMs = Math.round(fraction * tile.durationMs);
                        tile.seek(targetMs);
                    }

                    onPressed: (mouse) => {
                        scrubContainer.isScrubbing = true;
                        updateScrub(mouse.x);
                    }

                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            updateScrub(mouse.x);
                        }
                    }

                    onReleased: (mouse) => {
                        if (scrubContainer.isScrubbing) {
                            updateScrub(mouse.x);
                            scrubContainer.isScrubbing = false;
                        }
                    }

                    onCanceled: {
                        scrubContainer.isScrubbing = false;
                    }
                }
            }
        }

        // --- Footer Controls: Loop, Cue Badge, Stop, Volume Slider ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            // Loop Toggle Button
            Button {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                flat: true
                onClicked: {
                    if (tile) tile.isLooped = !tile.isLooped;
                }

                background: Rectangle {
                    color: (tile && tile.isLooped) ? "#8E2DE2" : (parent.hovered ? "#353A42" : "transparent")
                    radius: 4
                }

                contentItem: Image {
                    source: "icons/loop.png"
                    sourceSize.width: 14
                    sourceSize.height: 14
                    anchors.centerIn: parent
                    opacity: (tile && tile.isLooped) ? 1.0 : 0.5
                }

                ToolTip.visible: hovered
                ToolTip.text: tile && tile.isLooped ? qsTr("Looping Active (Click to disable)") : qsTr("Enable Looping")
            }

            // Cue Indicator Badge
            Rectangle {
                visible: tile && tile.hasCueRegion
                Layout.preferredHeight: 18
                Layout.preferredWidth: cueText.implicitWidth + 8
                radius: 3
                color: "#D35400"
                Text {
                    id: cueText
                    anchors.centerIn: parent
                    text: qsTr("CUE")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 7
                }
                ToolTip.visible: cueMouse.containsMouse
                ToolTip.text: tile ? qsTr("Cue: %1 -> %2").arg(root.formatTime(tile.cueBeginMs)).arg(root.formatTime(tile.cueEndMs)) : ""
                MouseArea {
                    id: cueMouse
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }

            // Stop Button
            Button {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                flat: true
                onClicked: {
                    if (tile) tile.stop();
                }

                background: Rectangle {
                    color: parent.hovered ? "#353A42" : "transparent"
                    radius: 4
                }

                contentItem: Image {
                    source: "icons/stop.png"
                    sourceSize.width: 14
                    sourceSize.height: 14
                    anchors.centerIn: parent
                    opacity: (tile && tile.playbackState !== 0) ? 0.9 : 0.4
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Stop & Reset to Start")
            }

            Item { Layout.fillWidth: true }

            // Mini Volume Slider
            Slider {
                id: volSlider
                Layout.preferredWidth: Math.max(50, 70 * scaleFactor)
                Layout.preferredHeight: 22
                from: 0.0
                to: 1.0
                value: tile ? tile.volume : 1.0
                onMoved: {
                    if (tile) tile.volume = value;
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Tile Volume: %1%").arg(Math.round(volSlider.value * 100))
            }
        }
    }

    // --- Hover & Right-Click Area ---
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.RightButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                root.settingsRequested(tile);
            }
        }
    }

    // --- Drag-and-Drop Loader ---
    DropArea {
        id: dropArea
        anchors.fill: parent
        onEntered: (drag) => {
            if (drag.hasUrls) {
                drag.acceptProposedAction();
                dropOverlay.visible = true;
            }
        }
        onExited: dropOverlay.visible = false
        onDropped: (drop) => {
            dropOverlay.visible = false;
            if (drop.hasUrls && drop.urls.length > 0) {
                var url = drop.urls[0].toString();
                if (url.indexOf("file://") === 0) {
                    url = url.substring(7);
                }
                if (tile) {
                    tile.filePath = url;
                    if (tile.displayName.length === 0 || tile.displayName === "Empty Sound" || tile.displayName.indexOf("Tile ") === 0) {
                        var filename = url.substring(url.lastIndexOf('/') + 1);
                        var dotIdx = filename.lastIndexOf('.');
                        if (dotIdx > 0) filename = filename.substring(0, dotIdx);
                        tile.displayName = filename;
                    }
                }
                if (drop.urls.length > 1) {
                    var extraUrls = [];
                    for (var i = 1; i < drop.urls.length; ++i) {
                        var u = drop.urls[i].toString();
                        if (u.indexOf("file://") === 0) u = u.substring(7);
                        extraUrls.push(u);
                    }
                    root.extraFilesDropped(extraUrls);
                }
                drop.acceptProposedAction();
            }
        }
    }

    // Drag-and-Drop Visual Overlay
    Rectangle {
        id: dropOverlay
        anchors.fill: parent
        radius: root.radius
        color: "#1E824C"
        opacity: 0.88
        visible: dropArea.containsDrag

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 6
            Image {
                source: "icons/openMusic.png"
                sourceSize.width: 32
                sourceSize.height: 32
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: qsTr("Drop Media File Here")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 10
            }
        }
    }
}
