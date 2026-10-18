import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    component DarkTextField : TextField {
        id: dtf
        color: "#FFFFFF"
        placeholderTextColor: "#7E8590"
        implicitHeight: 40
        background: Rectangle {
            color: dtf.activeFocus ? "#1E2228" : (dtf.hovered ? "#1B1E22" : "#16191D")
            radius: 6
            border.color: dtf.activeFocus ? "#8E2DE2" : (dtf.hovered ? "#6B7280" : "#3B424D")
            border.width: dtf.activeFocus ? 2 : 1
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
        }
    }

    component DarkComboBox : ComboBox {
        id: dcb
        implicitHeight: 40
        contentItem: Text {
            leftPadding: 10
            rightPadding: dcb.indicator.width + 8
            text: dcb.displayText
            font.pointSize: 9.5
            color: dcb.enabled ? "#FFFFFF" : "#888888"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: (dcb.activeFocus || dcb.popup.visible) ? "#1E2228" : (dcb.hovered ? "#1B1E22" : "#16191D")
            radius: 6
            border.color: (dcb.activeFocus || dcb.popup.visible) ? "#8E2DE2" : (dcb.hovered ? "#6B7280" : "#3B424D")
            border.width: (dcb.activeFocus || dcb.popup.visible) ? 2 : 1
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
        }
        popup: Popup {
            y: dcb.height + 2
            width: dcb.width
            implicitHeight: Math.min(280, contentItem.implicitHeight + 10)
            padding: 4
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: dcb.popup.visible ? dcb.delegateModel : null
                currentIndex: dcb.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }
            background: Rectangle {
                color: "#202428"
                radius: 6
                border.color: "#454B54"
                border.width: 1
            }
        }
    }

    component DarkCheckBox : CheckBox {
        id: dchk
        HoverHandler { cursorShape: Qt.PointingHandCursor }
        indicator: Rectangle {
            implicitWidth: 20
            implicitHeight: 20
            x: dchk.leftPadding
            y: Math.round(dchk.height / 2 - height / 2)
            radius: 5
            color: dchk.checked ? "#8E2DE2" : (dchk.hovered ? "#363B42" : "#282C32")
            border.color: dchk.checked ? "#A855F7" : (dchk.hovered ? "#6B7280" : "#454B54")
            border.width: dchk.checked ? 1.5 : 1

            Behavior on color { ColorAnimation { duration: 120 } }
            Behavior on border.color { ColorAnimation { duration: 120 } }

            Text {
                anchors.centerIn: parent
                text: "✓"
                color: "#FFFFFF"
                font.bold: true
                font.pixelSize: 13
                visible: dchk.checked
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#181A1B"

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 24
            contentHeight: contentCol.implicitHeight + 40
            clip: true

            Timer {
                id: scrollTimer
                interval: 150
                running: typeof scrollBottomOnStart !== "undefined" && scrollBottomOnStart
                onTriggered: {
                    flickable.contentY = Math.max(0, flickable.contentHeight - flickable.height);
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contentCol
                width: flickable.width - 12
                spacing: 20

                // 1. Header
                RowLayout {
                    spacing: 12
                    Image {
                        source: "../icons/audio_devices.png"
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        sourceSize: Qt.size(36, 36)
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Audio Devices & Virtual Microphone")
                            color: "#FFFFFF"
                            font.bold: true
                            font.pointSize: 18
                        }
                        Text {
                            text: qsTr("Configure playback output destination and voice chat virtual microphone piping for Discord, OBS, or Zoom.")
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

                // 2. Status Banner & Quick Action Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(88, statusRow.implicitHeight + 32)
                    color: "#222528"
                    radius: 8
                    border.color: "#33373B"
                    border.width: 1

                    RowLayout {
                        id: statusRow
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: {
                                if (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                    return "#2ECC71"; // Active Green
                                if (typeof audioRoutingController !== "undefined" && audioRoutingController.isBusy)
                                    return "#F39C12"; // Busy Amber
                                return "#7E8590"; // Inactive Gray
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: {
                                    if (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                        return qsTr("Virtual Audio Routing Active");
                                    if (typeof audioRoutingController !== "undefined" && audioRoutingController.isBusy)
                                        return qsTr("Audio System Busy");
                                    return qsTr("Virtual Audio Routing Inactive");
                                }
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 13
                            }
                            Text {
                                text: (typeof audioRoutingController !== "undefined" && audioRoutingController.statusMessage.length > 0)
                                      ? audioRoutingController.statusMessage
                                      : qsTr("Ready — changes are applied only with Apply")
                                color: "#AAAAAA"
                                font.pointSize: 10
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            id: stopBtn
                            text: qsTr("Stop Audio Sharing")
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 44
                            visible: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                            onClicked: {
                                if (typeof audioRoutingController !== "undefined") {
                                    audioRoutingController.stopSharing();
                                }
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#922B21" : (parent.hovered ? "#C0392B" : "#A93226")
                                radius: 6
                            }
                        }

                        Button {
                            id: applyBtn
                            text: qsTr("Apply Routing")
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 44
                            enabled: typeof audioRoutingController !== "undefined" && !audioRoutingController.isBusy
                            onClicked: {
                                if (typeof audioRoutingController !== "undefined") {
                                    if (typeof micNameField !== "undefined") {
                                        audioRoutingController.virtualMicName = micNameField.text;
                                    }
                                    audioRoutingController.apply();
                                }
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#5B18A7" : (parent.hovered ? "#A044FF" : "#8E2DE2")
                                radius: 6
                                opacity: applyBtn.enabled ? 1.0 : 0.5
                            }
                        }
                    }
                }

                // Error Alert Banner (if routing failed or device missing)
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: errCol.implicitHeight + 16
                    radius: 8
                    color: "#3B1818"
                    border.color: "#E74C3C"
                    border.width: 1
                    visible: typeof audioRoutingController !== "undefined" && audioRoutingController.hasError

                    RowLayout {
                        id: errCol
                        anchors.fill: parent
                        anchors.margins: 10
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10

                        Text {
                            text: "⚠️"
                            font.pointSize: 13
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("<b>Routing Validation Failed:</b>")
                                color: "#FF8585"
                                font.bold: true
                                font.pointSize: 9.5
                            }
                            Text {
                                Layout.fillWidth: true
                                text: typeof audioRoutingController !== "undefined" ? audioRoutingController.lastErrorMessage : ""
                                color: "#FFCDD2"
                                font.pointSize: 9.0
                                wrapMode: Text.WordWrap
                            }
                        }

                        Button {
                            text: qsTr("Dismiss")
                            Layout.preferredHeight: 32
                            onClicked: audioRoutingController.clearError()
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                font.pointSize: 8.5
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#5C1D1D" : (parent.hovered ? "#782828" : "#612020")
                                radius: 4
                                border.color: "#E74C3C"
                                border.width: 1
                            }
                        }
                    }
                }

                // 2.5 Active Routing Mode Pill Banner
                Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    radius: 8
                    color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                           ? "#142B20" : "#281A35"
                    border.color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                  ? "#1E7E4E" : "#6E2EA6"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10

                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                   ? "#2ECC71" : "#A855F7"
                        }

                        Text {
                            Layout.fillWidth: true
                            text: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                  ? qsTr("<b>Virtual Audio Routing Active:</b> Voice and Soundboard audio merge at ⊕ into Virtual Microphone; clean soundboard audio plays to Headphones.")
                                  : qsTr("<b>Direct Playback Mode:</b> Soundboard audio routes directly to local speakers. Virtual microphone is bypassed.")
                            color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                   ? "#A7F3D0" : "#D8B4FE"
                            font.pointSize: 9.0
                            textFormat: Text.RichText
                            elide: Text.ElideRight
                            clip: true
                        }

                        Rectangle {
                            height: 24
                            width: modeTagText.implicitWidth + 16
                            radius: 4
                            color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                   ? "#1E4631" : "#3B2252"
                            border.color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                          ? "#2ECC71" : "#A855F7"
                            border.width: 1

                            Text {
                                id: modeTagText
                                anchors.centerIn: parent
                                text: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                      ? qsTr("ROUTING ACTIVE") : qsTr("DIRECT MODE")
                                color: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                       ? "#2ECC71" : "#D8B4FE"
                                font.bold: true
                                font.pointSize: 8.5
                                font.family: "Monospace"
                            }
                        }
                    }
                }

                // =========================================================================
                // 3. SCHEMATIC FLOW DIAGRAM BOARD (90° ORTHOGONAL CIRCUIT ROUTING)
                // =========================================================================
                Rectangle {
                    id: boardRect
                    Layout.fillWidth: true
                    implicitHeight: flowCol.implicitHeight + 36
                    color: "#131518"
                    radius: 12
                    border.color: "#2C3138"
                    border.width: 1

                    ColumnLayout {
                        id: flowCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 16

                        // Header Row with Refresh Button
                        RowLayout {
                            Layout.fillWidth: true

                            Item { Layout.fillWidth: true }

                            Button {
                                text: qsTr("Refresh Devices")
                                implicitHeight: 32
                                onClicked: {
                                    if (typeof audioRoutingController !== "undefined") {
                                        audioRoutingController.refresh();
                                    }
                                    if (typeof projectController !== "undefined") {
                                        projectController.refreshAudioDevices();
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 9
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#252930")
                                    radius: 6
                                    border.color: "#3F4652"
                                    border.width: 1
                                }
                            }
                        }

                        // Flow Grid Container
                        Item {
                            id: flowGrid
                            Layout.fillWidth: true
                            implicitHeight: rightCol.implicitHeight + 16

                            // --- ZONE 1: ORIGINATING SOURCES (Left 38% Width) ---
                            ColumnLayout {
                                id: leftCol
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: flowGrid.width * 0.38
                                spacing: 18

                                // Mic Card
                                Rectangle {
                                    id: cardMic
                                    Layout.fillWidth: true
                                    Layout.topMargin: 15
                                    implicitHeight: micCol.implicitHeight + 24
                                    color: isHoveredOrFocused ? "#22272E" : "#1C2025"
                                    radius: 10
                                    border.color: (isRouting || isHoveredOrFocused) ? "#3B82F6" : "#2E3642"
                                    border.width: isHoveredOrFocused ? 2 : 1.5
                                    opacity: isRouting ? 1.0 : (isHoveredOrFocused ? 1.0 : 0.45)

                                    readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                    readonly property bool isHoveredOrFocused: cardMicHover.hovered || micCombo.activeFocus || micCombo.popup.visible

                                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    Behavior on border.color { ColorAnimation { duration: 180 } }
                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    HoverHandler {
                                        id: cardMicHover
                                    }

                                    ColumnLayout {
                                        id: micCol
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: "🎤 " + qsTr("Physical Microphone")
                                                color: (cardMic.isRouting || cardMic.isHoveredOrFocused) ? "#93C5FD" : "#6B7280"
                                                font.bold: true
                                                font.pointSize: 10.5
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                            }
                                            Item { Layout.fillWidth: true }
                                            Rectangle {
                                                width: micBadgeTxt.implicitWidth + 10
                                                height: 18
                                                radius: 4
                                                color: (cardMic.isRouting || cardMic.isHoveredOrFocused) ? "#17253D" : "#161B22"
                                                border.color: (cardMic.isRouting || cardMic.isHoveredOrFocused) ? "#2563EB" : "#2E3642"
                                                border.width: 1
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                                Text {
                                                    id: micBadgeTxt
                                                    anchors.centerIn: parent
                                                    text: qsTr("Voice In")
                                                    color: (cardMic.isRouting || cardMic.isHoveredOrFocused) ? "#93C5FD" : "#6B7280"
                                                    font.pointSize: 8
                                                    font.bold: true
                                                    Behavior on color { ColorAnimation { duration: 180 } }
                                                }
                                            }
                                        }

                                        Text {
                                            text: qsTr("Select your hardware voice microphone:")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }

                                        DarkComboBox {
                                            id: micCombo
                                            Layout.fillWidth: true
                                            Component.onCompleted: {
                                                if (typeof focusMicOnStart !== "undefined" && focusMicOnStart) {
                                                    forceActiveFocus();
                                                }
                                            }
                                            model: (typeof audioRoutingController !== "undefined" && audioRoutingController.inputDevices.length > 0)
                                                   ? audioRoutingController.inputDevices
                                                   : [qsTr("None / Disconnected")]
                                            currentIndex: {
                                                if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedMicrophone)
                                                    return 0;
                                                var idx = micCombo.model.indexOf(audioRoutingController.selectedMicrophone);
                                                return idx >= 0 ? idx : 0;
                                            }
                                            onActivated: (index) => {
                                                if (typeof audioRoutingController !== "undefined") {
                                                    audioRoutingController.selectedMicrophone = currentText;
                                                }
                                            }
                                        }
                                    }

                                    // Right Pin
                                    Rectangle {
                                        id: pinMic
                                        anchors.right: parent.right
                                        anchors.rightMargin: -5
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: (cardMic.isRouting || cardMic.isHoveredOrFocused) ? "#3B82F6" : "#2E3642"
                                        border.color: "#131518"
                                        border.width: 2
                                        z: 10
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }
                                }

                                Item {
                                    Layout.fillHeight: true
                                    Layout.minimumHeight: 30
                                }

                                // Soundboard Card
                                Rectangle {
                                    id: cardSb
                                    Layout.fillWidth: true
                                    implicitHeight: sbCol.implicitHeight + 24
                                    color: cardSbHover.hovered ? "#221E2A" : "#1C2025"
                                    radius: 10
                                    border.color: "#A855F7"
                                    border.width: cardSbHover.hovered ? 2 : 1.5

                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    HoverHandler {
                                        id: cardSbHover
                                    }

                                    ColumnLayout {
                                        id: sbCol
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: "🔊 " + qsTr("Caster Soundboard Audio")
                                                color: "#D8B4FE"
                                                font.bold: true
                                                font.pointSize: 10.5
                                            }
                                            Item { Layout.fillWidth: true }
                                            Rectangle {
                                                width: sbBadgeTxt.implicitWidth + 10
                                                height: 18
                                                radius: 4
                                                color: "#2C1B3B"
                                                border.color: "#7E22CE"
                                                border.width: 1
                                                Text {
                                                    id: sbBadgeTxt
                                                    anchors.centerIn: parent
                                                    text: qsTr("Sound FX")
                                                    color: "#D8B4FE"
                                                    font.pointSize: 8
                                                    font.bold: true
                                                }
                                            }
                                        }

                                        Text {
                                            text: qsTr("Audio clips, music, and effects triggered on soundboard tiles:")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            height: 32
                                            radius: 6
                                            color: "#14171A"
                                            border.color: "#3E4652"
                                            border.width: 1

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                Text {
                                                    text: qsTr("Master Output Level:")
                                                    color: "#94A3B8"
                                                    font.pointSize: 9
                                                }
                                                Item { Layout.fillWidth: true }
                                                Text {
                                                    text: "100% (0 dB)"
                                                    color: "#C084FC"
                                                    font.bold: true
                                                    font.pointSize: 9
                                                    font.family: "Monospace"
                                                }
                                            }
                                        }
                                    }

                                    // Right Pin
                                    Rectangle {
                                        id: pinSb
                                        anchors.right: parent.right
                                        anchors.rightMargin: -5
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: "#A855F7"
                                        border.color: "#131518"
                                        border.width: 2
                                        z: 10
                                    }
                                }
                            }

                            // --- ZONE 3: DESTINATION ENDPOINTS (Right 38% Width) ---
                            ColumnLayout {
                                id: rightCol
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: flowGrid.width * 0.38
                                spacing: 18

                                // Virtual Mic Card
                                Rectangle {
                                    id: cardVirt
                                    Layout.fillWidth: true
                                    implicitHeight: virtCol.implicitHeight + 24
                                    color: isHoveredOrFocused ? "#202824" : "#1C2025"
                                    radius: 10
                                    border.color: (isRouting || isHoveredOrFocused) ? "#10B981" : "#22352B"
                                    border.width: isHoveredOrFocused ? 2 : 1.5
                                    opacity: isRouting ? 1.0 : (isHoveredOrFocused ? 1.0 : 0.45)

                                    readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                    readonly property bool isHoveredOrFocused: cardVirtHover.hovered || micNameField.activeFocus || copyVirtBtn.hovered || copyVirtBtn.activeFocus

                                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    Behavior on border.color { ColorAnimation { duration: 180 } }
                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    HoverHandler {
                                        id: cardVirtHover
                                    }

                                    ColumnLayout {
                                        id: virtCol
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: "🎙️ " + qsTr("Virtual Microphone")
                                                color: (cardVirt.isRouting || cardVirt.isHoveredOrFocused) ? "#6EE7B7" : "#6B7280"
                                                font.bold: true
                                                font.pointSize: 10.5
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                            }
                                            Item { Layout.fillWidth: true }
                                            Rectangle {
                                                width: virtBadgeTxt.implicitWidth + 10
                                                height: 18
                                                radius: 4
                                                color: (cardVirt.isRouting || cardVirt.isHoveredOrFocused) ? "#163125" : "#161B22"
                                                border.color: (cardVirt.isRouting || cardVirt.isHoveredOrFocused) ? "#059669" : "#22352B"
                                                border.width: 1
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                                Text {
                                                    id: virtBadgeTxt
                                                    anchors.centerIn: parent
                                                    text: qsTr("VoIP / Stream")
                                                    color: (cardVirt.isRouting || cardVirt.isHoveredOrFocused) ? "#6EE7B7" : "#6B7280"
                                                    font.pointSize: 8
                                                    font.bold: true
                                                    Behavior on color { ColorAnimation { duration: 180 } }
                                                }
                                            }
                                        }

                                        Text {
                                            text: qsTr("In receiving app (Discord, OBS, Zoom), set Input Device to:")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 6

                                            DarkTextField {
                                                id: micNameField
                                                Layout.fillWidth: true
                                                Component.onCompleted: {
                                                    if (typeof focusVirtOnStart !== "undefined" && focusVirtOnStart) {
                                                        forceActiveFocus();
                                                    }
                                                }
                                                text: typeof audioRoutingController !== "undefined" ? audioRoutingController.virtualMicName : "Caster Soundboard Mic"
                                                placeholderText: qsTr("Caster Soundboard Mic")
                                                onTextChanged: {
                                                    if (activeFocus && typeof audioRoutingController !== "undefined") {
                                                        audioRoutingController.virtualMicName = text;
                                                    }
                                                }
                                                onEditingFinished: {
                                                    if (typeof audioRoutingController !== "undefined") {
                                                        audioRoutingController.virtualMicName = text;
                                                    }
                                                }
                                            }

                                            Button {
                                                id: copyVirtBtn
                                                text: qsTr("Copy")
                                                implicitHeight: 40
                                                implicitWidth: 54
                                                onClicked: {
                                                    micNameField.selectAll();
                                                    micNameField.copy();
                                                }
                                                contentItem: Text {
                                                    text: parent.text
                                                    color: "#FFFFFF"
                                                    font.bold: true
                                                    font.pointSize: 9
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                                background: Rectangle {
                                                    color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                                    radius: 6
                                                    border.color: "#454B54"
                                                    border.width: 1
                                                }
                                            }
                                        }
                                    }

                                    // Left Pin
                                    Rectangle {
                                        id: pinVirt
                                        anchors.left: parent.left
                                        anchors.leftMargin: -5
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: (cardVirt.isRouting || cardVirt.isHoveredOrFocused) ? "#10B981" : "#22352B"
                                        border.color: "#131518"
                                        border.width: 2
                                        z: 10
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }
                                }

                                // Direct Output Card
                                Rectangle {
                                    id: cardDir
                                    Layout.fillWidth: true
                                    implicitHeight: dirCol.implicitHeight + 24
                                    color: isHoveredOrFocused ? "#251F2D" : "#1C2025"
                                    radius: 10
                                    border.color: (!isRouting || isHoveredOrFocused) ? "#A855F7" : "#362646"
                                    border.width: isHoveredOrFocused ? 2 : 1.5
                                    opacity: isRouting ? (isHoveredOrFocused ? 1.0 : 0.35) : 1.0

                                    readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                    readonly property bool isHoveredOrFocused: cardDirHover.hovered || sbOutCombo.activeFocus || sbOutCombo.popup.visible

                                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    Behavior on border.color { ColorAnimation { duration: 180 } }
                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    HoverHandler {
                                        id: cardDirHover
                                    }

                                    ColumnLayout {
                                        id: dirCol
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: "🔈 " + qsTr("Direct Playback Output")
                                                color: (!cardDir.isRouting || cardDir.isHoveredOrFocused) ? "#D8B4FE" : "#7E6E8E"
                                                font.bold: true
                                                font.pointSize: 10.5
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                            }
                                            Item { Layout.fillWidth: true }
                                            Rectangle {
                                                width: dirBadgeTxt.implicitWidth + 10
                                                height: 18
                                                radius: 4
                                                color: (!cardDir.isRouting || cardDir.isHoveredOrFocused) ? "#2C1B3B" : "#1E1825"
                                                border.color: (!cardDir.isRouting || cardDir.isHoveredOrFocused) ? "#7E22CE" : "#362646"
                                                border.width: 1
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                                Text {
                                                    id: dirBadgeTxt
                                                    anchors.centerIn: parent
                                                    text: qsTr("Speakers / DAC")
                                                    color: (!cardDir.isRouting || cardDir.isHoveredOrFocused) ? "#D8B4FE" : "#7E6E8E"
                                                    font.pointSize: 8
                                                    font.bold: true
                                                    Behavior on color { ColorAnimation { duration: 180 } }
                                                }
                                            }
                                        }

                                        Text {
                                            text: qsTr("Where soundboard plays directly when virtual routing is off:")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }

                                        DarkComboBox {
                                            id: sbOutCombo
                                            Layout.fillWidth: true
                                            enabled: typeof audioRoutingController === "undefined" || !audioRoutingController.isRouting
                                            model: (typeof projectController !== "undefined" && projectController.availableAudioOutputs.length > 0)
                                                   ? projectController.availableAudioOutputs
                                                   : [qsTr("Default System Output")]
                                            currentIndex: {
                                                if (typeof projectController === "undefined" || !projectController.outputDeviceName)
                                                    return 0;
                                                var idx = sbOutCombo.model.indexOf(projectController.outputDeviceName);
                                                return idx >= 0 ? idx : 0;
                                            }
                                            displayText: (typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting)
                                                         ? qsTr("Virtual Sink (Managed)")
                                                         : currentText
                                            onActivated: (index) => {
                                                if (typeof projectController !== "undefined") {
                                                    projectController.outputDeviceName = currentText;
                                                }
                                                if (typeof audioRoutingController !== "undefined") {
                                                    audioRoutingController.selectedSystemOutput = currentText;
                                                }
                                            }
                                        }
                                    }

                                    // Left Pin
                                    Rectangle {
                                        id: pinDir
                                        anchors.left: parent.left
                                        anchors.leftMargin: -5
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: (!cardDir.isRouting || cardDir.isHoveredOrFocused) ? "#A855F7" : "#362646"
                                        border.color: "#131518"
                                        border.width: 2
                                        z: 10
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }
                                }

                                // Headphones Monitor Output Card
                                Rectangle {
                                    id: cardMon
                                    Layout.fillWidth: true
                                    implicitHeight: monCol.implicitHeight + 24
                                    color: isHoveredOrFocused ? "#28241D" : "#1C2025"
                                    radius: 10
                                    border.color: (isRouting || isHoveredOrFocused) ? "#F59E0B" : "#362C1E"
                                    border.width: isHoveredOrFocused ? 2 : 1.5
                                    opacity: isRouting ? 1.0 : (isHoveredOrFocused ? 1.0 : 0.45)

                                    readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                    readonly property bool isHoveredOrFocused: cardMonHover.hovered || monOutCombo.activeFocus || monOutCombo.popup.visible

                                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    Behavior on border.color { ColorAnimation { duration: 180 } }
                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    HoverHandler {
                                        id: cardMonHover
                                    }

                                    ColumnLayout {
                                        id: monCol
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: "🎧 " + qsTr("Headphones Monitor Output")
                                                color: (cardMon.isRouting || cardMon.isHoveredOrFocused) ? "#FCD34D" : "#8A795D"
                                                font.bold: true
                                                font.pointSize: 10.5
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                            }
                                            Item { Layout.fillWidth: true }
                                            Rectangle {
                                                width: monBadgeTxt.implicitWidth + 10
                                                height: 18
                                                radius: 4
                                                color: (cardMon.isRouting || cardMon.isHoveredOrFocused) ? "#342211" : "#1E1A16"
                                                border.color: (cardMon.isRouting || cardMon.isHoveredOrFocused) ? "#B45309" : "#362C1E"
                                                border.width: 1
                                                Behavior on color { ColorAnimation { duration: 180 } }
                                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                                Text {
                                                    id: monBadgeTxt
                                                    anchors.centerIn: parent
                                                    text: qsTr("YOU Hear")
                                                    color: (cardMon.isRouting || cardMon.isHoveredOrFocused) ? "#FCD34D" : "#8A795D"
                                                    font.pointSize: 8
                                                    font.bold: true
                                                    Behavior on color { ColorAnimation { duration: 180 } }
                                                }
                                            }
                                        }

                                        Text {
                                            text: qsTr("Plays soundboard clips into your ears. Your live voice is excluded.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                        }

                                        DarkComboBox {
                                            id: monOutCombo
                                            Layout.fillWidth: true
                                            model: (typeof audioRoutingController !== "undefined" && audioRoutingController.outputDevices.length > 0)
                                                   ? audioRoutingController.outputDevices
                                                   : [qsTr("Default System Output")]
                                            currentIndex: {
                                                if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedMonitorOutput)
                                                    return 0;
                                                var idx = monOutCombo.model.indexOf(audioRoutingController.selectedMonitorOutput);
                                                return idx >= 0 ? idx : 0;
                                            }
                                            onActivated: (index) => {
                                                if (typeof audioRoutingController !== "undefined") {
                                                    audioRoutingController.selectedMonitorOutput = currentText;
                                                }
                                            }
                                        }
                                    }

                                    // Left Pin
                                    Rectangle {
                                        id: pinMon
                                        anchors.left: parent.left
                                        anchors.leftMargin: -5
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: (cardMon.isRouting || cardMon.isHoveredOrFocused) ? "#F59E0B" : "#362C1E"
                                        border.color: "#131518"
                                        border.width: 2
                                        z: 10
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }
                                }
                            }

                            // --- ZONE 2: 90° ORTHOGONAL WIRING CANVAS ---
                            Canvas {
                                id: wireCanvas
                                anchors.fill: parent
                                antialiasing: true
                                z: 3

                                property real dashOffset: 0
                                NumberAnimation on dashOffset {
                                    from: 28; to: 0; duration: 900
                                    loops: Animation.Infinite; running: true
                                }
                                onDashOffsetChanged: requestPaint()

                                function drawArrow(ctx, x, y, angle, color) {
                                    ctx.save();
                                    ctx.translate(x, y);
                                    ctx.rotate(angle);
                                    ctx.fillStyle = color;
                                    ctx.beginPath();
                                    ctx.moveTo(-7, -4);
                                    ctx.lineTo(0, 0);
                                    ctx.lineTo(-7, 4);
                                    ctx.closePath();
                                    ctx.fill();
                                    ctx.restore();
                                }

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);

                                    if (!pinMic || !pinSb || !pinVirt || !pinDir || !pinMon || width <= 0 || height <= 0)
                                        return;

                                    var isRouting = typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting;

                                    // Pin coordinates mapped to canvas space
                                    var pMic = flowGrid.mapFromItem(pinMic, pinMic.width/2, pinMic.height/2);
                                    var pSb = flowGrid.mapFromItem(pinSb, pinSb.width/2, pinSb.height/2);
                                    var pVirt = flowGrid.mapFromItem(pinVirt, pinVirt.width/2, pinVirt.height/2);
                                    var pDir = flowGrid.mapFromItem(pinDir, pinDir.width/2, pinDir.height/2);
                                    var pMon = flowGrid.mapFromItem(pinMon, pinMon.width/2, pinMon.height/2);
    

                                    var corLeft = leftCol.width;
                                    var corRight = flowGrid.width - rightCol.width;
                                    var corMidX = (corLeft + corRight) / 2;
                                    var lane1X = corLeft + 24;
                                    var lane2X = corRight - 24;
                                    var mixerRadius = 22;
                                    var mixerCenterY = pVirt.y;

                                    // Position badges in corridor
                                    mixerBadge.x = corMidX - mixerBadge.width / 2;
                                    mixerBadge.y = mixerCenterY - mixerBadge.height / 2;
                                    directBadge.x = corMidX - directBadge.width / 2;
                                    directBadge.y = pDir.y - directBadge.height / 2;

                                    if (isRouting) {
                                        // 1. Blue Wire: Mic Pin -> Straight horizontal into Mixer Left
                                        ctx.save();
                                        ctx.strokeStyle = "#3B82F6";
                                        ctx.lineWidth = 2.5;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        ctx.moveTo(pMic.x, mixerCenterY);
                                        ctx.lineTo(corMidX - mixerRadius - 2, mixerCenterY);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, corMidX - mixerRadius - 2, mixerCenterY, 0, "#3B82F6");

                                        // 2. Purple Soundboard Wire: Pin -> lane1X
                                        ctx.save();
                                        ctx.strokeStyle = "#A855F7";
                                        ctx.lineWidth = 2.5;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        ctx.moveTo(pSb.x, pSb.y);
                                        ctx.lineTo(lane1X, pSb.y);
                                        ctx.stroke();
                                        ctx.restore();

                                        // Junction dot at (lane1X, pSb.y)
                                        ctx.save();
                                        ctx.fillStyle = "#A855F7";
                                        ctx.strokeStyle = "#131518";
                                        ctx.lineWidth = 1.5;
                                        ctx.beginPath();
                                        ctx.arc(lane1X, pSb.y, 3.5, 0, 2 * Math.PI);
                                        ctx.fill();
                                        ctx.stroke();
                                        ctx.restore();

                                        // 2A. Purple Branch: Junction -> 90° UP -> 90° RIGHT -> 90° UP into Mixer Bottom
                                        ctx.save();
                                        ctx.strokeStyle = "#A855F7";
                                        ctx.lineWidth = 2.2;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        ctx.moveTo(lane1X, pSb.y);
                                        ctx.lineTo(lane1X, mixerCenterY + 28);
                                        ctx.lineTo(corMidX, mixerCenterY + 28);
                                        ctx.lineTo(corMidX, mixerCenterY + mixerRadius + 2);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, corMidX, mixerCenterY + mixerRadius + 2, -Math.PI / 2, "#A855F7");

                                        // 2B. Amber Monitor Wire: Junction -> 90° RIGHT across to Headphones
                                        ctx.save();
                                        ctx.strokeStyle = "#F59E0B";
                                        ctx.lineWidth = 2.5;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        if (Math.abs(pSb.y - pMon.y) < 3) {
                                            ctx.moveTo(lane1X, pSb.y);
                                            ctx.lineTo(pMon.x - 2, pMon.y);
                                        } else {
                                            ctx.moveTo(lane1X, pSb.y);
                                            ctx.lineTo(lane2X, pSb.y);
                                            ctx.lineTo(lane2X, pMon.y);
                                            ctx.lineTo(pMon.x - 2, pMon.y);
                                        }
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pMon.x - 2, pMon.y, 0, "#F59E0B");

                                        // 3. Green Virtual Mic Wire: Mixer Right -> Straight horizontal to Virtual Mic Pin
                                        ctx.save();
                                        ctx.strokeStyle = "#10B981";
                                        ctx.lineWidth = 2.8;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        ctx.moveTo(corMidX + mixerRadius + 2, mixerCenterY);
                                        ctx.lineTo(pVirt.x - 2, pVirt.y);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pVirt.x - 2, pVirt.y, 0, "#10B981");

                                        // Inactive Direct wire (Lights up on hover/focus!)
                                        var dirLit = cardDir.isHoveredOrFocused || directBadge.isHovered;
                                        ctx.save();
                                        ctx.strokeStyle = "#A855F7";
                                        ctx.globalAlpha = dirLit ? 0.8 : 0.12;
                                        ctx.lineWidth = dirLit ? 2.5 : 2;
                                        if (dirLit) {
                                            ctx.setLineDash([7, 7]);
                                            ctx.lineDashOffset = wireCanvas.dashOffset;
                                        }
                                        ctx.beginPath();
                                        ctx.moveTo(pSb.x, pSb.y);
                                        ctx.lineTo(lane1X, pSb.y);
                                        ctx.lineTo(lane1X, pDir.y);
                                        ctx.lineTo(pDir.x - 2, pDir.y);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pDir.x - 2, pDir.y, 0, dirLit ? "rgba(168, 85, 247, 0.8)" : "rgba(168, 85, 247, 0.12)");

                                    } else {
                                        // DIRECT MODE
                                        // 1. Purple Soundboard Wire: Pin -> lane1X -> 90° UP to pDir.y -> 90° RIGHT to Direct Pin
                                        ctx.save();
                                        ctx.strokeStyle = "#A855F7";
                                        ctx.lineWidth = 2.8;
                                        ctx.setLineDash([7, 7]);
                                        ctx.lineDashOffset = wireCanvas.dashOffset;
                                        ctx.beginPath();
                                        ctx.moveTo(pSb.x, pSb.y);
                                        ctx.lineTo(lane1X, pSb.y);
                                        ctx.lineTo(lane1X, pDir.y);
                                        ctx.lineTo(pDir.x - 2, pDir.y);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pDir.x - 2, pDir.y, 0, "#A855F7");

                                        // Inactive Dimmed Wires (Light up when their respective card or mixer badge is hovered or focused!)
                                        var micLit = cardMic.isHoveredOrFocused || mixerBadge.isHovered;
                                        var virtLit = cardVirt.isHoveredOrFocused || mixerBadge.isHovered;
                                        var monLit = cardMon.isHoveredOrFocused;

                                        // Mic -> Mixer
                                        ctx.save();
                                        ctx.strokeStyle = "#3B82F6";
                                        ctx.globalAlpha = micLit ? 0.8 : 0.12;
                                        ctx.lineWidth = micLit ? 2.5 : 2;
                                        if (micLit) {
                                            ctx.setLineDash([7, 7]);
                                            ctx.lineDashOffset = wireCanvas.dashOffset;
                                        }
                                        ctx.beginPath();
                                        ctx.moveTo(pMic.x, mixerCenterY);
                                        ctx.lineTo(corMidX - mixerRadius - 2, mixerCenterY);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, corMidX - mixerRadius - 2, mixerCenterY, 0, micLit ? "rgba(59, 130, 246, 0.8)" : "rgba(59, 130, 246, 0.12)");

                                        // Mixer -> Virtual Mic
                                        ctx.save();
                                        ctx.strokeStyle = "#10B981";
                                        ctx.globalAlpha = virtLit ? 0.8 : 0.12;
                                        ctx.lineWidth = virtLit ? 2.5 : 2;
                                        if (virtLit) {
                                            ctx.setLineDash([7, 7]);
                                            ctx.lineDashOffset = wireCanvas.dashOffset;
                                        }
                                        ctx.beginPath();
                                        ctx.moveTo(corMidX + mixerRadius + 2, mixerCenterY);
                                        ctx.lineTo(pVirt.x - 2, pVirt.y);
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pVirt.x - 2, pVirt.y, 0, virtLit ? "rgba(16, 185, 129, 0.8)" : "rgba(16, 185, 129, 0.12)");

                                        // Monitor
                                        ctx.save();
                                        ctx.strokeStyle = "#F59E0B";
                                        ctx.globalAlpha = monLit ? 0.8 : 0.12;
                                        ctx.lineWidth = monLit ? 2.5 : 2;
                                        if (monLit) {
                                            ctx.setLineDash([7, 7]);
                                            ctx.lineDashOffset = wireCanvas.dashOffset;
                                        }
                                        ctx.beginPath();
                                        if (Math.abs(pSb.y - pMon.y) < 3) {
                                            ctx.moveTo(lane1X, pSb.y);
                                            ctx.lineTo(pMon.x - 2, pMon.y);
                                        } else {
                                            ctx.moveTo(lane1X, pSb.y);
                                            ctx.lineTo(lane2X, pSb.y);
                                            ctx.lineTo(lane2X, pMon.y);
                                            ctx.lineTo(pMon.x - 2, pMon.y);
                                        }
                                        ctx.stroke();
                                        ctx.restore();
                                        drawArrow(ctx, pMon.x - 2, pMon.y, 0, monLit ? "rgba(245, 158, 11, 0.8)" : "rgba(245, 158, 11, 0.12)");
                                    }
                                }
                            }

                            // Mixer Badge ⊕ (Center of corridor)
                            Rectangle {
                                id: mixerBadge
                                width: 44
                                height: 44
                                radius: 22
                                color: isHovered ? "#1F382B" : "#16241D"
                                border.color: (isRouting || isHovered) ? "#10B981" : "#333942"
                                border.width: isHovered ? 2.5 : 2
                                opacity: isRouting ? 1.0 : (isHovered ? 1.0 : 0.35)
                                z: 5

                                readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                readonly property bool isHovered: mixerHover.hovered

                                Behavior on opacity { NumberAnimation { duration: 180 } }
                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                Behavior on color { ColorAnimation { duration: 180 } }

                                HoverHandler {
                                    id: mixerHover
                                }

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 0
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: "⊕"
                                        color: (mixerBadge.isRouting || mixerBadge.isHovered) ? "#34D399" : "#6EE7B7"
                                        font.bold: true
                                        font.pixelSize: 16
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: "MIX"
                                        color: (mixerBadge.isRouting || mixerBadge.isHovered) ? "#FFFFFF" : "#94A3B8"
                                        font.bold: true
                                        font.pixelSize: 8
                                    }
                                }
                            }

                            // Direct Badge (Center of corridor)
                            Rectangle {
                                id: directBadge
                                width: 62
                                height: 22
                                radius: 11
                                color: isHovered ? "#361E47" : "#241A2D"
                                border.color: (!isRouting || isHovered) ? "#A855F7" : "#4A2868"
                                border.width: isHovered ? 2 : 1
                                opacity: isRouting ? (isHovered ? 1.0 : 0.2) : 1.0
                                z: 5

                                readonly property bool isRouting: typeof audioRoutingController !== "undefined" && audioRoutingController.isRouting
                                readonly property bool isHovered: directHover.hovered

                                Behavior on opacity { NumberAnimation { duration: 180 } }
                                Behavior on border.color { ColorAnimation { duration: 180 } }
                                Behavior on color { ColorAnimation { duration: 180 } }

                                HoverHandler {
                                    id: directHover
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "Direct ►"
                                    color: (!directBadge.isRouting || directBadge.isHovered) ? "#D8B4FE" : "#8B65AA"
                                    font.bold: true
                                    font.pointSize: 8.5
                                    font.family: "Monospace"
                                }
                            }
                        }

                        // Directional Legend Row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 6
                                color: "#181C21"
                                border.color: "#2B313A"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Rectangle { width: 12; height: 3; radius: 1.5; color: "#3B82F6" }
                                    Text {
                                        text: qsTr("Voice Signal (Mic → Mixer ⊕)")
                                        color: "#93C5FD"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 6
                                color: "#181C21"
                                border.color: "#2B313A"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Rectangle { width: 12; height: 3; radius: 1.5; color: "#A855F7" }
                                    Text {
                                        text: qsTr("Sound FX (Clips → Output / Mix)")
                                        color: "#D8B4FE"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 6
                                color: "#181C21"
                                border.color: "#2B313A"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Rectangle { width: 12; height: 3; radius: 1.5; color: "#10B981" }
                                    Text {
                                        text: qsTr("Virtual Mic (Merged → Apps)")
                                        color: "#6EE7B7"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 6
                                color: "#181C21"
                                border.color: "#2B313A"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Rectangle { width: 12; height: 3; radius: 1.5; color: "#F59E0B" }
                                    Text {
                                        text: qsTr("Monitor (Clean SFX → Headphones)")
                                        color: "#FCD34D"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }
                        }
                    }
                }

                // 4. Discord & Voice Chat Configuration Guide Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: discordGuideCol.implicitHeight + 36
                    color: "#1E2235"
                    radius: 8
                    border.color: "#5865F2"
                    border.width: 1

                    ColumnLayout {
                        id: discordGuideCol
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            spacing: 10
                            Rectangle {
                                width: 28
                                height: 28
                                radius: 14
                                color: "#5865F2"
                                Text {
                                    anchors.centerIn: parent
                                    text: "!"
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 13
                                }
                            }
                            ColumnLayout {
                                spacing: 2
                                Text {
                                    text: qsTr("Discord & Voice Applications Setup (Required for Voice Chat)")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 13
                                }
                                Text {
                                    text: qsTr("If friends or stream viewers in Discord cannot hear your soundboard audio, check these 3 Discord settings:")
                                    color: "#A2ADC7"
                                    font.pointSize: 9.5
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2E3654"
                        }

                        ColumnLayout {
                            spacing: 10
                            Layout.fillWidth: true
                            Layout.leftMargin: 4

                            RowLayout {
                                spacing: 10
                                Layout.fillWidth: true
                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#353D66"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "1"
                                        color: "#8DA0F0"
                                        font.bold: true
                                        font.pointSize: 9.5
                                    }
                                }
                                Text {
                                    text: qsTr("<b>Select Input Device:</b> In Discord <i>Settings → Voice & Video</i>, change <b>Input Device</b> to <b>\"" + (typeof audioRoutingController !== "undefined" ? audioRoutingController.virtualMicName : "Caster Soundboard Mic") + "\"</b> (do not leave on 'Default').")
                                    color: "#E2E8F0"
                                    font.pointSize: 9.5
                                    textFormat: Text.RichText
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Layout.fillWidth: true
                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#5C2626"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "2"
                                        color: "#FF8585"
                                        font.bold: true
                                        font.pointSize: 9.5
                                    }
                                }
                                Text {
                                    text: qsTr("<b>Turn OFF Noise Suppression (Krisp) [CRITICAL]:</b> In Discord <i>Voice & Video → Noise Suppression</i>, set to <b>None / Off</b>. Discord's Krisp AI classifies music and sound effects as background noise and completely mutes them!")
                                    color: "#FFA0A0"
                                    font.bold: true
                                    font.pointSize: 9.5
                                    textFormat: Text.RichText
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }

                            RowLayout {
                                spacing: 10
                                Layout.fillWidth: true
                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#353D66"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "3"
                                        color: "#8DA0F0"
                                        font.bold: true
                                        font.pointSize: 9.5
                                    }
                                }
                                Text {
                                    text: qsTr("<b>Disable Echo Cancellation & Advanced Voice Activity:</b> In Discord <i>Voice & Video</i>, turn off <i>Echo Cancellation</i> and <i>Advanced Voice Activity</i> so soundboard playback does not get truncated.")
                                    color: "#E2E8F0"
                                    font.pointSize: 9.5
                                    textFormat: Text.RichText
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                // 5. Advanced & Troubleshooting Accordion (Collapsed by Default)
                Rectangle {
                    id: advancedAccordion
                    Layout.fillWidth: true
                    property bool isExpanded: (typeof openAccordionOnStart !== "undefined" && openAccordionOnStart) ? true : false
                    implicitHeight: isExpanded ? advInnerCol.implicitHeight : 52
                    Layout.preferredHeight: implicitHeight
                    color: "#222528"
                    radius: 8
                    border.color: isExpanded ? "#4E545F" : "#33373B"
                    border.width: 1
                    clip: true

                    Behavior on implicitHeight {
                        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
                    }

                    ColumnLayout {
                        id: advInnerCol
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        spacing: 0

                        // Clickable Accordion Header
                        MouseArea {
                            id: advHeaderMouse
                            Layout.fillWidth: true
                            Layout.preferredHeight: 52
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: advancedAccordion.isExpanded = !advancedAccordion.isExpanded

                            Rectangle {
                                anchors.fill: parent
                                color: advHeaderMouse.containsMouse ? "#2A2E33" : "transparent"
                                radius: 8
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 18
                                anchors.rightMargin: 18
                                spacing: 12

                                Text {
                                    text: advancedAccordion.isExpanded ? "▼" : "▶"
                                    color: "#A855F7"
                                    font.pixelSize: 13
                                    font.bold: true
                                }

                                Text {
                                    text: qsTr("Advanced & Troubleshooting")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 12
                                }

                                Text {
                                    text: qsTr("Mixing options, device recovery & diagnostics")
                                    color: "#888888"
                                    font.pointSize: 9
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Rectangle {
                                    width: expandBadgeText.implicitWidth + 18
                                    height: 26
                                    radius: 13
                                    color: advHeaderMouse.containsMouse ? "#3E434D" : "#2E333B"
                                    border.color: "#454B54"
                                    border.width: 1

                                    Text {
                                        id: expandBadgeText
                                        anchors.centerIn: parent
                                        text: advancedAccordion.isExpanded ? qsTr("Hide") : qsTr("Show")
                                        color: "#D8B4FE"
                                        font.bold: true
                                        font.pointSize: 8.5
                                    }
                                }
                            }
                        }

                        // Collapsible Body
                        ColumnLayout {
                            id: advContentCol
                            Layout.fillWidth: true
                            Layout.leftMargin: 18
                            Layout.rightMargin: 18
                            Layout.bottomMargin: 18
                            Layout.topMargin: 4
                            spacing: 18
                            opacity: advancedAccordion.isExpanded ? 1.0 : 0.0
                            Behavior on opacity {
                                NumberAnimation { duration: 180 }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: "#33373B"
                            }

                            // --- SUBSECTION 1: Application Routing & Mixing Options ---
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Text {
                                    text: qsTr("Application Routing & Mixing Overrides")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }

                                // Checkbox 1: Mix physical mic with soundboard
                                RowLayout {
                                    spacing: 12
                                    Layout.fillWidth: true
                                    DarkCheckBox {
                                        id: mixCheck
                                        checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.mixMicrophone : true
                                        onToggled: {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.mixMicrophone = checked;
                                            }
                                        }
                                    }
                                    ColumnLayout {
                                        spacing: 2
                                        Layout.fillWidth: true
                                        TapHandler {
                                            cursorShape: Qt.PointingHandCursor
                                            onTapped: mixCheck.toggle()
                                        }
                                        Text {
                                            text: qsTr("Send microphone + soundboard to virtual microphone (Summing Mixer ⊕)")
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Combines your physical voice microphone and soundboard audio clips into one virtual input for voice chat or recording.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }

                                // Checkbox 2: Monitor soundboard through chosen output
                                RowLayout {
                                    spacing: 12
                                    Layout.fillWidth: true
                                    DarkCheckBox {
                                        id: monCheck
                                        checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.monitorSoundboard : true
                                        onToggled: {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.monitorSoundboard = checked;
                                            }
                                        }
                                    }
                                    ColumnLayout {
                                        spacing: 2
                                        Layout.fillWidth: true
                                        TapHandler {
                                            cursorShape: Qt.PointingHandCursor
                                            onTapped: monCheck.toggle()
                                        }
                                        Text {
                                            text: qsTr("Monitor soundboard through headphones (never plays your live microphone back)")
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Plays soundboard audio through your monitor output without echoing your live voice back into your ears.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }

                                // Checkbox 3: Set as system default input
                                RowLayout {
                                    spacing: 12
                                    Layout.fillWidth: true
                                    DarkCheckBox {
                                        id: defInputCheck
                                        checked: typeof audioRoutingController !== "undefined" ? audioRoutingController.useAsDefaultInput : false
                                        onToggled: {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.useAsDefaultInput = checked;
                                            }
                                        }
                                    }
                                    ColumnLayout {
                                        spacing: 2
                                        Layout.fillWidth: true
                                        TapHandler {
                                            cursorShape: Qt.PointingHandCursor
                                            onTapped: defInputCheck.toggle()
                                        }
                                        Text {
                                            text: qsTr("Use mixed virtual microphone as system default input (global)")
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9.5
                                        }
                                        Text {
                                            text: qsTr("Sets the virtual microphone as the system-wide default input for applications that use 'System Default'.")
                                            color: "#888888"
                                            font.pointSize: 8.5
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: "#33373B"
                            }

                            // --- SUBSECTION 2: Device Recovery & Troubleshooting ---
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Text {
                                    text: qsTr("Device Recovery & Service Restart")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }

                                Text {
                                    text: qsTr("Cycle hardware profiles if a USB audio interface falls asleep, or restart Linux user audio services if PipeWire hangs.")
                                    color: "#888888"
                                    font.pointSize: 8.5
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    DarkComboBox {
                                        id: recCombo
                                        Layout.fillWidth: true
                                        model: (typeof audioRoutingController !== "undefined" && audioRoutingController.recoveryDevices.length > 0)
                                               ? audioRoutingController.recoveryDevices
                                               : [qsTr("Choose a device to enable or reset...")]
                                        currentIndex: {
                                            if (typeof audioRoutingController === "undefined" || !audioRoutingController.selectedRecoveryDevice)
                                                return 0;
                                            var idx = recCombo.model.indexOf(audioRoutingController.selectedRecoveryDevice);
                                            return idx >= 0 ? idx : 0;
                                        }
                                        onActivated: (index) => {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.selectedRecoveryDevice = currentText;
                                            }
                                        }
                                    }

                                    Button {
                                        text: qsTr("Recover Device…")
                                        Layout.preferredHeight: 40
                                        onClicked: {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.recoverSelectedDevice();
                                            }
                                        }
                                        contentItem: Text {
                                            text: parent.text
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                            radius: 6
                                            border.color: "#454B54"
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        text: qsTr("Restart Desktop Audio…")
                                        Layout.preferredHeight: 40
                                        onClicked: {
                                            if (typeof audioRoutingController !== "undefined") {
                                                audioRoutingController.restartDesktopAudio();
                                            }
                                        }
                                        contentItem: Text {
                                            text: parent.text
                                            color: "#FFFFFF"
                                            font.bold: true
                                            font.pointSize: 9
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: parent.down ? "#4E545F" : (parent.hovered ? "#3E434D" : "#2E333B")
                                            radius: 6
                                            border.color: "#454B54"
                                            border.width: 1
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: "#33373B"
                            }

                            // --- SUBSECTION 3: Diagnostics Log ---
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Text {
                                    text: qsTr("Diagnostics Log")
                                    color: "#FFFFFF"
                                    font.bold: true
                                    font.pointSize: 11
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 120
                                    color: "#181A1B"
                                    radius: 6
                                    border.color: "#33373B"
                                    border.width: 1

                                    ScrollView {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        clip: true

                                        TextArea {
                                            id: diagText
                                            readOnly: true
                                            color: "#74B9FF"
                                            font.family: "Monospace"
                                            font.pointSize: 8.5
                                            wrapMode: Text.WrapAnywhere
                                            text: typeof audioRoutingController !== "undefined" ? audioRoutingController.diagnosticsLog : qsTr("Diagnostics ready.")
                                            background: null
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
        }
    }
}
