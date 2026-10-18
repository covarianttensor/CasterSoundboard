import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    signal openDocumentationRequested()

    function calculateTotalTiles() {
        if (typeof projectController === "undefined" || !projectController.tabModel) return 0;
        var total = 0;
        for (var i = 0; i < projectController.tabModel.count; ++i) {
            var g = projectController.tabModel.gridModelAt(i);
            if (g) total += g.count;
        }
        return total;
    }

    function generateDiagnosticReport() {
        var lines = [];
        lines.push("### Caster Soundboard Diagnostic Report");
        lines.push("- **Application Version:** 2.0.0");
        lines.push("- **Framework:** Qt 6.8+ (QtQuick / QML / Multimedia)");
        lines.push("- **Core Architecture:** C++17 (Headless CasterCore static engine)");
        lines.push("- **OS Platform:** Linux (Wayland / X11)");
        lines.push("- **Active Project:** " + ((typeof projectController !== "undefined" && projectController.filePath.length > 0) ? projectController.filePath : "Unsaved Project (Memory)"));
        lines.push("- **Project Status:** " + ((typeof projectController !== "undefined" && projectController.isDirty) ? "Modified *" : "Saved / Clean"));
        lines.push("- **Soundboard Tabs:** " + ((typeof projectController !== "undefined" && projectController.tabModel) ? projectController.tabModel.count : 0));
        lines.push("- **Total Sound Tiles:** " + calculateTotalTiles());
        lines.push("- **Tile Zoom Scale:** " + ((typeof projectController !== "undefined") ? Math.round(projectController.tileScale * 100) + "%" : "100%"));
        lines.push("- **PTT Profiles:** " + ((typeof projectController !== "undefined" && projectController.pttController) ? projectController.pttController.count : 0));
        lines.push("- **PTT uinput Driver:** " + ((typeof projectController !== "undefined" && projectController.pttController && projectController.pttController.uinputAvailable) ? "Ready (/dev/uinput)" : "Fallback / User Mode"));
        lines.push("- **Audio Hardware Target:** " + ((typeof projectController !== "undefined" && projectController.outputDeviceName.length > 0) ? projectController.outputDeviceName : "System Default"));
        lines.push("- **Virtual Routing Target:** Caster Soundboard Mic (caster_sink.monitor)");
        lines.push("- **Audio Ducking:** " + ((typeof projectController !== "undefined" && projectController.isDuckingActive) ? "Active" : "Idle") + " (" + ((typeof projectController !== "undefined") ? Math.round(projectController.duckingVolumeRatio * 100) + "% volume" : "25%") + ")");
        lines.push("- **Web Remote Server:** " + ((typeof webRemoteController !== "undefined" && webRemoteController.isRunning) ? ("Online (Port " + webRemoteController.port + ")") : "Stopped"));
        lines.push("- **Model Context Protocol (MCP):** " + ((typeof mcpServerController !== "undefined" && mcpServerController.isRunning) ? ("Online (Port " + mcpServerController.port + ")") : "Stopped"));
        lines.push("- **Open Sound Control (OSC):** " + ((typeof oscController !== "undefined" && oscController.isRunning) ? ("Listening (Port " + oscController.inboundPort + ")") : "Stopped"));
        lines.push("- **Bug Tracker & Feature Requests:** https://github.com/covarianttensor/CasterSoundboard/issues");
        return lines.join("\n");
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            id: aboutFlickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: contentCol.implicitHeight + 40
            clip: true
            flickableDirection: Flickable.VerticalFlick

            Timer {
                interval: 50
                running: typeof scrollBottomOnStart !== "undefined" && scrollBottomOnStart
                repeat: false
                onTriggered: {
                    aboutFlickable.contentY = Math.max(0, aboutFlickable.contentHeight - aboutFlickable.height);
                }
            }

            // Touch-Friendly Vertical ScrollBar
            ScrollBar.vertical: ScrollBar {
                id: aboutScrollbar
                width: 14
                policy: ScrollBar.AsNeeded
                interactive: true
                background: Rectangle {
                    color: "#16181B"
                    radius: 7
                }
                contentItem: Rectangle {
                    radius: 7
                    color: aboutScrollbar.pressed ? "#8E2DE2" : (aboutScrollbar.hovered ? "#5A6270" : "#353940")
                }
            }

            ColumnLayout {
                id: contentCol
                width: parent.width - 24
                spacing: 20

                // View Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/about.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("About & System Information")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Caster Soundboard v2.0 • Linux Touch-First Soundboard & Virtual Audio Router")
                            color: "#888888"
                            font.pointSize: 10
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#33373B"
                }

                // 1. App Identity Hero Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: heroLayout.implicitHeight + 36
                    color: "#222528"
                    radius: 10
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        id: heroLayout
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 20

                        // App Icon with glow badge
                        Rectangle {
                            Layout.preferredWidth: 84
                            Layout.preferredHeight: 84
                            radius: 16
                            color: "#181A1D"
                            border.color: "#8E2DE2"
                            border.width: 1.5

                            Image {
                                anchors.centerIn: parent
                                source: "../icons/app.png"
                                width: 68
                                height: 68
                                sourceSize: Qt.size(68, 68)
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        // App Title & Description
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            RowLayout {
                                spacing: 10
                                Text {
                                    text: "Caster Soundboard"
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 17
                                }
                                Rectangle {
                                    color: "#3D1E4E"
                                    border.color: "#8E2DE2"
                                    border.width: 1
                                    radius: 4
                                    implicitWidth: versionBadgeText.implicitWidth + 12
                                    implicitHeight: 22
                                    Text {
                                        id: versionBadgeText
                                        anchors.centerIn: parent
                                        text: "v2.0.0"
                                        color: "#E0C3FC"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                                Rectangle {
                                    color: "#064E3B"
                                    border.color: "#10B981"
                                    border.width: 1
                                    radius: 4
                                    implicitWidth: platformBadgeText.implicitWidth + 12
                                    implicitHeight: 22
                                    Text {
                                        id: platformBadgeText
                                        anchors.centerIn: parent
                                        text: "Linux Native"
                                        color: "#A7F3D0"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            Text {
                                text: qsTr("High-performance, touch-first soundboard and live audio routing tool for Linux (Wayland and X11). Built for streamers, podcasters, broadcasters, and tabletop gamers.")
                                color: "#B0B5BD"
                                font.pointSize: 9.5
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            Text {
                                text: qsTr("Originally forked from CasterSoundboard v1 (Qt Widgets) by Jupiter Broadcasting. Rebuilt from the ground up in modern C++17 and declarative QtQuick/QML.")
                                color: "#7E8590"
                                font.pointSize: 8.5
                                font.italic: true
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                spacing: 12
                                Layout.topMargin: 4

                                Button {
                                    id: copyDiagBtn
                                    implicitHeight: 34
                                    background: Rectangle {
                                        color: copyDiagBtn.hovered ? "#3E2B5A" : "#2A1D3B"
                                        radius: 6
                                        border.color: "#8E2DE2"
                                        border.width: 1
                                    }
                                    contentItem: RowLayout {
                                        spacing: 6
                                        anchors.centerIn: parent
                                        Text { text: "📋"; font.pixelSize: 11 }
                                        Text {
                                            text: qsTr("Copy System Diagnostic Report")
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.copyToClipboard(generateDiagnosticReport());
                                            copiedNoticeTimer.restart();
                                        }
                                    }
                                }

                                Text {
                                    id: copiedNotice
                                    text: qsTr("✓ Copied to clipboard!")
                                    color: "#10B981"
                                    font.bold: true
                                    font.pointSize: 9
                                    visible: copiedNoticeTimer.running
                                }

                                Timer {
                                    id: copiedNoticeTimer
                                    interval: 2500
                                    repeat: false
                                }

                                Item { Layout.fillWidth: true }

                                Button {
                                    id: githubBtn
                                    implicitHeight: 34
                                    background: Rectangle {
                                        color: githubBtn.hovered ? "#3A3F47" : "#282C32"
                                        radius: 6
                                        border.color: "#454B54"
                                        border.width: 1
                                    }
                                    contentItem: RowLayout {
                                        spacing: 6
                                        anchors.centerIn: parent
                                        Text { text: "🔗"; font.pixelSize: 11 }
                                        Text {
                                            text: qsTr("GitHub Repository")
                                            color: "#D1D5DB"
                                            font.pointSize: 9
                                        }
                                    }
                                    onClicked: Qt.openUrlExternally("https://github.com/covarianttensor/CasterSoundboard")
                                }
                            }
                        }
                    }
                }

                // 2. Active Project Information Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: projectCardCol.implicitHeight + 32
                    color: "#222528"
                    radius: 10
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: projectCardCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        RowLayout {
                            spacing: 8
                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#8E2DE2"
                            }
                            Text {
                                text: qsTr("Active Project Information")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 12
                            }
                            Item { Layout.fillWidth: true }
                            // Dirty / Saved indicator
                            Rectangle {
                                color: (typeof projectController !== "undefined" && projectController.isDirty) ? "#4A2B0F" : "#064E3B"
                                border.color: (typeof projectController !== "undefined" && projectController.isDirty) ? "#F59E0B" : "#10B981"
                                border.width: 1
                                radius: 4
                                implicitWidth: projectStatusText.implicitWidth + 14
                                implicitHeight: 22
                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    Text {
                                        text: (typeof projectController !== "undefined" && projectController.isDirty) ? "●" : "✓"
                                        color: (typeof projectController !== "undefined" && projectController.isDirty) ? "#F59E0B" : "#10B981"
                                        font.pixelSize: 8
                                    }
                                    Text {
                                        id: projectStatusText
                                        text: (typeof projectController !== "undefined" && projectController.isDirty) ? qsTr("Modified *") : qsTr("Saved")
                                        color: (typeof projectController !== "undefined" && projectController.isDirty) ? "#FDE68A" : "#A7F3D0"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }
                        }

                        // Project File Location Row
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 40
                            color: "#181A1D"
                            radius: 6
                            border.color: "#2C3036"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Text {
                                    text: qsTr("File:")
                                    color: "#7E8590"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                                Text {
                                    text: (typeof projectController !== "undefined" && projectController.filePath.length > 0)
                                          ? projectController.filePath : qsTr("(Unsaved Project)")
                                    color: (typeof projectController !== "undefined" && projectController.filePath.length > 0)
                                           ? "#FFFFFF" : "#7E8590"
                                    font.pointSize: 9
                                    elide: Text.ElideMiddle
                                    Layout.fillWidth: true
                                }
                                Button {
                                    text: qsTr("Copy Path")
                                    implicitHeight: 26
                                    visible: typeof projectController !== "undefined" && projectController.filePath.length > 0
                                    background: Rectangle {
                                        color: parent.hovered ? "#33373B" : "#222528"
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "#B0B5BD"
                                        font.pointSize: 8
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        if (typeof projectController !== "undefined") {
                                            projectController.copyToClipboard(projectController.filePath);
                                        }
                                    }
                                }
                            }
                        }

                        // Project Metric Badges (4 cards in row)
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            MetricCard {
                                Layout.fillWidth: true
                                metricLabel: qsTr("Soundboard Tabs")
                                metricValue: (typeof projectController !== "undefined" && projectController.tabModel) ? String(projectController.tabModel.count) : "0"
                                metricIcon: "🗂️"
                                accentColor: "#8E2DE2"
                            }

                            MetricCard {
                                Layout.fillWidth: true
                                metricLabel: qsTr("Sound Tiles")
                                metricValue: String(root.calculateTotalTiles())
                                metricIcon: "🎵"
                                accentColor: "#10B981"
                            }

                            MetricCard {
                                Layout.fillWidth: true
                                metricLabel: qsTr("PTT Profiles")
                                metricValue: (typeof projectController !== "undefined" && projectController.pttController) ? String(projectController.pttController.count) : "0"
                                metricIcon: "🎙️"
                                accentColor: "#3B82F6"
                            }

                            MetricCard {
                                Layout.fillWidth: true
                                metricLabel: qsTr("Tile Zoom Scale")
                                metricValue: (typeof projectController !== "undefined") ? Math.round(projectController.tileScale * 100) + "%" : "100%"
                                metricIcon: "🔍"
                                accentColor: "#F59E0B"
                            }
                        }
                    }
                }

                // 3. Audio Engine & Virtual Routing Status Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: audioCardCol.implicitHeight + 32
                    color: "#222528"
                    radius: 10
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: audioCardCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            spacing: 8
                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#3B82F6"
                            }
                            Text {
                                text: qsTr("Audio Engine & Virtual Routing")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 12
                            }
                        }

                        GridLayout {
                            columns: 2
                            rowSpacing: 10
                            columnSpacing: 20
                            Layout.fillWidth: true

                            InfoRow {
                                label: qsTr("Audio Subsystem:")
                                value: qsTr("Direct PipeWire & PulseAudio integration via pactl")
                            }
                            InfoRow {
                                label: qsTr("Virtual Mic Endpoint:")
                                value: qsTr("Caster Soundboard Mic (caster_sink.monitor)")
                            }
                            InfoRow {
                                label: qsTr("Direct Hardware Output:")
                                value: (typeof projectController !== "undefined" && projectController.outputDeviceName.length > 0)
                                       ? projectController.outputDeviceName : qsTr("System Default Audio Device")
                            }
                            InfoRow {
                                label: qsTr("PTT Automation Driver:")
                                value: (typeof projectController !== "undefined" && projectController.pttController && projectController.pttController.uinputAvailable)
                                       ? qsTr("✓ Ready (/dev/uinput virtual keyboard)")
                                       : qsTr("Simulated / User-mode key injector")
                            }
                            InfoRow {
                                label: qsTr("Voice-Over Ducking:")
                                value: (typeof projectController !== "undefined")
                                       ? ((projectController.isDuckingActive ? qsTr("Active (") : qsTr("Idle (")) + Math.round(projectController.duckingVolumeRatio * 100) + qsTr("% level)"))
                                       : qsTr("Idle (25% level)")
                            }
                            InfoRow {
                                label: qsTr("Multi-Codec Waveforms:")
                                value: qsTr("Native PCM WAV parser + FFmpeg downsampling (MP3, OGG, FLAC, M4A)")
                            }
                        }
                    }
                }

                // 4. Remote Control Surfaces & AI Integrations Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: remoteCardCol.implicitHeight + 32
                    color: "#222528"
                    radius: 10
                    border.color: "#33373B"
                    border.width: 1

                    ColumnLayout {
                        id: remoteCardCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            spacing: 8
                            Rectangle {
                                width: 4
                                height: 18
                                radius: 2
                                color: "#10B981"
                            }
                            Text {
                                text: qsTr("Remote Control & Network Interfaces")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 12
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 14

                            ServiceCard {
                                Layout.fillWidth: true
                                title: qsTr("Local Web Remote")
                                isOnline: typeof webRemoteController !== "undefined" && webRemoteController.isRunning
                                detail: isOnline ? qsTr("Listening on port %1").arg(webRemoteController.port) : qsTr("Stopped")
                                actionText: qsTr("Open Browser")
                                onActionClicked: {
                                    if (typeof webRemoteController !== "undefined" && webRemoteController.isRunning) {
                                        Qt.openUrlExternally("http://127.0.0.1:" + webRemoteController.port);
                                    }
                                }
                            }

                            ServiceCard {
                                Layout.fillWidth: true
                                title: qsTr("Model Context Protocol (MCP)")
                                isOnline: typeof mcpServerController !== "undefined" && mcpServerController.isRunning
                                detail: isOnline ? qsTr("HTTP/SSE on port %1 & stdio bridge").arg(mcpServerController.port) : qsTr("Stopped")
                                actionText: qsTr("Status")
                                onActionClicked: {
                                    if (typeof mcpServerController !== "undefined" && mcpServerController.isRunning) {
                                        Qt.openUrlExternally("http://127.0.0.1:" + mcpServerController.port + "/status");
                                    }
                                }
                            }

                            ServiceCard {
                                Layout.fillWidth: true
                                title: qsTr("Open Sound Control (OSC)")
                                isOnline: typeof oscController !== "undefined" && oscController.isRunning
                                detail: isOnline ? qsTr("Two-way UDP port %1").arg(oscController.inboundPort) : qsTr("Stopped")
                                actionText: ""
                            }
                        }
                    }
                }

                // 5. Bug Reports & Feature Requests Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: feedbackCardCol.implicitHeight + 28
                    color: "#222528"
                    radius: 10
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        id: feedbackCardCol
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Text {
                            text: "🐛"
                            font.pixelSize: 24
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3

                            RowLayout {
                                spacing: 8
                                Text {
                                    text: qsTr("Bug Reports & Feature Requests")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }
                                Rectangle {
                                    color: "#1E293B"
                                    border.color: "#3B82F6"
                                    border.width: 1
                                    radius: 4
                                    implicitWidth: issuesBadgeText.implicitWidth + 10
                                    implicitHeight: 18
                                    Text {
                                        id: issuesBadgeText
                                        anchors.centerIn: parent
                                        text: "GitHub Issues"
                                        color: "#93C5FD"
                                        font.bold: true
                                        font.pointSize: 7.5
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Encountered an audio glitch, crash, or have an idea for a new feature? You can report bugs or submit feature requests on our GitHub tracker:")
                                color: "#A0A7B4"
                                font.pointSize: 9
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            Text {
                                text: "https://github.com/covarianttensor/CasterSoundboard/issues"
                                color: "#00D2FF"
                                font.family: "Monospace"
                                font.pointSize: 8.5
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            id: openIssuesBtn
                            implicitHeight: 34
                            background: Rectangle {
                                color: openIssuesBtn.hovered ? "#2563EB" : "#1D4ED8"
                                radius: 6
                                border.color: "#3B82F6"
                                border.width: 1
                            }
                            contentItem: RowLayout {
                                spacing: 6
                                anchors.centerIn: parent
                                Text { text: "🚀"; font.pixelSize: 11 }
                                Text {
                                    text: qsTr("Open GitHub Issues")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                            }
                            onClicked: Qt.openUrlExternally("https://github.com/covarianttensor/CasterSoundboard/issues")
                        }
                    }
                }

                // 6. Open Source Community & Documentation Jump Card
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: communityCardCol.implicitHeight + 28
                    color: "#1E2023"
                    radius: 10
                    border.color: "#2C3035"
                    border.width: 1

                    RowLayout {
                        id: communityCardCol
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Text { text: "📖"; font.pixelSize: 22 }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("Need help with audio setup, CLI commands, or keyboard shortcuts?")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10.5
                            }
                            Text {
                                text: qsTr("Explore comprehensive documentation, Discord audio troubleshooting steps, and production recipes.")
                                color: "#8E95A0"
                                font.pointSize: 9
                            }
                        }

                        Button {
                            id: openDocsBtn
                            implicitHeight: 34
                            background: Rectangle {
                                color: openDocsBtn.hovered ? "#9D3BF0" : "#8E2DE2"
                                radius: 6
                            }
                            contentItem: RowLayout {
                                spacing: 6
                                anchors.centerIn: parent
                                Text { text: "📚"; font.pixelSize: 11 }
                                Text {
                                    text: qsTr("Open Documentation View")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                            }
                            onClicked: root.openDocumentationRequested()
                        }
                    }
                }
            }
        }
    }

    // Reusable Sub-components
    component MetricCard : Rectangle {
        property string metricLabel: ""
        property string metricValue: ""
        property string metricIcon: ""
        property color accentColor: "#8E2DE2"

        implicitHeight: 68
        radius: 8
        color: "#181A1D"
        border.color: "#2C3036"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Text {
                text: metricIcon
                font.pixelSize: 18
            }

            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true
                Text {
                    text: metricValue
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 13
                }
                Text {
                    text: metricLabel
                    color: "#7E8590"
                    font.pointSize: 8.5
                }
            }
        }
    }

    component InfoRow : RowLayout {
        property string label: ""
        property string value: ""

        spacing: 8
        Layout.fillWidth: true

        Text {
            text: label
            color: "#7E8590"
            font.bold: true
            font.pointSize: 9
            Layout.preferredWidth: 170
        }
        Text {
            text: value
            color: "#D1D5DB"
            font.pointSize: 9
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }

    component ServiceCard : Rectangle {
        id: scRoot
        property string title: ""
        property bool isOnline: false
        property string detail: ""
        property string actionText: ""
        signal actionClicked()

        implicitHeight: 88
        radius: 8
        color: "#181A1D"
        border.color: isOnline ? "#10B981" : "#2C3036"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4

            RowLayout {
                spacing: 6
                Rectangle {
                    width: 7
                    height: 7
                    radius: 3.5
                    color: scRoot.isOnline ? "#10B981" : "#6E7684"
                }
                Text {
                    text: scRoot.title
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 9.5
                    Layout.fillWidth: true
                }
            }

            Text {
                text: scRoot.detail
                color: scRoot.isOnline ? "#A7F3D0" : "#7E8590"
                font.pointSize: 8.5
            }

            Item { Layout.fillHeight: true }

            Button {
                visible: scRoot.actionText.length > 0 && scRoot.isOnline
                text: scRoot.actionText
                implicitHeight: 22
                background: Rectangle {
                    color: parent.hovered ? "#2D3436" : "#202428"
                    radius: 4
                    border.color: "#3A3F47"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: "#8E2DE2"
                    font.bold: true
                    font.pointSize: 7.5
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: scRoot.actionClicked()
            }
        }
    }
}
