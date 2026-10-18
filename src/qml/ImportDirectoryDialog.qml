import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    implicitWidth: 600
    topPadding: 16
    bottomPadding: 16
    leftPadding: 20
    rightPadding: 20

    property string selectedPath: ""
    property int playableFilesCount: 0

    background: Rectangle {
        color: "#222528"
        radius: 8
        border.color: "#3A3E42"
        border.width: 1
    }

    function openDialog(initialPath) {
        if (initialPath && initialPath.length > 0) {
            setDirectory(initialPath);
        } else {
            selectedPath = "";
            folderPathInput.text = "";
            tabNameInput.text = "";
            playableFilesCount = 0;
        }
        root.open();
    }

    function setDirectory(path) {
        // Strip trailing slashes
        var clean = path.replace(/\/+$/, "");
        selectedPath = clean;
        folderPathInput.text = clean;

        // Auto-populate tab name from folder name
        var lastSlash = clean.lastIndexOf('/');
        var folderName = lastSlash >= 0 ? clean.substring(lastSlash + 1) : clean;
        if (folderName.length > 0) {
            tabNameInput.text = folderName;
        }
        updateFileCount();
    }

    function updateFileCount() {
        if (!selectedPath || selectedPath.length === 0 || typeof projectController === "undefined" || !projectController.tabModel) {
            playableFilesCount = 0;
            return;
        }
        playableFilesCount = projectController.tabModel.countPlayableFilesInDirectory(selectedPath, recursiveCheck.checked);
    }

    function confirmImport() {
        if (playableFilesCount <= 0 || typeof projectController === "undefined" || !projectController.tabModel) return;

        var orderMode = orderCombo.currentIndex; // 0=RootFilesFirst, 1=RelativePathOrder
        // Naming combo indices: 0=Disambiguate duplicates, 1=Filename only, 2=Prefix folder name
        var namingScheme = 2; // Default DisambiguateDuplicates
        if (namingCombo.currentIndex === 1) namingScheme = 0; // FilenameOnly
        else if (namingCombo.currentIndex === 2) namingScheme = 1; // FolderPrefix

        var customName = tabNameInput.text.trim();
        projectController.tabModel.importDirectory(selectedPath, recursiveCheck.checked, customName, orderMode, namingScheme);
        root.close();
    }

    FolderDialog {
        id: folderPicker
        title: qsTr("Select Directory to Import as Soundboard")
        onAccepted: {
            var path = selectedFolder.toString();
            if (path.indexOf("file://") === 0) {
                path = path.substring(7);
            }
            root.setDirectory(path);
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        // --- Header Title & Close Button ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                width: 32
                height: 32
                radius: 16
                color: "#2C2340"
                border.color: "#8E2DE2"
                border.width: 1

                Image {
                    anchors.centerIn: parent
                    source: "icons/import_directory.png"
                    sourceSize.width: 16
                    sourceSize.height: 16
                }
            }

            Text {
                text: qsTr("Import Directory as Soundboard")
                color: "#FFFFFF"
                font.bold: true
                font.pointSize: 12.5
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

        // --- Subtitle Description ---
        Text {
            text: qsTr("Discover all supported audio files in a folder and create a new soundboard.")
            color: "#A0A6B2"
            font.pointSize: 9
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#33373B"
        }

        // --- Directory Path Field & Browse ---
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: qsTr("Folder Path:")
                color: "#CCCCCC"
                font.bold: true
                font.pointSize: 9.5
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                TextField {
                    id: folderPathInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: qsTr("Choose or enter a folder path...")
                    placeholderTextColor: "#666B74"
                    color: "#FFFFFF"
                    font.pointSize: 9.5
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 10
                    selectByMouse: true
                    background: Rectangle {
                        color: "#282C32"
                        radius: 6
                        border.color: folderPathInput.activeFocus ? "#8E2DE2" : "#3A3E42"
                        border.width: 1
                    }
                    onEditingFinished: {
                        if (text.trim().length > 0 && text.trim() !== root.selectedPath) {
                            root.setDirectory(text.trim());
                        }
                    }
                }

                Button {
                    text: qsTr("Browse...")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 90
                    onClicked: folderPicker.open()
                    background: Rectangle {
                        color: parent.hovered ? "#3E444E" : "#2E333B"
                        radius: 6
                        border.color: parent.hovered ? "#8E2DE2" : "#3A3E42"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.bold: true
                        font.pointSize: 9.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        // --- Soundboard Tab Name ---
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: qsTr("Soundboard Tab Name:")
                color: "#CCCCCC"
                font.bold: true
                font.pointSize: 9.5
            }

            TextField {
                id: tabNameInput
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                placeholderText: qsTr("Soundboard name...")
                placeholderTextColor: "#666B74"
                color: "#FFFFFF"
                font.pointSize: 9.5
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 10
                selectByMouse: true
                background: Rectangle {
                    color: "#282C32"
                    radius: 6
                    border.color: tabNameInput.activeFocus ? "#8E2DE2" : "#3A3E42"
                    border.width: 1
                }
            }
        }

        // --- Recursive Option ---
        CheckBox {
            id: recursiveCheck
            checked: true
            Layout.fillWidth: true
            text: qsTr("Include subdirectories (recursive scan)")
            contentItem: Text {
                text: recursiveCheck.text
                color: "#E2E8F0"
                font.pointSize: 9.5
                verticalAlignment: Text.AlignVCenter
                leftPadding: recursiveCheck.indicator.width + 8
            }
            onCheckedChanged: root.updateFileCount()
        }

        // --- Subdirectory Behavior Configuration Box ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: subConfigCol.implicitHeight + 20
            color: recursiveCheck.checked ? "#1F2226" : "#1B1C1E"
            radius: 6
            border.color: recursiveCheck.checked ? "#33373B" : "#282A2E"
            border.width: 1

            ColumnLayout {
                id: subConfigCol
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                // Ordering Option
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    opacity: recursiveCheck.checked ? 1.0 : 0.4

                    Text {
                        text: qsTr("File Ordering:")
                        color: "#CCCCCC"
                        font.pointSize: 9
                        Layout.preferredWidth: 120
                    }

                    ComboBox {
                        id: orderCombo
                        enabled: recursiveCheck.checked
                        Layout.fillWidth: true
                        Layout.preferredHeight: 32
                        model: [
                            qsTr("Parent folder first, then subdirectories"),
                            qsTr("Alphabetical by full relative path")
                        ]
                        currentIndex: 0
                        background: Rectangle {
                            color: "#282C32"
                            radius: 5
                            border.color: orderCombo.activeFocus ? "#8E2DE2" : "#3A3E42"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: orderCombo.displayText
                            color: recursiveCheck.checked ? "#FFFFFF" : "#777777"
                            font.pointSize: 9
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 20
                            elide: Text.ElideRight
                        }
                    }
                }

                // Naming Scheme Option
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    opacity: recursiveCheck.checked ? 1.0 : 0.4

                    Text {
                        text: qsTr("Tile Naming:")
                        color: "#CCCCCC"
                        font.pointSize: 9
                        Layout.preferredWidth: 120
                    }

                    ComboBox {
                        id: namingCombo
                        enabled: recursiveCheck.checked
                        Layout.fillWidth: true
                        Layout.preferredHeight: 32
                        model: [
                            qsTr("Disambiguate duplicates only (e.g. 'Subfolder / sound')"),
                            qsTr("Filename only (e.g. 'sound')"),
                            qsTr("Always prefix folder name (e.g. 'Subfolder / sound')")
                        ]
                        currentIndex: 0
                        background: Rectangle {
                            color: "#282C32"
                            radius: 5
                            border.color: namingCombo.activeFocus ? "#8E2DE2" : "#3A3E42"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: namingCombo.displayText
                            color: recursiveCheck.checked ? "#FFFFFF" : "#777777"
                            font.pointSize: 9
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 20
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        // --- File Counter Badge ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            radius: 6
            color: {
                if (root.playableFilesCount > 0) return "#143022";
                if (root.selectedPath.length > 0) return "#332211";
                return "#222528";
            }
            border.color: {
                if (root.playableFilesCount > 0) return "#2ECC71";
                if (root.selectedPath.length > 0) return "#E67E22";
                return "#3A3E42";
            }
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    text: {
                        if (root.playableFilesCount > 0) return "✓";
                        if (root.selectedPath.length > 0) return "⚠";
                        return "📁";
                    }
                    color: {
                        if (root.playableFilesCount > 0) return "#2ECC71";
                        if (root.selectedPath.length > 0) return "#E67E22";
                        return "#777777";
                    }
                    font.bold: true
                }

                Text {
                    text: {
                        if (root.playableFilesCount > 0)
                            return qsTr("%1 playable media files found ready to import.").arg(root.playableFilesCount);
                        if (root.selectedPath.length > 0)
                            return qsTr("No supported audio files found in selected directory.");
                        return qsTr("Select a folder to scan for supported audio files.");
                    }
                    color: {
                        if (root.playableFilesCount > 0) return "#2ECC71";
                        if (root.selectedPath.length > 0) return "#E67E22";
                        return "#888888";
                    }
                    font.pointSize: 9
                    font.bold: root.playableFilesCount > 0
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }

        // --- Action Buttons ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Cancel")
                Layout.preferredHeight: 36
                Layout.preferredWidth: 85
                onClicked: root.close()
                background: Rectangle {
                    color: parent.hovered ? "#353A42" : "#282C32"
                    radius: 6
                    border.color: "#3A3E42"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: "#CCCCCC"
                    font.pointSize: 9.5
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: qsTr("Import Soundboard")
                enabled: root.playableFilesCount > 0 && tabNameInput.text.trim().length > 0
                Layout.preferredHeight: 36
                Layout.preferredWidth: 150
                onClicked: root.confirmImport()
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
