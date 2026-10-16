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
    implicitHeight: 230 * scaleFactor
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

    // Track playback fraction (0.0 to 1.0)
    readonly property real playbackFraction: {
        if (!tile || tile.durationMs <= 0) return 0.0;
        if (typeof scrubContainer !== "undefined" && scrubContainer.isScrubbing) {
            return Math.max(0.0, Math.min(1.0, scrubContainer.scrubFraction));
        }
        return Math.max(0.0, Math.min(1.0, tile.positionMs / tile.durationMs));
    }

    onTileChanged: {
        if (waveformCanvas) waveformCanvas.requestPaint();
        if (playedWaveformCanvas) playedWaveformCanvas.requestPaint();
    }

    // Background Layer Container
    Item {
        id: tileBackgroundContainer
        anchors.fill: parent
        anchors.margins: 2
        clip: true

        // 1. Dark base background fill
        Rectangle {
            anchors.fill: parent
            radius: root.radius - 2
            color: "#181A1D"
        }

        // 2. Video Output container for video files
        VideoOutput {
            id: videoOutput
            anchors.fill: parent
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
            radius: root.radius - 2
            color: "#181A1D"
            visible: tile && tile.hasVideo
            opacity: 0.45
        }

        // 3. Audio Waveform Background (Unplayed - subtle white bars)
        Canvas {
            id: waveformCanvas
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            anchors.topMargin: 10
            anchors.bottomMargin: 10
            visible: tile && !tile.hasVideo && tile.waveform && tile.waveform.length > 0
            opacity: 0.22

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                var data = (tile && tile.waveform) ? tile.waveform : [];
                if (!data || data.length === 0) return;

                var w = width;
                var h = height;
                var count = data.length;
                var step = w / count;
                var barWidth = Math.max(1.5, step * 0.68);

                ctx.fillStyle = "#FFFFFF";
                for (var i = 0; i < count; ++i) {
                    var amp = Math.max(0.06, Math.min(1.0, Number(data[i])));
                    var barH = Math.max(3, amp * (h * 0.60));
                    var barX = i * step + (step - barWidth) / 2;
                    var barY = (h - barH) / 2;
                    ctx.fillRect(barX, barY, barWidth, barH);
                }
            }

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            Component.onCompleted: requestPaint()
        }

        // 4. Visual Progress Color Sweep (changes color from left to right based on track progress)
        Rectangle {
            id: progressColorSweep
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: Math.round(parent.width * root.playbackFraction)
            visible: tile && tile.filePath && tile.filePath.length > 0 && root.playbackFraction > 0
            clip: true

            topLeftRadius: root.radius - 2
            bottomLeftRadius: root.radius - 2
            topRightRadius: (width >= parent.width - 2) ? (root.radius - 2) : 0
            bottomRightRadius: (width >= parent.width - 2) ? (root.radius - 2) : 0

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: (tile && tile.color && tile.color !== "#3A3D40") ? Qt.darker(tile.color, 1.35) : "#667eea"
                }
                GradientStop {
                    position: 1.0
                    color: (tile && tile.color && tile.color !== "#3A3D40") ? tile.color : "#764ba2"
                }
            }
            opacity: (tile && tile.playbackState === 1) ? 0.65 : 0.45

            // Illuminated played waveform bars inside the progress sweep
            Canvas {
                id: playedWaveformCanvas
                x: 6
                y: 10
                width: tileBackgroundContainer.width - 12
                height: tileBackgroundContainer.height - 20
                visible: tile && !tile.hasVideo && tile.waveform && tile.waveform.length > 0
                opacity: 0.85

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    var data = (tile && tile.waveform) ? tile.waveform : [];
                    if (!data || data.length === 0) return;

                    var w = width;
                    var h = height;
                    var count = data.length;
                    var step = w / count;
                    var barWidth = Math.max(1.5, step * 0.68);

                    ctx.fillStyle = "#FFFFFF";
                    for (var i = 0; i < count; ++i) {
                        var amp = Math.max(0.06, Math.min(1.0, Number(data[i])));
                        var barH = Math.max(3, amp * (h * 0.60));
                        var barX = i * step + (step - barWidth) / 2;
                        var barY = (h - barH) / 2;
                        ctx.fillRect(barX, barY, barWidth, barH);
                    }
                }

                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                Component.onCompleted: requestPaint()
            }

            // Playhead indicator beam at the leading right edge of the progression
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 2
                color: (tile && tile.playbackState === 1) ? "#2ECC71" : "#FFFFFF"
                opacity: 0.8
                visible: parent.width > 2 && parent.width < parent.parent.width - 2
            }
        }
    }

    Connections {
        target: tile
        function onWaveformChanged() {
            if (waveformCanvas) waveformCanvas.requestPaint();
            if (playedWaveformCanvas) playedWaveformCanvas.requestPaint();
        }
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

        // --- Header Row: Tile Color Dot, Floated-Left Hotkey Badge, Settings Cog ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 7

            // Tile Color Dot
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: (tile && tile.color && tile.color.length > 0) ? tile.color : "#8E2DE2"
                Layout.alignment: Qt.AlignVCenter
            }

            // Hotkey Badge (floated left next to color dot)
            Rectangle {
                id: hotkeyBadge
                visible: tile && tile.hotkey && tile.hotkey.length > 0
                Layout.preferredHeight: 26
                Layout.preferredWidth: hotkeyLabel.implicitWidth + 16
                Layout.alignment: Qt.AlignVCenter
                radius: 6
                color: "#252930"
                border.color: (tile && tile.color && tile.color.length > 0) ? tile.color : "#8E2DE2"
                border.width: 1.5

                Text {
                    id: hotkeyLabel
                    anchors.centerIn: parent
                    text: tile ? tile.hotkey : ""
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 10
                }

                MouseArea {
                    id: hotkeyMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.settingsRequested(tile)
                }

                ToolTip.visible: hotkeyMouse.containsMouse
                ToolTip.text: tile ? qsTr("Hotkey: %1 (Click to edit)").arg(tile.hotkey) : ""
            }

            // Spacer expanding across middle, keeping hotkey on left and cog on right
            Item {
                Layout.fillWidth: true
            }

            // Settings Cog Button
            Button {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignVCenter
                flat: true
                onClicked: root.settingsRequested(tile)

                background: Rectangle {
                    color: parent.hovered ? "#3E444E" : (parent.pressed ? "#4F5763" : "transparent")
                    radius: 6
                }

                contentItem: Image {
                    source: "icons/cog.png"
                    sourceSize.width: 22
                    sourceSize.height: 22
                    anchors.centerIn: parent
                    opacity: parent.hovered ? 1.0 : 0.75
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Tile Settings & Hotkeys")
            }
        }

        // --- Display Title Row (Dedicated row, scrolls like a stock ticker if text overflows) ---
        Item {
            id: titleRow
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(18, titleText.implicitHeight + 2)
            clip: true

            readonly property real textWidth: titleText.implicitWidth
            readonly property real gap: 48
            readonly property real cycleWidth: textWidth + gap
            readonly property bool isOverflowing: width > 0 && (textWidth > width)

            Item {
                id: tickerContainer
                height: parent.height
                width: titleRow.isOverflowing ? (titleRow.cycleWidth * 2) : parent.width
                x: 0

                // Primary Display Title
                Text {
                    id: titleText
                    anchors.verticalCenter: parent.verticalCenter
                    x: 0
                    text: tile ? (tile.displayName.length > 0 ? tile.displayName : qsTr("Empty Sound")) : qsTr("Empty")
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 10 * Math.max(0.8, scaleFactor * 0.9)
                }

                // Seamless Duplicate for Stock Ticker Loop
                Text {
                    id: titleTextCopy
                    visible: titleRow.isOverflowing
                    anchors.verticalCenter: parent.verticalCenter
                    x: titleRow.cycleWidth
                    text: titleText.text
                    color: titleText.color
                    font: titleText.font
                }
            }

            NumberAnimation {
                id: tickerAnim
                target: tickerContainer
                property: "x"
                running: titleRow.isOverflowing && root.visible
                loops: Animation.Infinite
                from: 0
                to: -titleRow.cycleWidth
                duration: Math.max(3000, titleRow.cycleWidth * 32)
                easing.type: Easing.Linear
            }

            onIsOverflowingChanged: {
                if (!isOverflowing) {
                    tickerAnim.stop();
                    tickerContainer.x = 0;
                }
            }

            onCycleWidthChanged: {
                if (isOverflowing) {
                    tickerAnim.restart();
                } else {
                    tickerAnim.stop();
                    tickerContainer.x = 0;
                }
            }

            ToolTip.visible: titleHover.containsMouse && titleRow.isOverflowing
            ToolTip.text: titleText.text

            MouseArea {
                id: titleHover
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }
        }

        // --- Center: Touch Trigger Button ---
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                id: triggerBtn
                anchors.centerIn: parent
                width: Math.max(46, 54 * scaleFactor)
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
                        color: (tile && tile.playbackState === 1) ? "#2ECC71" : ((tile && tile.color && tile.color !== "#3A3D40") ? tile.color : "#8E2DE2")
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
                        border.color: (tile && tile.playbackState === 1) ? "#2ECC71" : ((tile && tile.color && tile.color !== "#3A3D40") ? tile.color : "#8E2DE2")
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
                    border.color: (tile && tile.color && tile.color !== "#3A3D40") ? tile.color : "#8E2DE2"
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
                    preventStealing: true
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

        // --- Transport Controls: Loop, Cue Badge, Stop ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            // Loop Toggle Button
            Button {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 24
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
                Layout.preferredWidth: 28
                Layout.preferredHeight: 24
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
        }

        // --- Bottom Row: Dedicated Touch-Friendly Volume Slider (Original Development Branch Style) ---
        Item {
            id: volumeRow
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(26, 28 * scaleFactor)

            Slider {
                id: volumeSlider
                anchors.fill: parent
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: tile ? tile.volume : 1.0
                onMoved: {
                    if (tile) tile.volume = value;
                }

                // VU Meter Gradient Track (Green -> Yellow -> Red audio level emulating original development branch)
                background: Rectangle {
                    x: volumeSlider.leftPadding
                    y: volumeSlider.topPadding + (volumeSlider.availableHeight - height) / 2
                    width: volumeSlider.availableWidth
                    height: Math.max(12, 14 * scaleFactor)
                    radius: height / 2
                    color: "#181A1D"
                    border.color: volumeTouchArea.pressed ? "#8E2DE2" : (volumeTouchArea.containsMouse ? "#4A505C" : "#2C3036")
                    border.width: 1
                    clip: true

                    // Filled Volume Bar with Broadcast Audio VU Meter Gradient
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: Math.max(0, volumeSlider.visualPosition * parent.width)
                        radius: parent.radius
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "#2ECC71" } // Green
                            GradientStop { position: 0.7; color: "#F1C40F" } // Yellow
                            GradientStop { position: 1.0; color: "#E74C3C" } // Red
                        }
                    }
                }

                // Oversized Touch Handle with Dark Metallic Gradient and Volume Level Percentage
                handle: Rectangle {
                    id: volumeHandle
                    x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                    y: volumeSlider.topPadding + (volumeSlider.availableHeight - height) / 2
                    width: Math.max(48, 54 * scaleFactor)
                    height: Math.max(22, 24 * scaleFactor)
                    radius: 5
                    border.color: volumeTouchArea.pressed ? "#8E2DE2" : (volumeTouchArea.containsMouse ? "#FFFFFF" : "#7A828E")
                    border.width: volumeTouchArea.pressed ? 2 : 1.5

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#4B4E54" }
                        GradientStop { position: 1.0; color: "#23262A" }
                    }

                    // Tactile center grip line
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 10
                        height: 2
                        color: volumeTouchArea.pressed ? "#8E2DE2" : "#555A64"
                        opacity: 0.4
                    }

                    // Volume Level Percentage Text
                    Text {
                        anchors.centerIn: parent
                        color: "#FFFFFF"
                        font.bold: true
                        font.pointSize: Math.max(8, 9 * Math.max(0.8, scaleFactor * 0.9))
                        text: Math.round(volumeSlider.value * 100) + "%"
                    }
                }

                // Touch-Friendly Drag Area with flickable theft prevention
                MouseArea {
                    id: volumeTouchArea
                    anchors.fill: parent
                    preventStealing: true
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    function updateVolume(mouseX) {
                        var handleW = volumeHandle.width;
                        var trackW = volumeSlider.availableWidth - handleW;
                        if (trackW <= 0) return;
                        var relX = mouseX - volumeSlider.leftPadding - (handleW / 2);
                        var frac = Math.max(0.0, Math.min(1.0, relX / trackW));
                        volumeSlider.value = frac;
                        if (tile) tile.volume = frac;
                    }

                    onPressed: (mouse) => updateVolume(mouse.x)
                    onPositionChanged: (mouse) => {
                        if (pressed) updateVolume(mouse.x);
                    }
                }

                Connections {
                    target: tile
                    function onVolumeChanged() {
                        if (tile && !volumeTouchArea.pressed) {
                            volumeSlider.value = tile.volume;
                        }
                    }
                }

                ToolTip.visible: volumeTouchArea.containsMouse || volumeTouchArea.pressed
                ToolTip.text: qsTr("Tile Volume: %1%").arg(Math.round(volumeSlider.value * 100))
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
