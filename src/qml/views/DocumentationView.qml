import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string searchQuery: ""
    property string activeCategory: "all" // "all", "cli", "shortcuts", "osc", "audio", "ptt", "web", "mcp"
    property string copyToastText: ""

    Timer {
        id: toastTimer
        interval: 2000
        onTriggered: root.copyToastText = ""
    }

    function showToast(msg) {
        root.copyToastText = msg;
        toastTimer.restart();
    }

    function copyText(txt) {
        if (typeof projectController !== "undefined" && projectController.copyToClipboard) {
            projectController.copyToClipboard(txt);
        }
        showToast(qsTr("Copied to clipboard: %1").arg(txt));
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        // Toast Notification Badge
        Rectangle {
            z: 100
            anchors.top: parent.top
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            height: 38
            width: toastLabel.implicitWidth + 32
            radius: 19
            color: "#8E2DE2"
            visible: root.copyToastText.length > 0
            opacity: root.copyToastText.length > 0 ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            Text {
                id: toastLabel
                anchors.centerIn: parent
                text: root.copyToastText
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 10
            }
        }

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: contentCol.implicitHeight + 40
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contentCol
                width: parent.width
                spacing: 20

                // --- 1. Header ---
                RowLayout {
                    spacing: 14
                    Layout.fillWidth: true

                    Image {
                        source: "../icons/book.png"
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 42
                        sourceSize: Qt.size(42, 42)
                        fillMode: Image.PreserveAspectFit
                    }

                    ColumnLayout {
                        spacing: 2
                        Layout.fillWidth: true

                        RowLayout {
                            spacing: 8
                            Text {
                                text: qsTr("Caster Soundboard Documentation")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 18
                            }
                            Rectangle {
                                width: 56
                                height: 22
                                radius: 11
                                color: "#2B213A"
                                border.color: "#8E2DE2"
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: "v2.0.0"
                                    color: "#C285FF"
                                    font.bold: true
                                    font.pointSize: 8
                                }
                            }
                        }

                        Text {
                            text: qsTr("Complete guide to terminal arguments, CLI automation, keyboard shortcuts, OSC network control, Web Remote, and MCP AI server.")
                            color: "#888888"
                            font.pointSize: 10
                        }
                    }
                }

                // Divider
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#33373B"
                }

                // --- 2. Search & Filter Bar ---
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    // Search Box
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        radius: 8
                        color: "#222528"
                        border.color: searchInput.activeFocus ? "#8E2DE2" : "#33373B"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            Text {
                                text: "🔍"
                                font.pointSize: 12
                                opacity: 0.7
                            }

                            TextInput {
                                id: searchInput
                                Layout.fillWidth: true
                                color: "#FFFFFF"
                                font.pointSize: 10
                                clip: true
                                selectByMouse: true
                                onTextChanged: root.searchQuery = text.toLowerCase().trim()

                                Text {
                                    text: qsTr("Search CLI options, flags, commands, or shortcuts...")
                                    color: "#666666"
                                    font.pointSize: 10
                                    visible: !searchInput.text && !searchInput.activeFocus
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Button {
                                visible: searchInput.text.length > 0
                                text: "✕"
                                flat: true
                                onClicked: searchInput.text = ""
                                background: Rectangle { color: "transparent" }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#AAAAAA"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }

                    // Category Filter Pills
                    RowLayout {
                        spacing: 6

                        FilterPill {
                            label: qsTr("All Topics")
                            catId: "all"
                        }
                        FilterPill {
                            label: qsTr("CLI Arguments")
                            catId: "cli"
                        }
                        FilterPill {
                            label: qsTr("Shortcuts")
                            catId: "shortcuts"
                        }
                        FilterPill {
                            label: qsTr("OSC API")
                            catId: "osc"
                        }
                        FilterPill {
                            label: qsTr("Audio")
                            catId: "audio"
                        }
                        FilterPill {
                            label: qsTr("Push-to-Talk")
                            catId: "ptt"
                        }
                        FilterPill {
                            label: qsTr("Web Remote")
                            catId: "web"
                        }
                        FilterPill {
                            label: qsTr("MCP AI Server")
                            catId: "mcp"
                        }
                    }
                }

                // ========================================================
                // SECTION: Command Line Interface (CLI) & Terminal Arguments
                // ========================================================
                ColumnLayout {
                    id: cliSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "cli")

                    // Section Title
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Terminal & Command Line Arguments")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    // Syntax synopsis card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: synopsisCol.implicitHeight + 24
                        radius: 8
                        color: "#1E2226"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: synopsisCol
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            Text {
                                text: qsTr("SYNOPSIS & USAGE")
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 9
                                font.letterSpacing: 1.0
                            }

                            TerminalBox {
                                commandText: "./CasterSoundboard [options] [project.caster]"
                            }

                            Text {
                                text: qsTr("Launch Caster Soundboard with optional arguments to preconfigure window geometry, activate remote OSC control, load specific project profiles, run headless automated tests, or jump directly to sub-views.")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // --- CLI ARGUMENT CARDS ---

                    // 1. Project file argument
                    CliCard {
                        optFlag: "[project]"
                        optParams: qsTr("Path to .caster file")
                        optBadge: qsTr("Core Launch")
                        badgeColor: "#8E2DE2"
                        description: qsTr("Path to a Caster Soundboard project file (.caster). When provided, the project profile is loaded immediately on launch, restoring all audio tiles, cue points, tab configurations, volumes, and custom hotkeys.")
                        exampleCommand: "./CasterSoundboard ~/podcasts/live_show.caster"
                    }

                    // 2. --view <index>
                    CliCard {
                        optFlag: "--view <index>"
                        optParams: qsTr("0 | 1 | 2 | 3 | 4 | 5")
                        optBadge: qsTr("Navigation")
                        badgeColor: "#00B4D8"
                        description: qsTr("Specifies the initial view to display on launch instead of the default soundboard grid:\n • 0 = Soundboards View (main tile grid)\n • 1 = Audio Devices & Virtual Mic Routing\n • 2 = Open Sound Control (OSC) Server\n • 3 = Preferences & Workspace Settings\n • 4 = Documentation & CLI Guide\n • 5 = About & System Information")
                        exampleCommand: "./CasterSoundboard --view 5"
                    }

                    // 3. --auto-start-osc
                    CliCard {
                        optFlag: "--auto-start-osc"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("OSC / Network")
                        badgeColor: "#4EBA6F"
                        description: qsTr("Automatically binds and starts the UDP Open Sound Control server immediately upon launch without requiring manual activation in the UI. Ideal for automated broadcast start scripts or studio controllers (Elgato Stream Deck, Bitfocus Companion, QLab).")
                        exampleCommand: "./CasterSoundboard ~/shows/morning.caster --auto-start-osc"
                    }

                    // 4. --headless
                    CliCard {
                        optFlag: "--headless"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Server / Headless")
                        badgeColor: "#E74C3C"
                        description: qsTr("Runs Caster Soundboard in pure headless mode without creating any desktop GUI window, display server requirement (X11/Wayland), or GPU overhead. Automatically runs the audio engine, OSC UDP server, and Web Remote server. Ideal for Raspberry Pi, studio rack mounts, Docker, and background system services.")
                        exampleCommand: "./CasterSoundboard ~/shows/live.caster --headless"
                    }

                    // 5. --auto-start-web
                    CliCard {
                        optFlag: "--auto-start-web"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Web Remote")
                        badgeColor: "#00D2FF"
                        description: qsTr("Automatically starts the embedded HTTP and Server-Sent Events (SSE) Web Remote server upon launch, allowing wireless control from any phone, tablet, or browser on your local network.")
                        exampleCommand: "./CasterSoundboard ~/shows/morning.caster --auto-start-web"
                    }

                    // 6. --web-port <port>
                    CliCard {
                        optFlag: "--web-port <port>"
                        optParams: qsTr("Integer port (defaults to 5052)")
                        optBadge: qsTr("Web Remote")
                        badgeColor: "#00D2FF"
                        description: qsTr("Overrides the listening TCP port for the embedded Web Remote server.")
                        exampleCommand: "./CasterSoundboard --auto-start-web --web-port 8080"
                    }

                    // 7. --web-user <user> & --web-pass <password>
                    CliCard {
                        optFlag: "--web-user <user>  /  --web-pass <pass>"
                        optParams: qsTr("Strings (defaults to 'caster' / 'caster')")
                        optBadge: qsTr("Web Remote")
                        badgeColor: "#00D2FF"
                        description: qsTr("Configures the HTTP Basic authentication username and password for browser access. Disallows colons in username for security.")
                        exampleCommand: "./CasterSoundboard --auto-start-web --web-user studio --web-pass secret123"
                    }

                    // 8. --auto-start-mcp
                    CliCard {
                        optFlag: "--auto-start-mcp"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("MCP / AI")
                        badgeColor: "#8E2DE2"
                        description: qsTr("Automatically binds and starts the Model Context Protocol (MCP) HTTP and Server-Sent Events server upon launch (default port 5055). Allows AI assistants (Claude Desktop, Antigravity, Cursor) to control Caster Soundboard.")
                        exampleCommand: "./CasterSoundboard ~/shows/morning.caster --auto-start-mcp"
                    }

                    // 9. --mcp-port <port>
                    CliCard {
                        optFlag: "--mcp-port <port>"
                        optParams: qsTr("Integer port (defaults to 5055)")
                        optBadge: qsTr("MCP / AI")
                        badgeColor: "#8E2DE2"
                        description: qsTr("Overrides the listening TCP port for the embedded Model Context Protocol (MCP) server.")
                        exampleCommand: "./CasterSoundboard --auto-start-mcp --mcp-port 5060"
                    }

                    // 10. --mcp-stdio
                    CliCard {
                        optFlag: "--mcp-stdio"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("MCP / AI")
                        badgeColor: "#8E2DE2"
                        description: qsTr("Runs the Model Context Protocol (MCP) JSON-RPC loop directly over standard input/output (stdio) in headless mode. Ideal for direct process spawning by AI desktop applications.")
                        exampleCommand: "./CasterSoundboard ~/shows/morning.caster --mcp-stdio"
                    }

                    // 4. -s, --screenshot <path>
                    CliCard {
                        optFlag: "-s, --screenshot <path>"
                        optParams: qsTr("Output image file path (.png)")
                        optBadge: qsTr("Automation & CI")
                        badgeColor: "#E056FD"
                        description: qsTr("Captures a complete high-resolution screenshot of the application window to the specified file path and then exits immediately with status code 0. Perfect for headless automated testing, CI pipelines, and visual regression verification.")
                        exampleCommand: "./CasterSoundboard -s /tmp/soundboard_snapshot.png"
                    }

                    // 5. --window-width <pixels> & --window-height <pixels>
                    CliCard {
                        optFlag: "--window-width <px>  /  --window-height <px>"
                        optParams: qsTr("Width and height in integer pixels")
                        optBadge: qsTr("Window Geometry")
                        badgeColor: "#F39C12"
                        description: qsTr("Forces the application window to initialize with exact pixel dimensions, overriding previously saved project geometry. Minimum dimensions are 640x480.")
                        exampleCommand: "./CasterSoundboard --window-width 1920 --window-height 1080"
                    }

                    // 5b. -f, --fullscreen
                    CliCard {
                        optFlag: "-f, --fullscreen"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Window Geometry")
                        badgeColor: "#F39C12"
                        description: qsTr("Launches Caster Soundboard directly in fullscreen mode. Fullscreen can also be toggled dynamically at any time using the F11 keyboard shortcut.")
                        exampleCommand: "./CasterSoundboard -f"
                    }

                    // 6. --open-drawer
                    CliCard {
                        optFlag: "--open-drawer"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Drawer & Editing")
                        badgeColor: "#3498DB"
                        description: qsTr("Opens the sound tile settings drawer immediately upon launch. Useful for quick inspection, cue editing, volume configuration, and color selection.")
                        exampleCommand: "./CasterSoundboard --open-drawer"
                    }

                    // 7. --drawer-index <index>
                    CliCard {
                        optFlag: "--drawer-index <index>"
                        optParams: qsTr("Integer (0 to 11)")
                        optBadge: qsTr("Drawer & Editing")
                        badgeColor: "#3498DB"
                        description: qsTr("Used in combination with --open-drawer to select which sound tile on the active tab is opened for editing.")
                        exampleCommand: "./CasterSoundboard --open-drawer --drawer-index 2"
                    }

                    // 8. --scroll-cue
                    CliCard {
                        optFlag: "--scroll-cue"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Drawer & Editing")
                        badgeColor: "#3498DB"
                        description: qsTr("Opens the settings drawer and automatically scrolls down to the interactive Cue In / Cue Out & Waveform editor on startup.")
                        exampleCommand: "./CasterSoundboard --open-drawer --drawer-index 0 --scroll-cue"
                    }

                    // 9. --test-seek-ms <ms>
                    CliCard {
                        optFlag: "--test-seek-ms <ms>"
                        optParams: qsTr("Playback position in milliseconds")
                        optBadge: qsTr("Audio Testing")
                        badgeColor: "#E67E22"
                        description: qsTr("Immediately seeks playback on Tile 0 to the specified millisecond offset. Used for visual progress confirmation, waveform playback verification, and automated tests.")
                        exampleCommand: "./CasterSoundboard --test-seek-ms 4500"
                    }

                    // 10. --open-hotkey-prompt
                    CliCard {
                        optFlag: "--open-hotkey-prompt"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Dialogs")
                        badgeColor: "#9B59B6"
                        description: qsTr("Pops up the modal Tile Hotkey Assignment dialog on launch, allowing immediate key assignment or automated dialog verification.")
                        exampleCommand: "./CasterSoundboard --open-hotkey-prompt"
                    }

                    // 11. --open-tab-hotkey-prompt
                    CliCard {
                        optFlag: "--open-tab-hotkey-prompt"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Dialogs")
                        badgeColor: "#9B59B6"
                        description: qsTr("Pops up the modal Tab Hotkey Assignment dialog on launch for configuring the keyboard shortcut of the active soundboard tab.")
                        exampleCommand: "./CasterSoundboard --open-tab-hotkey-prompt"
                    }

                    // 12. --open-action-hotkey-prompt <action>
                    CliCard {
                        optFlag: "--open-action-hotkey-prompt <action>"
                        optParams: qsTr("'stopAll' | 'ducking'")
                        optBadge: qsTr("Emergency Hotkeys")
                        badgeColor: "#9B59B6"
                        description: qsTr("Opens the global emergency shortcut assignment dialog for either the Stop All Sounds panic button or the Audio Ducking toggle (separate from Push-to-Talk automation).")
                        exampleCommand: "./CasterSoundboard --open-action-hotkey-prompt stopAll"
                    }

                    // 13. --open-ptt-prompt
                    CliCard {
                        optFlag: "--open-ptt-prompt"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Push-to-Talk")
                        badgeColor: "#3B82F6"
                        description: qsTr("Opens the Push-to-Talk (PTT) & Mic Automation configuration dialog on launch. Configure Discord PTT keys, lead-in/tail buffers, and tab assignments.")
                        exampleCommand: "./CasterSoundboard --open-ptt-prompt"
                    }

                    // 14. --open-delete-tab-prompt
                    CliCard {
                        optFlag: "--open-delete-tab-prompt"
                        optParams: qsTr("Flag (no arguments)")
                        optBadge: qsTr("Dialogs")
                        badgeColor: "#E74C3C"
                        description: qsTr("Opens the Delete Tab confirmation dialog on launch. Demonstrates the safety guard preventing accidental loss of tab tiles.")
                        exampleCommand: "./CasterSoundboard --open-delete-tab-prompt"
                    }

                    // 14. -h, --help & -v, --version
                    CliCard {
                        optFlag: "-h, --help  /  -v, --version  /  --help-all"
                        optParams: qsTr("Standard help flags")
                        optBadge: qsTr("Standard Info")
                        badgeColor: "#7F8C8D"
                        description: qsTr("Prints command line usage syntax, full Qt options (--help-all), or current software version to standard output and exits.")
                        exampleCommand: "./CasterSoundboard --help"
                    }

                    // Common Production Recipes
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: recipeCol.implicitHeight + 24
                        radius: 8
                        color: "#1E2226"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: recipeCol
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 12

                            Text {
                                text: qsTr("RECOMMENDED PRODUCTION WORKFLOW RECIPES")
                                color: "#8E2DE2"
                                font.bold: true
                                font.pointSize: 10
                                font.letterSpacing: 0.8
                            }

                            // Recipe 1
                            ColumnLayout {
                                spacing: 4
                                Text {
                                    text: qsTr("1. Automated Studio Launch (Project + Remote OSC Server Active):")
                                    color: "#DDDDDD"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                                TerminalBox {
                                    commandText: "./CasterSoundboard ~/Shows/MorningShow.caster --auto-start-osc"
                                }
                            }

                            // Recipe 2
                            ColumnLayout {
                                spacing: 4
                                Text {
                                    text: qsTr("2. Launch Directly into Audio Routing View at Full HD 1080p:")
                                    color: "#DDDDDD"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                                TerminalBox {
                                    commandText: "./CasterSoundboard --view 1 --window-width 1920 --window-height 1080"
                                }
                            }

                            // Recipe 3
                            ColumnLayout {
                                spacing: 4
                                Text {
                                    text: qsTr("3. Headless Automated UI Verification / Image Snapshot:")
                                    color: "#DDDDDD"
                                    font.bold: true
                                    font.pointSize: 9
                                }
                                TerminalBox {
                                    commandText: "./CasterSoundboard ~/Shows/Live.caster --view 0 -s /tmp/soundboard_ui.png"
                                }
                            }
                        }
                    }
                }

                // ========================================================
                // SECTION: Keyboard Shortcuts Reference
                // ========================================================
                ColumnLayout {
                    id: shortcutsSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "shortcuts")

                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Application Keyboard Shortcuts")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: shortcutGridCol.implicitHeight + 24
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: shortcutGridCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            ShortcutRow { key: "Space"; action: qsTr("Panic Button: Stop all playing sounds across all tabs") }
                            ShortcutRow { key: "Ctrl+D"; action: qsTr("Toggle Audio Ducking (attenuates volume for voice-over)") }
                            ShortcutRow { key: "F11"; action: qsTr("Toggle Fullscreen mode") }
                            ShortcutRow { key: "Ctrl+N"; action: qsTr("Create a new soundboard project") }
                            ShortcutRow { key: "Ctrl+O"; action: qsTr("Open an existing .caster project file") }
                            ShortcutRow { key: "Ctrl+S"; action: qsTr("Save current project") }
                            ShortcutRow { key: "Ctrl+Shift+S"; action: qsTr("Save project as a new file") }
                            ShortcutRow { key: "Ctrl+T"; action: qsTr("Add a new soundboard tab") }
                            ShortcutRow { key: "Ctrl+W"; action: qsTr("Delete / close the current soundboard tab") }
                            ShortcutRow { key: "1 - 9"; action: qsTr("Quick-switch to Tab 1–9 (Mode B) or trigger global sound tile hotkeys (Mode A)") }
                            ShortcutRow { key: "🎙️ PTT (Top Bar)"; action: qsTr("Open Push-to-Talk & Mic Automation dialog (or right-click any sound tab)") }
                        }
                    }
                }

                // ========================================================
                // SECTION: Open Sound Control (OSC) Reference
                // ========================================================
                ColumnLayout {
                    id: oscSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "osc")

                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Open Sound Control (OSC) Network API")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: oscCol.implicitHeight + 24
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: oscCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Text {
                                text: qsTr("Caster Soundboard listens on UDP port 4444 by default (configurable in the OSC view). You can send UDP packets from Bitfocus Companion, TouchOSC, QLab, or Python scripts:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            OscRow { method: "/caster/play <index>"; desc: qsTr("Triggers playback of sound tile index (0 to 11)") }
                            OscRow { method: "/caster/stop <index>"; desc: qsTr("Stops playback of sound tile index") }
                            OscRow { method: "/caster/pause <index>"; desc: qsTr("Pauses or resumes playback of sound tile index") }
                            OscRow { method: "/caster/stopAll"; desc: qsTr("Stops all sounds immediately across all tabs") }
                            OscRow { method: "/caster/duck <0|1>"; desc: qsTr("Sets audio ducking state (1 = ducked, 0 = normal)") }
                            OscRow { method: "/caster/tab <index>"; desc: qsTr("Switches the active soundboard tab") }
                            OscRow { method: "/caster/volume <0.0-1.0>"; desc: qsTr("Sets the master playback volume level") }
                        }
                    }
                }

                // ========================================================
                // SECTION: Audio Routing Architecture
                // ========================================================
                ColumnLayout {
                    id: audioSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "audio")

                    // Section Title
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Virtual Mic & Audio Device Architecture")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    // 1. Overview & 3-Step Setup Workflow
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: audioFlowCol.implicitHeight + 28
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: audioFlowCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 14

                            RowLayout {
                                spacing: 8
                                Text { text: "🎛️"; font.pixelSize: 18 }
                                ColumnLayout {
                                    spacing: 2
                                    Text {
                                        text: qsTr("Dual-Path Audio Signal Architecture")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 11
                                    }
                                    Text {
                                        text: qsTr("Plays sounds in your headphones while mixing audio into a virtual microphone for your listeners.")
                                        color: "#9AA0A6"
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            // 3-Step Setup Workflow Cards
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Step 1
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#2B1B4A"
                                            border.color: "#8E2DE2"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "1"
                                                color: "#C285FF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Set Monitor Output")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("Choose physical headphones/speakers in Audio Devices or Preferences.")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }

                                // Step 2
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#2B1B4A"
                                            border.color: "#8E2DE2"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "2"
                                                color: "#C285FF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Apply Routing")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("In Audio Devices view, select your physical mic and click Apply Routing.")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }

                                // Step 3
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#064E3B"
                                            border.color: "#10B981"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "3"
                                                color: "#6EE7B7"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Set Discord Input")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("In Discord/OBS, set Input Device to Caster Soundboard Mic.")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 2. Dual Output Pathways (Side-by-Side Cards)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        // Local Headphones Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: localHeadphonesCol.implicitHeight + 24
                            radius: 8
                            color: "#222528"
                            border.color: "#33373B"
                            border.width: 1

                            ColumnLayout {
                                id: localHeadphonesCol
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Text {
                                        text: qsTr("Local Headphones (Your Ears)")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 10
                                    }
                                    Rectangle {
                                        width: 86
                                        height: 18
                                        radius: 9
                                        color: "#2B1B4A"
                                        border.color: "#8E2DE2"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "MONITORING"
                                            color: "#C285FF"
                                            font.bold: true
                                            font.pointSize: 7.5
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Routes soundboard audio straight to your physical headphones/speakers. Does NOT loop your microphone back into your ears, preventing feedback and distracting self-echo.")
                                    color: "#CCCCCC"
                                    font.pointSize: 8.5
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    lineHeight: 1.2
                                }
                            }
                        }

                        // Virtual Mic Sink Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: virtualMicCol.implicitHeight + 24
                            radius: 8
                            color: "#222528"
                            border.color: "#33373B"
                            border.width: 1

                            ColumnLayout {
                                id: virtualMicCol
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Text {
                                        text: qsTr("Virtual Mic (Your Listeners)")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 10
                                    }
                                    Rectangle {
                                        width: 82
                                        height: 18
                                        radius: 9
                                        color: "#064E3B"
                                        border.color: "#10B981"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "BROADCAST"
                                            color: "#A7F3D0"
                                            font.bold: true
                                            font.pointSize: 7.5
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Creates a PipeWire/PulseAudio virtual sink (caster_sink.monitor) that combines your hardware microphone and soundboard clips into a single input for Discord, OBS, and games.")
                                    color: "#CCCCCC"
                                    font.pointSize: 8.5
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    lineHeight: 1.2
                                }
                            }
                        }
                    }

                    // 3. Discord & VoIP Configuration Checklist Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: voipChecklistCol.implicitHeight + 28
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: voipChecklistCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: qsTr("VoIP & Streaming Configuration Checklist (Discord, OBS, Zoom)")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10.5
                            }

                            // 3 settings in row
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 14

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    RowLayout {
                                        spacing: 6
                                        Text { text: "1️⃣"; font.pixelSize: 12 }
                                        Text {
                                            text: qsTr("Input Device")
                                            color: "#93C5FD"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    Text {
                                        text: qsTr("In Discord Settings → Voice & Video, set Input Device to Caster Soundboard Mic (do not leave on 'Default').")
                                        color: "#CCCCCC"
                                        font.pointSize: 8.5
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    RowLayout {
                                        spacing: 6
                                        Text { text: "2️⃣"; font.pixelSize: 12 }
                                        Text {
                                            text: qsTr("Turn Off Krisp AI")
                                            color: "#F59E0B"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    Text {
                                        text: qsTr("Set Noise Suppression to None/Off. Krisp cancels non-vocal audio (music, explosions) making sounds silent to others!")
                                        color: "#CCCCCC"
                                        font.pointSize: 8.5
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    RowLayout {
                                        spacing: 6
                                        Text { text: "3️⃣"; font.pixelSize: 12 }
                                        Text {
                                            text: qsTr("Echo Cancellation")
                                            color: "#93C5FD"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    Text {
                                        text: qsTr("Turn off Echo Cancellation and Advanced Voice Activity in Discord if sound clips stutter or cut out prematurely.")
                                        color: "#CCCCCC"
                                        font.pointSize: 8.5
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: "#2C3035"
                            }

                            // Diagnostics Command
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                RowLayout {
                                    spacing: 6
                                    Text { text: "🔍"; font.pixelSize: 12 }
                                    Text {
                                        text: qsTr("Verify PipeWire / PulseAudio Virtual Mic via CLI")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 9
                                    }
                                    Text {
                                        text: qsTr("(Checks if the caster_sink null-sink and loopback source are currently active)")
                                        color: "#7E8590"
                                        font.pointSize: 8
                                    }
                                }

                                TerminalBox {
                                    commandText: "pactl list short sinks | grep caster"
                                }

                                Text {
                                    text: qsTr("Displays caster_sink module ID. You can also view live routing status anytime in Caster Soundboard's Audio Devices and About views.")
                                    color: "#7E8590"
                                    font.pointSize: 8
                                }
                            }
                        }
                    }
                }

                // ========================================================
                // SECTION: Push-to-Talk (PTT) & Mic Automation
                // ========================================================
                ColumnLayout {
                    id: pttSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "ptt")

                    // Section Title
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#3B82F6"
                        }
                        Text {
                            text: qsTr("Push-to-Talk (PTT) & Mic Automation")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    // 1. Overview & 3-Step Quick Start
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: pttQuickStartCol.implicitHeight + 28
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: pttQuickStartCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 14

                            RowLayout {
                                spacing: 8
                                Text { text: "🎙️"; font.pixelSize: 18 }
                                ColumnLayout {
                                    spacing: 2
                                    Text {
                                        text: qsTr("Auto-Key Voice Chat During Sound Playback")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 11
                                    }
                                    Text {
                                        text: qsTr("Automatically keys up Discord, TeamSpeak, OBS, or in-game voice chat while sound tiles play.")
                                        color: "#9AA0A6"
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            // 3-Step Cards
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Step 1
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#1E3A8A"
                                            border.color: "#3B82F6"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "1"
                                                color: "#93C5FD"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Capture Key")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("Click 🎙️ PTT in top bar and press your key (e.g. V or F24).")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }

                                // Step 2
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#1E3A8A"
                                            border.color: "#3B82F6"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "2"
                                                color: "#93C5FD"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Assign to Tab")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("Link profile to a tab. The tab displays 🎙️ PTT: KEY.")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }

                                // Step 3
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 74
                                    radius: 6
                                    color: "#181A1D"
                                    border.color: "#2C3036"
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 10

                                        Rectangle {
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: "#064E3B"
                                            border.color: "#10B981"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "3"
                                                color: "#6EE7B7"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: qsTr("Play Audio")
                                                color: "#FFFFFF"
                                                font.bold: true
                                                font.pointSize: 9
                                            }
                                            Text {
                                                text: qsTr("Play any sound tile. Key holds and tab glows 🟢 TRANSMITTING.")
                                                color: "#8E95A0"
                                                font.pointSize: 8
                                                wrapMode: Text.WordWrap
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 2. Operating Modes (Side-by-Side Cards)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        // PTT Mode Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: pttModeCol.implicitHeight + 24
                            radius: 8
                            color: "#222528"
                            border.color: "#33373B"
                            border.width: 1

                            ColumnLayout {
                                id: pttModeCol
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Text {
                                        text: qsTr("Push-to-Talk Mode")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 10
                                    }
                                    Rectangle {
                                        width: 52
                                        height: 18
                                        radius: 9
                                        color: "#064E3B"
                                        border.color: "#10B981"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "HOLD"
                                            color: "#A7F3D0"
                                            font.bold: true
                                            font.pointSize: 7.5
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Emulates holding down your PTT key (e.g. KEY_V) while the sound plays, then automatically releases when playback ends. Perfect for Discord or in-game PTT.")
                                    color: "#CCCCCC"
                                    font.pointSize: 8.5
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    lineHeight: 1.2
                                }
                            }
                        }

                        // Mute Toggle Mode Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: muteModeCol.implicitHeight + 24
                            radius: 8
                            color: "#222528"
                            border.color: "#33373B"
                            border.width: 1

                            ColumnLayout {
                                id: muteModeCol
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Text {
                                        text: qsTr("Mute Toggle Mode")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 10
                                    }
                                    Rectangle {
                                        width: 48
                                        height: 18
                                        radius: 9
                                        color: "#451A03"
                                        border.color: "#F59E0B"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "TAP"
                                            color: "#FDE68A"
                                            font.bold: true
                                            font.pointSize: 7.5
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Taps your mute key (e.g. KEY_M) once before playing to unmute, and taps it again after playback finishes to re-mute. Ideal for open-mic streamers.")
                                    color: "#CCCCCC"
                                    font.pointSize: 8.5
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    lineHeight: 1.2
                                }
                            }
                        }
                    }

                    // 3. Audio Delays & Kernel uinput Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: pttTechCol.implicitHeight + 28
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: pttTechCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: qsTr("Timing & Linux Driver Settings")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10.5
                            }

                            // Delays breakdown
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 14

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    RowLayout {
                                        spacing: 6
                                        Text { text: "⏱️"; font.pixelSize: 12 }
                                        Text {
                                            text: qsTr("Audio Delay Buffer (Lead-In)")
                                            color: "#93C5FD"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    Text {
                                        text: qsTr("Delays audio playback by 50–200 ms after pressing PTT. Gives Discord time to open the mic channel so the first syllable is never chopped.")
                                        color: "#CCCCCC"
                                        font.pointSize: 8.5
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    RowLayout {
                                        spacing: 6
                                        Text { text: "⏱️"; font.pixelSize: 12 }
                                        Text {
                                            text: qsTr("PTT Release Delay (Tail)")
                                            color: "#93C5FD"
                                            font.bold: true
                                            font.pointSize: 9
                                        }
                                    }
                                    Text {
                                        text: qsTr("Holds PTT down for 100–300 ms after the sound finishes. Prevents abrupt cutoff of room reverberations and speech echoes.")
                                        color: "#CCCCCC"
                                        font.pointSize: 8.5
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: "#2C3035"
                            }

                            // Linux /dev/uinput command
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                RowLayout {
                                    spacing: 6
                                    Text { text: "⌨️"; font.pixelSize: 12 }
                                    Text {
                                        text: qsTr("Linux /dev/uinput Hardware Permissions")
                                        color: "#FFFFFF"
                                        font.bold: true
                                        font.pointSize: 9
                                    }
                                    Text {
                                        text: qsTr("(Enables key injection in full-screen games & Flatpaks without window focus)")
                                        color: "#7E8590"
                                        font.pointSize: 8
                                    }
                                }

                                TerminalBox {
                                    commandText: "sudo usermod -aG input $USER"
                                }

                                Text {
                                    text: qsTr("Run once in terminal, then log out and back in. Check status anytime in the About view (PTT Automation Driver: ✓ Ready).")
                                    color: "#7E8590"
                                    font.pointSize: 8
                                }
                            }
                        }
                    }

                    // 4. Discord Pro-Tip Callout Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: discordTipCol.implicitHeight + 20
                        radius: 8
                        color: "#231B15"
                        border.color: "#B45309"
                        border.width: 1

                        RowLayout {
                            id: discordTipCol
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 12

                            Text {
                                text: "⚠️"
                                font.pixelSize: 20
                                Layout.alignment: Qt.AlignVCenter
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: qsTr("Critical Discord Voice Setting: Disable Krisp Noise Suppression")
                                    color: "#FDE68A"
                                    font.bold: true
                                    font.pointSize: 9.5
                                }
                                Text {
                                    text: qsTr("In Discord <i>Settings → Voice & Video</i>, set <b>Noise Suppression</b> to <b>None / Off</b>. Discord's Krisp filter detects soundboard music, memes, and sound effects as background noise and completely mutes them for other voice chat participants!")
                                    color: "#D1D5DB"
                                    font.pointSize: 8.5
                                    textFormat: Text.RichText
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                // ========================================================
                // SECTION: Web Remote Touch Control & Browser Interface
                // ========================================================
                ColumnLayout {
                    id: webSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "web")

                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Web Remote Control & Touch Surface")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: webDocCol.implicitHeight + 24
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: webDocCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: qsTr("Caster Soundboard features an embedded HTTP and Server-Sent Events (SSE) server. Any smartphone, tablet (iPad/Android), or secondary laptop on your local Wi-Fi / Ethernet can act as a wireless touch-pad control surface:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            TerminalBox {
                                commandText: "http://<computer-ip>:5052  (Default credentials: caster / caster)"
                            }

                            Text {
                                text: qsTr("• <b>Touch Surface Grid:</b> Displays all active soundboard tabs with their authentic colors, labels, and hotkey badges. Tapping any tile triggers or stops playback on your host computer.\n• <b>Live Status Synchronization:</b> Powered by Server-Sent Events (SSE), all connected browsers receive instant state updates with green glowing halos for playing sounds and amber for paused audio.\n• <b>Panic & Ducking Controls:</b> Dedicated 'Stop All' panic button and 'Ducking' toggle are fixed at the top of the browser view.\n• <b>Browser Keyboard Hotkeys:</b> Whenever the browser tab has focus on a device with a physical keyboard, pressing <kbd>Space</kbd> executes Stop All, <kbd>D</kbd> toggles Ducking, and tile hotkey letters trigger sound tiles.\n• <b>Security Hardened:</b> HTTP Basic Authentication protects all endpoints. State-changing requests enforce the custom <code>X-Caster-Remote: 1</code> header to prevent cross-origin abuse.")
                                color: "#CCCCCC"
                                font.pointSize: 9
                                textFormat: Text.RichText
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                lineHeight: 1.2
                            }

                            OscRow { method: "GET /"; desc: qsTr("Serves the single-page responsive touch interface") }
                            OscRow { method: "GET /api/state"; desc: qsTr("Returns JSON snapshot of all tabs, tiles, and playback states") }
                            OscRow { method: "GET /events"; desc: qsTr("Server-Sent Events push stream for instant state synchronization") }
                            OscRow { method: "POST /api/toggle/<tab>/<tile>"; desc: qsTr("Toggles playback on a specific sound tile") }
                            OscRow { method: "POST /api/stopAll"; desc: qsTr("Panic-stops all playing sounds across all tabs") }
                            OscRow { method: "POST /api/duck"; desc: qsTr("Toggles audio ducking attenuation") }
                            OscRow { method: "POST /api/tab/<tab>"; desc: qsTr("Switches the active soundboard tab") }
                        }
                    }
                }

                // ========================================================
                // SECTION: Model Context Protocol (MCP) AI Server
                // ========================================================
                ColumnLayout {
                    id: mcpSection
                    Layout.fillWidth: true
                    spacing: 14
                    visible: (root.activeCategory === "all" || root.activeCategory === "mcp")

                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: 20
                            radius: 2
                            color: "#8E2DE2"
                        }
                        Text {
                            text: qsTr("Model Context Protocol (MCP) AI Server")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 14
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2C3035"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: mcpDocCol.implicitHeight + 24
                        radius: 8
                        color: "#222528"
                        border.color: "#33373B"
                        border.width: 1

                        ColumnLayout {
                            id: mcpDocCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Text {
                                text: qsTr("Caster Soundboard features a native <b>Model Context Protocol (MCP)</b> server implementing the official JSON-RPC 2.0 specification over HTTP/Server-Sent Events (SSE) and stdio. AI assistants such as <b>Claude Desktop</b>, <b>Antigravity</b>, and <b>Cursor</b> can directly inspect soundboard state, trigger sound effects, duck audio during live speech, adjust volumes, and query virtual audio routing:")
                                color: "#AAAAAA"
                                font.pointSize: 9
                                textFormat: Text.RichText
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            TerminalBox {
                                commandText: "http://127.0.0.1:5055/sse  (JSON-RPC POST endpoint: http://127.0.0.1:5055/message)"
                            }

                            Text {
                                text: qsTr("<b>Claude Desktop & Stdio Bridge Configuration:</b><br/>To connect Claude Desktop or command-line MCP clients, add the bundled Node.js stdio bridge (<code>scripts/caster-mcp.js</code>) to your configuration:")
                                color: "#CCCCCC"
                                font.pointSize: 9
                                textFormat: Text.RichText
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            TerminalBox {
                                commandText: "node /path/to/CasterSoundboard/scripts/caster-mcp.js"
                            }

                            Text {
                                text: qsTr("<b>Available Native MCP AI Tools:</b>")
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10
                                Layout.topMargin: 4
                            }

                            McpToolRow { toolName: "caster_get_state"; desc: qsTr("Returns JSON snapshot of all tabs, tiles, playback states, volume, and ducking") }
                            McpToolRow { toolName: "caster_trigger_sound"; desc: qsTr("Toggles or plays a sound tile by tab/tile index or title search query") }
                            McpToolRow { toolName: "caster_stop_all"; desc: qsTr("Emergency panic stop: silences all active audio channels across all tabs") }
                            McpToolRow { toolName: "caster_set_ducking"; desc: qsTr("Enables or disables audio ducking attenuation for voiceover commentary") }
                            McpToolRow { toolName: "caster_switch_tab"; desc: qsTr("Switches the active sound bank tab by index or tab name") }
                            McpToolRow { toolName: "caster_set_volume"; desc: qsTr("Sets playback volume for a specific tile or active master tab (0.0 to 1.0)") }
                            McpToolRow { toolName: "caster_get_audio_routing"; desc: qsTr("Inspects PipeWire/PulseAudio endpoints and virtual mic routing status") }
                        }
                    }
                }
            }
        }
    }

    // --- REUSABLE COMPONENTS ---

    // Category Filter Pill Component
    component FilterPill : Rectangle {
        property string label: ""
        property string catId: ""

        width: pillText.implicitWidth + 24
        height: 36
        radius: 18
        color: root.activeCategory === catId ? "#8E2DE2" : (pillMouse.containsMouse ? "#2E3338" : "#222528")
        border.color: root.activeCategory === catId ? "#A855F7" : "#383E44"
        border.width: 1

        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            id: pillText
            anchors.centerIn: parent
            text: parent.label
            color: root.activeCategory === catId ? "#FFFFFF" : "#AAAAAA"
            font.bold: root.activeCategory === catId
            font.pointSize: 9
        }

        MouseArea {
            id: pillMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.activeCategory = parent.catId
        }
    }

    // Terminal Code Snippet Box Component
    component TerminalBox : Rectangle {
        id: termItem
        property string commandText: ""

        Layout.fillWidth: true
        Layout.preferredHeight: 38
        radius: 6
        color: "#131518"
        border.color: "#2C3136"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 8
            spacing: 8

            Text {
                text: "$"
                color: "#4EBA6F"
                font.bold: true
                font.family: "Monospace"
                font.pointSize: 9
            }

            Text {
                Layout.fillWidth: true
                text: termItem.commandText
                color: "#E2E8F0"
                font.family: "Monospace"
                font.pointSize: 9
                elide: Text.ElideRight
            }

            Button {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 64
                flat: true
                onClicked: root.copyText(termItem.commandText)
                background: Rectangle {
                    color: parent.hovered ? "#2D3339" : "#1E2226"
                    radius: 4
                    border.color: "#3A4148"
                    border.width: 1
                }
                contentItem: Text {
                    text: qsTr("Copy")
                    color: parent.hovered ? "#FFFFFF" : "#B0B7C0"
                    font.bold: true
                    font.pointSize: 8
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    // CLI Argument Card Component
    component CliCard : Rectangle {
        id: cardItem
        property string optFlag: ""
        property string optParams: ""
        property string optBadge: ""
        property color badgeColor: "#8E2DE2"
        property string description: ""
        property string exampleCommand: ""

        visible: {
            if (root.searchQuery === "") return true;
            var q = root.searchQuery;
            return optFlag.toLowerCase().indexOf(q) !== -1 ||
                   description.toLowerCase().indexOf(q) !== -1 ||
                   exampleCommand.toLowerCase().indexOf(q) !== -1 ||
                   optBadge.toLowerCase().indexOf(q) !== -1;
        }

        Layout.fillWidth: true
        Layout.preferredHeight: cardCol.implicitHeight + 24
        radius: 8
        color: "#222528"
        border.color: "#33373B"
        border.width: 1

        ColumnLayout {
            id: cardCol
            anchors.fill: parent
            anchors.margins: 14
            spacing: 8

            // Top row: Flag, badge, params
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: cardItem.optFlag
                    color: "#00D2FF"
                    font.bold: true
                    font.family: "Monospace"
                    font.pointSize: 11
                }

                Text {
                    text: cardItem.optParams
                    color: "#888888"
                    font.italic: true
                    font.pointSize: 9
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Rectangle {
                    width: badgeText.implicitWidth + 14
                    height: 20
                    radius: 10
                    color: Qt.rgba(cardItem.badgeColor.r, cardItem.badgeColor.g, cardItem.badgeColor.b, 0.2)
                    border.color: cardItem.badgeColor
                    border.width: 1

                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: cardItem.optBadge
                        color: cardItem.badgeColor
                        font.bold: true
                        font.pointSize: 8
                    }
                }
            }

            // Description
            Text {
                text: cardItem.description
                color: "#CCCCCC"
                font.pointSize: 9
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.2
            }

            // Example command
            TerminalBox {
                commandText: cardItem.exampleCommand
            }
        }
    }

    // Shortcut Row Component
    component ShortcutRow : Rectangle {
        id: scRow
        property string key: ""
        property string action: ""

        visible: {
            if (root.searchQuery === "") return true;
            var q = root.searchQuery;
            return key.toLowerCase().indexOf(q) !== -1 || action.toLowerCase().indexOf(q) !== -1;
        }

        Layout.fillWidth: true
        Layout.preferredHeight: 34
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 100
                Layout.preferredHeight: 26
                radius: 4
                color: "#181A1B"
                border.color: "#4A525D"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: scRow.key
                    color: "#00D2FF"
                    font.bold: true
                    font.family: "Monospace"
                    font.pointSize: 9
                }
            }

            Text {
                Layout.fillWidth: true
                text: scRow.action
                color: "#E2E8F0"
                font.pointSize: 9
                elide: Text.ElideRight
            }
        }
    }

    // OSC Row Component
    component OscRow : Rectangle {
        id: oscItem
        property string method: ""
        property string desc: ""

        visible: {
            if (root.searchQuery === "") return true;
            var q = root.searchQuery;
            return method.toLowerCase().indexOf(q) !== -1 || desc.toLowerCase().indexOf(q) !== -1;
        }

        Layout.fillWidth: true
        Layout.preferredHeight: 34
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 26
                radius: 4
                color: "#131518"
                border.color: "#343940"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: oscItem.method
                    color: "#4EBA6F"
                    font.bold: true
                    font.family: "Monospace"
                    font.pointSize: 9
                }
            }

            Text {
                Layout.fillWidth: true
                text: oscItem.desc
                color: "#CCCCCC"
                font.pointSize: 9
                elide: Text.ElideRight
            }
        }
    }

    // MCP Tool Row Component
    component McpToolRow : Rectangle {
        id: mcpItem
        property string toolName: ""
        property string desc: ""

        visible: {
            if (root.searchQuery === "") return true;
            var q = root.searchQuery;
            return toolName.toLowerCase().indexOf(q) !== -1 || desc.toLowerCase().indexOf(q) !== -1;
        }

        Layout.fillWidth: true
        Layout.preferredHeight: 34
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 230
                Layout.preferredHeight: 26
                radius: 4
                color: "#1C1728"
                border.color: "#8E2DE2"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: mcpItem.toolName
                    color: "#C084FC"
                    font.bold: true
                    font.family: "Monospace"
                    font.pointSize: 9
                }
            }

            Text {
                Layout.fillWidth: true
                text: mcpItem.desc
                color: "#CCCCCC"
                font.pointSize: 9
                elide: Text.ElideRight
            }
        }
    }
}
