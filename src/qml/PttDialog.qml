import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    implicitWidth: 580
    implicitHeight: 640
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

    onClosed: {
        root.isCapturingKey = false;
        root.saveCurrentProfile();
    }

    function selectProfileForTab(tabName) {
        if (!projectController || !projectController.pttController) return;
        var list = projectController.pttController.profilesList();
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
            var pData = projectController.pttController.getProfile(unpinnedIdx);
            pData["pinnedTabName"] = tabName;
            projectController.pttController.setProfile(unpinnedIdx, pData);
            root.selectedProfileIndex = unpinnedIdx;
        } else {
            var createdIdx = projectController.pttController.addProfile(tabName + " PTT");
            var createdData = projectController.pttController.getProfile(createdIdx);
            createdData["pinnedTabName"] = tabName;
            projectController.pttController.setProfile(createdIdx, createdData);
            root.selectedProfileIndex = createdIdx;
        }
        refreshProfileUI();
    }

    function refreshProfileUI() {
        if (!projectController || !projectController.pttController) return;
        var list = projectController.pttController.profilesList();
        if (list.length === 0) {
            projectController.pttController.addProfile(qsTr("Default PTT"));
            list = projectController.pttController.profilesList();
        }
        if (selectedProfileIndex >= list.length) {
            selectedProfileIndex = Math.max(0, list.length - 1);
        }
        profileCombo.currentIndex = selectedProfileIndex;
        var p = list[selectedProfileIndex];
        if (p) {
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
        if (!projectController || !projectController.pttController) return;
        var list = projectController.pttController.profilesList();
        if (selectedProfileIndex < 0 || selectedProfileIndex >= list.length) return;

        var pinned = tabCombo.currentIndex === 0 ? "" : tabCombo.currentText;
        var modeVal = modePttRadio.checked ? 1 : (modeMuteRadio.checked ? 2 : 0);

        var data = {
            "mode": modeVal,
            "pinnedTabName": pinned,
            "bufferMs": Math.round(bufferSlider.value)
        };
        projectController.pttController.setProfile(selectedProfileIndex, data);
    }

    contentItem: ColumnLayout {
        spacing: 12

        // Header Title & Close
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("🎙️ Push-to-Talk (PTT) & Mic Automation")
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
            text: qsTr("Automatically keys up Push-To-Talk or toggles Mute in Discord or games when sounds play. Configured per soundboard tab.")
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
                text: qsTr("PTT Profile:")
                color: "#CCCCCC"
                font.bold: true
                font.pointSize: 10
            }

            ComboBox {
                id: profileCombo
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                model: (projectController && projectController.pttController) ? projectController.pttController.profiles : []
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
                    leftPadding: 10
                    rightPadding: 24
                    elide: Text.ElideRight
                }
                delegate: ItemDelegate {
                    width: profileCombo.width
                    implicitHeight: 34
                    contentItem: Text {
                        text: (model && model.name !== undefined) ? model.name : ((modelData && modelData.name !== undefined) ? modelData.name : "")
                        color: highlighted ? "#FFFFFF" : "#CCCCCC"
                        font.pointSize: 9.5
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    background: Rectangle {
                        color: highlighted ? "#8E2DE2" : (hovered ? "#2D3238" : "transparent")
                        radius: 4
                    }
                }
                popup: Popup {
                    y: profileCombo.height + 2
                    width: profileCombo.width
                    implicitHeight: Math.min(240, contentItem.implicitHeight + 10)
                    padding: 4
                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: profileCombo.popup.visible ? profileCombo.delegateModel : null
                        currentIndex: profileCombo.highlightedIndex
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                    background: Rectangle {
                        color: "#222528"
                        radius: 6
                        border.color: "#3A3E42"
                        border.width: 1
                    }
                }
            }

            Button {
                id: deleteProfileBtn
                text: qsTr("Delete")
                enabled: profileCombo.count > 1
                Layout.preferredHeight: 34
                onClicked: {
                    projectController.pttController.removeProfile(root.selectedProfileIndex);
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

            Button {
                id: newProfileBtn
                text: qsTr("+ New PTT Profile")
                Layout.preferredHeight: 34
                onClicked: {
                    root.saveCurrentProfile();
                    newProfileDialog.openForNew();
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
        }

        // Configuration Card
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1A1C1F"
            radius: 8
            border.color: "#2E3236"
            border.width: 1
            clip: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                // Pinned Tab
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("Pin to Tab:")
                        color: "#CCCCCC"
                        font.pointSize: 10
                        Layout.preferredWidth: 90
                    }

                    ComboBox {
                        id: tabCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 34
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
                            leftPadding: 10
                            rightPadding: 24
                            elide: Text.ElideRight
                        }
                        delegate: ItemDelegate {
                            width: tabCombo.width
                            implicitHeight: 34
                            contentItem: Text {
                                text: modelData || ""
                                color: highlighted ? "#FFFFFF" : "#CCCCCC"
                                font.pointSize: 9.5
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                            background: Rectangle {
                                color: highlighted ? "#8E2DE2" : (hovered ? "#2D3238" : "transparent")
                                radius: 4
                            }
                        }
                        popup: Popup {
                            y: tabCombo.height + 2
                            width: tabCombo.width
                            implicitHeight: Math.min(240, contentItem.implicitHeight + 10)
                            padding: 4
                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: tabCombo.popup.visible ? tabCombo.delegateModel : null
                                currentIndex: tabCombo.highlightedIndex
                                ScrollIndicator.vertical: ScrollIndicator { }
                            }
                            background: Rectangle {
                                color: "#222528"
                                radius: 6
                                border.color: "#3A3E42"
                                border.width: 1
                            }
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
                        Layout.preferredWidth: 90
                    }

                    ButtonGroup { id: modeGroup }

                    RadioButton {
                        id: modePttRadio
                        text: qsTr("Push-To-Talk (Hold Key)")
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
                        text: qsTr("Mic Mute Toggle")
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
                        text: qsTr("Disabled")
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
                        Layout.preferredHeight: 36
                        focus: root.isCapturingKey

                        property var currentProf: {
                            if (projectController && projectController.pttController && root.selectedProfileIndex >= 0 && root.selectedProfileIndex < profileCombo.count) {
                                var rev = projectController.pttController.revision;
                                return projectController.pttController.getProfile(root.selectedProfileIndex);
                            }
                            return null;
                        }

                        property string keyDisplay: {
                            if (root.isCapturingKey) return qsTr("Press any key…");
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
                            if (root.isCapturingKey && projectController && projectController.pttController) {
                                var codeName = projectController.pttController.keyNameFromEvent(event.key, event.nativeScanCode);
                                if (codeName && codeName.length > 0) {
                                    var data = {};
                                    if (modePttRadio.checked) data["pttKey"] = codeName;
                                    else data["muteKey"] = codeName;
                                    projectController.pttController.setProfile(root.selectedProfileIndex, data);
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
                        property bool isTesting: (typeof projectController !== "undefined" && projectController.pttController)
                            ? projectController.pttController.isTransmittingKey(currentKeyName)
                            : false

                        Layout.preferredHeight: 36
                        Layout.preferredWidth: 130
                        enabled: !root.isCapturingKey && (projectController && projectController.pttController && projectController.pttController.uinputAvailable)

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
                                text: testKeyBtn.isTesting ? "🟢" : "🎙️"
                                font.pixelSize: 11
                            }
                            Text {
                                text: testKeyBtn.isTesting ? qsTr("Holding…") : qsTr("Test Key (2s)")
                                color: testKeyBtn.isTesting ? "#A7F3D0" : (testKeyBtn.enabled ? "#FFFFFF" : "#666666")
                                font.bold: true
                                font.pointSize: 9
                            }
                        }

                        onClicked: {
                            if (projectController && projectController.pttController) {
                                projectController.pttController.testTriggerKey(currentKeyName, 2000);
                            }
                        }

                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Hold %1 down for 2 seconds to verify detection in Discord or your game").arg(currentKeyName)
                    }
                }

                Text {
                    visible: !modeNoneRadio.checked
                    text: qsTr("Tip: Click 'Test Key (2s)' and switch to Discord or your game to verify keypress detection.")
                    color: "#7E8590"
                    font.pointSize: 8
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }

                Item { Layout.fillHeight: true }

                // uinput System Status Notice
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    radius: 6
                    color: (projectController && projectController.pttController && projectController.pttController.uinputAvailable) ? "#16281E" : "#281E16"
                    border.color: (projectController && projectController.pttController && projectController.pttController.uinputAvailable) ? "#2ECC71" : "#F39C12"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Text {
                            text: (projectController && projectController.pttController && projectController.pttController.uinputAvailable) ? "✔" : "⚠"
                            color: (projectController && projectController.pttController && projectController.pttController.uinputAvailable) ? "#2ECC71" : "#F39C12"
                            font.bold: true
                        }

                        Text {
                            text: (projectController && projectController.pttController && projectController.pttController.uinputAvailable)
                                  ? qsTr("Linux uinput virtual keyboard ready for Push-to-Talk.")
                                  : qsTr("uinput device not accessible. Key injection requires user access to /dev/uinput.")
                            color: (projectController && projectController.pttController && projectController.pttController.uinputAvailable) ? "#2ECC71" : "#F39C12"
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

            Text {
                text: qsTr("💾 Saved to active project • Save project (Ctrl+S) to persist to disk")
                color: "#7E8590"
                font.pointSize: 8.5
                Layout.alignment: Qt.AlignVCenter
            }

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

    // New Profile Modal Name Prompt Dialog
    Dialog {
        id: newProfileDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        implicitWidth: 420
        topPadding: 18
        bottomPadding: 18
        leftPadding: 20
        rightPadding: 20

        background: Rectangle {
            color: "#222528"
            radius: 8
            border.color: "#3A3E42"
            border.width: 1
        }

        function openForNew() {
            var defaultNum = (projectController && projectController.pttController)
                             ? (projectController.pttController.count + 1) : 1;
            newProfileNameInput.text = qsTr("PTT Profile %1").arg(defaultNum);
            newProfileDialog.open();
            newProfileNameInput.selectAll();
            newProfileNameInput.forceActiveFocus();
        }

        function confirmCreate() {
            var trimmed = newProfileNameInput.text.trim();
            if (trimmed.length === 0) return;
            if (projectController && projectController.pttController) {
                var newIdx = projectController.pttController.addProfile(trimmed);
                root.selectedProfileIndex = newIdx;
                root.refreshProfileUI();
            }
            newProfileDialog.close();
        }

        contentItem: ColumnLayout {
            spacing: 14

            Text {
                text: qsTr("Create New PTT Profile")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 12
            }

            Text {
                text: qsTr("Enter a name for the new profile:")
                color: "#CCCCCC"
                font.pointSize: 9.5
            }

            TextField {
                id: newProfileNameInput
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                color: "#FFFFFF"
                font.pointSize: 10
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                selectByMouse: true
                background: Rectangle {
                    color: "#282C32"
                    radius: 6
                    border.color: newProfileNameInput.activeFocus ? "#8E2DE2" : "#3A3E42"
                    border.width: 1
                }
                Keys.onReturnPressed: newProfileDialog.confirmCreate()
                Keys.onEnterPressed: newProfileDialog.confirmCreate()
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Cancel")
                    Layout.preferredHeight: 34
                    Layout.preferredWidth: 80
                    onClicked: newProfileDialog.close()
                    background: Rectangle {
                        color: parent.hovered ? "#353A42" : "#282C32"
                        radius: 6
                        border.color: "#3A3E42"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#CCCCCC"
                        font.pointSize: 9
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: qsTr("Create")
                    enabled: newProfileNameInput.text.trim().length > 0
                    Layout.preferredHeight: 34
                    Layout.preferredWidth: 90
                    onClicked: newProfileDialog.confirmCreate()
                    background: Rectangle {
                        color: parent.enabled ? (parent.down ? "#7324B8" : (parent.hovered ? "#A855F7" : "#8E2DE2")) : "#222528"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: parent.enabled ? "#FFFFFF" : "#666666"
                        font.bold: true
                        font.pointSize: 9.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
