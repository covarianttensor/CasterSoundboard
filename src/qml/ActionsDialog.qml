import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    implicitWidth: 540
    implicitHeight: 620
    topPadding: 16
    bottomPadding: 16
    leftPadding: 20
    rightPadding: 20

    property int selectedProfileIndex: 0
    property bool isCapturingKey: false
    property string activeTargetKey: "ptt" // "ptt" or "mute"
    property string targetTabOnOpen: ""

    background: Rectangle {
        color: "#222528"
        radius: 8
        border.color: "#3A3E42"
        border.width: 1
    }

    function openForTab(tabName) {
        targetTabOnOpen = tabName;
        root.open();
    }

    onOpened: {
        root.isCapturingKey = false;
        if (targetTabOnOpen.length > 0) {
            selectProfileForTab(targetTabOnOpen);
            targetTabOnOpen = "";
        } else {
            root.selectedProfileIndex = 0;
            refreshProfileUI();
        }
    }

    function selectProfileForTab(tabName) {
        if (!projectController || !projectController.actionsController) return;
        var list = projectController.actionsController.profilesList();
        for (var i = 0; i < list.length; ++i) {
            if (list[i].pinnedTabName === tabName) {
                root.selectedProfileIndex = i;
                refreshProfileUI();
                return;
            }
        }
        // If not found, assign unpinned or create new
        var unpinnedIdx = -1;
        for (var j = 0; j < list.length; ++j) {
            if (!list[j].pinnedTabName || list[j].pinnedTabName.length === 0) {
                unpinnedIdx = j;
                break;
            }
        }
        if (unpinnedIdx !== -1) {
            var pData = projectController.actionsController.getProfile(unpinnedIdx);
            pData["pinnedTabName"] = tabName;
            projectController.actionsController.setProfile(unpinnedIdx, pData);
            root.selectedProfileIndex = unpinnedIdx;
        } else {
            var createdIdx = projectController.actionsController.addProfile(tabName + " PTT");
            var createdData = projectController.actionsController.getProfile(createdIdx);
            createdData["pinnedTabName"] = tabName;
            projectController.actionsController.setProfile(createdIdx, createdData);
            root.selectedProfileIndex = createdIdx;
        }
        refreshProfileUI();
    }

    function refreshProfileUI() {
        if (!projectController || !projectController.actionsController) return;
        var list = projectController.actionsController.profilesList();
        if (list.length === 0) {
            projectController.actionsController.addProfile(qsTr("Default PTT"));
            list = projectController.actionsController.profilesList();
        }
        if (selectedProfileIndex >= list.length) {
            selectedProfileIndex = Math.max(0, list.length - 1);
        }
        var p = list[selectedProfileIndex];
        if (p) {
            nameInput.text = p.name || "";
            bufferSlider.value = p.bufferMs || 150;
            if (p.mode === 1) modePttRadio.checked = true;
            else if (p.mode === 2) modeMuteRadio.checked = true;
            else modeNoneRadio.checked = true;
            updateTabCombo(p.pinnedTabName || "");
        }
    }

    function updateTabCombo(pinnedName) {
        var items = [qsTr("None (Unpinned)")];
        var selectIdx = 0;
        if (projectController && projectController.tabModel) {
            for (var i = 0; i < projectController.tabModel.count; ++i) {
                var tName = projectController.tabModel.tabNameAt(i);
                items.push(tName);
                if (tName === pinnedName) {
                    selectIdx = i + 1;
                }
            }
        }
        tabCombo.model = items;
        tabCombo.currentIndex = selectIdx;
    }

    function saveCurrentProfile() {
        if (!projectController || !projectController.actionsController) return;
        var list = projectController.actionsController.profilesList();
        if (selectedProfileIndex < 0 || selectedProfileIndex >= list.length) return;

        var pinned = tabCombo.currentIndex === 0 ? "" : tabCombo.currentText;
        var modeVal = modePttRadio.checked ? 1 : (modeMuteRadio.checked ? 2 : 0);

        var data = {
            "name": nameInput.text.trim(),
            "mode": modeVal,
            "pinnedTabName": pinned,
            "bufferMs": Math.round(bufferSlider.value)
        };
        projectController.actionsController.setProfile(selectedProfileIndex, data);
    }

    contentItem: ColumnLayout {
        spacing: 12

        // Header Title & Close
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("⚡ Soundboard Actions (PTT & Mute)")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 13
                Layout.fillWidth: true
            }

            Rectangle {
                width: 28
                height: 28
                radius: 14
                color: closeHover.hovered ? "#4E545F" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "#CCCCCC"
                    font.pixelSize: 13
                    font.bold: true
                }

                HoverHandler { id: closeHover }
                TapHandler { onTapped: root.close() }
            }
        }

        Text {
            text: qsTr("Automatically keys up Push-To-Talk or toggles Mute in games/Discord when sounds play. Actions are pinned per soundboard tab.")
            color: "#AAAAAA"
            font.pointSize: 9
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#33373B"
        }

        // Profile Selection Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: qsTr("Profile:")
                color: "#CCCCCC"
                font.bold: true
                font.pointSize: 10
            }

            ComboBox {
                id: profileCombo
                Layout.fillWidth: true
                model: projectController && projectController.actionsController ? projectController.actionsController.profilesList() : []
                textRole: "name"
                currentIndex: root.selectedProfileIndex
                onActivated: (index) => {
                    root.saveCurrentProfile();
                    root.selectedProfileIndex = index;
                    root.refreshProfileUI();
                }

                background: Rectangle {
                    color: "#282C32"
                    radius: 6
                    border.color: profileCombo.activeFocus ? "#8E2DE2" : "#454B54"
                    border.width: 1
                }
                contentItem: Text {
                    text: profileCombo.displayText
                    color: "#FFFFFF"
                    font.pointSize: 10
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                }
            }

            Button {
                text: qsTr("+ New")
                Layout.preferredHeight: 34
                onClicked: {
                    root.saveCurrentProfile();
                    var newIdx = projectController.actionsController.addProfile(qsTr("Action Profile %1").arg(profileCombo.count + 1));
                    root.selectedProfileIndex = newIdx;
                    root.refreshProfileUI();
                }
                background: Rectangle {
                    color: parent.down ? "#7324B8" : (parent.hovered ? "#A855F7" : "#8E2DE2")
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 9
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: qsTr("Delete")
                enabled: profileCombo.count > 1
                Layout.preferredHeight: 34
                onClicked: {
                    projectController.actionsController.removeProfile(root.selectedProfileIndex);
                    root.selectedProfileIndex = Math.max(0, root.selectedProfileIndex - 1);
                    root.refreshProfileUI();
                }
                background: Rectangle {
                    color: parent.enabled ? (parent.down ? "#A93226" : (parent.hovered ? "#C0392B" : "#2E333B")) : "#222528"
                    radius: 6
                    border.color: parent.enabled ? "#E74C3C" : "#33373B"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? (parent.hovered ? "#FFFFFF" : "#E74C3C") : "#666666"
                    font.bold: true
                    font.pointSize: 9
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        // Configuration Card
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1A1C1F"
            radius: 8
            border.color: "#2E3236"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                // Profile Name
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("Name:")
                        color: "#CCCCCC"
                        font.pointSize: 10
                        Layout.preferredWidth: 100
                    }

                    TextField {
                        id: nameInput
                        Layout.fillWidth: true
                        color: "#FFFFFF"
                        selectByMouse: true
                        background: Rectangle {
                            color: "#282C32"
                            radius: 6
                            border.color: nameInput.activeFocus ? "#8E2DE2" : "#3A3E42"
                            border.width: 1
                        }
                        onEditingFinished: root.saveCurrentProfile()
                    }
                }

                // Pinned Tab
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("Pin to Tab:")
                        color: "#CCCCCC"
                        font.pointSize: 10
                        Layout.preferredWidth: 100
                    }

                    ComboBox {
                        id: tabCombo
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: "#282C32"
                            radius: 6
                            border.color: tabCombo.activeFocus ? "#8E2DE2" : "#3A3E42"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: tabCombo.displayText
                            color: "#FFFFFF"
                            font.pointSize: 10
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                        }
                        onActivated: root.saveCurrentProfile()
                    }
                }

                // Talk Mode Radios
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("Talk Mode:")
                        color: "#CCCCCC"
                        font.pointSize: 10
                        Layout.preferredWidth: 100
                    }

                    ButtonGroup { id: modeGroup }

                    RadioButton {
                        id: modePttRadio
                        text: qsTr("Push-To-Talk")
                        ButtonGroup.group: modeGroup
                        contentItem: Text {
                            text: parent.text
                            color: parent.checked ? "#FFFFFF" : "#AAAAAA"
                            font.bold: parent.checked
                            leftPadding: parent.indicator.width + 4
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: root.saveCurrentProfile()
                    }

                    RadioButton {
                        id: modeMuteRadio
                        text: qsTr("Mute Toggle")
                        ButtonGroup.group: modeGroup
                        contentItem: Text {
                            text: parent.text
                            color: parent.checked ? "#FFFFFF" : "#AAAAAA"
                            font.bold: parent.checked
                            leftPadding: parent.indicator.width + 4
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: root.saveCurrentProfile()
                    }

                    RadioButton {
                        id: modeNoneRadio
                        text: qsTr("None")
                        ButtonGroup.group: modeGroup
                        contentItem: Text {
                            text: parent.text
                            color: parent.checked ? "#FFFFFF" : "#AAAAAA"
                            font.bold: parent.checked
                            leftPadding: parent.indicator.width + 4
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: root.saveCurrentProfile()
                    }
                }

                // Key Binding Capture Box & Test Button
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: !modeNoneRadio.checked

                    Text {
                        text: modePttRadio.checked ? qsTr("PTT Key:") : qsTr("Mute Key:")
                        color: "#CCCCCC"
                        font.pointSize: 10
                        Layout.preferredWidth: 90
                    }

                    Button {
                        id: keyCaptureBtn
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        focus: root.isCapturingKey

                        property var currentProf: (projectController && projectController.actionsController && root.selectedProfileIndex >= 0 && root.selectedProfileIndex < profileCombo.count)
                                                  ? projectController.actionsController.getProfile(root.selectedProfileIndex) : null

                        property string keyDisplay: {
                            if (root.isCapturingKey) return qsTr("Press any key on keyboard…");
                            if (!currentProf) return "KEY_V";
                            return modePttRadio.checked ? (currentProf.pttKey || "KEY_V") : (currentProf.muteKey || "KEY_M");
                        }

                        background: Rectangle {
                            color: root.isCapturingKey ? "#332244" : (keyCaptureBtn.hovered ? "#353A42" : "#282C32")
                            radius: 6
                            border.color: root.isCapturingKey ? "#8E2DE2" : "#3A3E42"
                            border.width: root.isCapturingKey ? 2 : 1
                        }

                        contentItem: RowLayout {
                            anchors.centerIn: parent
                            spacing: 8
                            Text {
                                text: "⌨"
                                font.pixelSize: 14
                                color: root.isCapturingKey ? "#8E2DE2" : "#AAAAAA"
                            }
                            Text {
                                text: keyCaptureBtn.keyDisplay
                                color: root.isCapturingKey ? "#F1C40F" : "#FFFFFF"
                                font.bold: true
                                font.pointSize: 10
                            }
                        }

                        onClicked: {
                            root.isCapturingKey = true;
                            keyCaptureBtn.forceActiveFocus();
                        }

                        Keys.onPressed: (event) => {
                            if (root.isCapturingKey && projectController && projectController.actionsController) {
                                var codeName = projectController.actionsController.keyNameFromEvent(event.key, event.nativeScanCode);
                                if (codeName && codeName.length > 0) {
                                    var data = {};
                                    if (modePttRadio.checked) data["pttKey"] = codeName;
                                    else data["muteKey"] = codeName;
                                    projectController.actionsController.setProfile(root.selectedProfileIndex, data);
                                    root.isCapturingKey = false;
                                    event.accepted = true;
                                    root.refreshProfileUI();
                                }
                            }
                        }
                    }

                    // Test Keypress Button
                    Button {
                        id: testKeyBtn
                        readonly property string currentKeyName: {
                            if (!keyCaptureBtn.currentProf) return "KEY_V";
                            return modePttRadio.checked ? (keyCaptureBtn.currentProf.pttKey || "KEY_V") : (keyCaptureBtn.currentProf.muteKey || "KEY_M");
                        }
                        property bool isTesting: (typeof projectController !== "undefined" && projectController.actionsController)
                            ? projectController.actionsController.isTransmittingKey(currentKeyName)
                            : false

                        Layout.preferredHeight: 38
                        Layout.preferredWidth: 140
                        enabled: !root.isCapturingKey && (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable)

                        background: Rectangle {
                            color: testKeyBtn.isTesting ? "#064E3B" : (testKeyBtn.pressed ? "#1E2022" : (testKeyBtn.hovered ? "#353A42" : "#282C32"))
                            radius: 6
                            border.color: testKeyBtn.isTesting ? "#10B981" : (testKeyBtn.hovered ? "#4E545F" : "#3A3E42")
                            border.width: testKeyBtn.isTesting ? 2 : 1
                        }

                        contentItem: RowLayout {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: testKeyBtn.isTesting ? "🟢" : "⚡"
                                font.pixelSize: 11
                            }
                            Text {
                                text: testKeyBtn.isTesting ? qsTr("Holding Key…") : qsTr("Test Key (2s)")
                                color: testKeyBtn.isTesting ? "#A7F3D0" : (testKeyBtn.enabled ? "#FFFFFF" : "#666666")
                                font.bold: true
                                font.pointSize: 9
                            }
                        }

                        onClicked: {
                            if (projectController && projectController.actionsController) {
                                projectController.actionsController.testTriggerKey(currentKeyName, 2000);
                            }
                        }

                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Hold %1 down for 2 seconds to verify detection in Discord or your game").arg(currentKeyName)
                    }
                }

                Text {
                    visible: !modeNoneRadio.checked
                    text: qsTr("Tip: Click 'Test Key (2s)' and switch to Discord or your game to test keypress detection.")
                    color: "#7E8590"
                    font.pointSize: 8
                    Layout.leftMargin: 98
                }

                // Buffer Slider
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: !modeNoneRadio.checked

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Audio Delay Buffer:")
                            color: "#CCCCCC"
                            font.pointSize: 10
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: Math.round(bufferSlider.value) + " ms"
                            color: "#8E2DE2"
                            font.bold: true
                            font.pointSize: 10
                        }
                    }

                    Slider {
                        id: bufferSlider
                        Layout.fillWidth: true
                        from: 0
                        to: 500
                        stepSize: 25
                        value: 150
                        onMoved: root.saveCurrentProfile()
                    }

                    Text {
                        text: qsTr("Lead-in delay before sound starts and tail delay before key release to prevent voice clipping.")
                        color: "#7E8590"
                        font.pointSize: 8
                    }
                }

                Item { Layout.fillHeight: true }

                // uinput System Status Notice
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    radius: 6
                    color: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable) ? "#16281E" : "#281E16"
                    border.color: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable) ? "#2ECC71" : "#F39C12"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Text {
                            text: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable) ? "✔" : "⚠"
                            color: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable) ? "#2ECC71" : "#F39C12"
                            font.bold: true
                        }

                        Text {
                            text: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable)
                                  ? qsTr("Linux uinput virtual keyboard ready for in-game PTT.")
                                  : qsTr("uinput device not accessible. Key injection requires user access to /dev/uinput.")
                            color: (projectController && projectController.actionsController && projectController.actionsController.uinputAvailable) ? "#2ECC71" : "#F39C12"
                            font.pointSize: 8
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        // Bottom Action Row
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Done")
                Layout.preferredHeight: 36
                Layout.preferredWidth: 100
                onClicked: {
                    root.saveCurrentProfile();
                    root.close();
                }
                background: Rectangle {
                    color: parent.down ? "#7324B8" : (parent.hovered ? "#A855F7" : "#8E2DE2")
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    font.bold: true
                    font.pointSize: 10
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
