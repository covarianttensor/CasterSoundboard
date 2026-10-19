# QML ComboBox Dropdowns & Desktop Theme Isolation

This document explains the root cause of blank/disappearing items in custom QML `ComboBox` controls under KDE/Breeze environments, why platform styling plugins cause breakage, and the robust architecture adopted by **Caster Soundboard**.

---

## 1. Problem Description

When running Caster Soundboard under the KDE Plasma desktop (or inside a Flatpak container using `org.kde.Platform`), users observed:
* ComboBox popups opening with blank/empty list entries.
* Initially only a single row (or highlighted item) rendering, while other items remained completely blank.
* In some cases, single-item lists opening with a massive 280px tall empty dark container.
* Warning log messages in console output:
  ```text
  QQuickItem::stackAfter: Cannot stack ItemDelegate... after ..., which must be a sibling
  ```

---

## 2. Root Cause Analysis

### A. Desktop Theme Hijacking (`org.kde.desktop` / `org.kde.union`)
Qt Quick Controls 2 (QQC2) is designed to adopt the host operating system's visual style. On KDE systems and in Flatpak runtimes based on `org.kde.Platform`, QQC2 automatically defaults to the `org.kde.desktop` plugin (Breeze style).

When a QML file imports `QtQuick.Controls` and declares:
```qml
delegate: ItemDelegate { ... }
```
The QML engine does not instantiate an abstract, unstyled item. Instead, the `org.kde.desktop` plugin injects its own implementation (`/usr/lib/qml/org/kde/union/ItemDelegate.qml`), which brings in:
* `Union.Element` and `Union.StyledRectangle`
* `Union.Positioner.positionItems: [contentItem]`
* `contentItem: P.DefaultContentItem { control: control }`

KDE's internal positioner expects specific standard properties (like `control.text` on the delegate itself). When custom dark-themed QML code replaces `contentItem: Text { ... }` and `background: Rectangle { ... }` without conforming to KDE's private styling contract, the positioner fails to lay out children, positions items at `(0, 0)`, and paints unhighlighted text as invisible or black-on-black.

### B. `ComboBox.delegateModel` Object Lifecycle Conflict
In early iterations, the ComboBox popup was structured as:
```qml
// ANTI-PATTERN: DO NOT USE
popup: Popup {
    contentItem: ListView {
        model: dcb.delegateModel
        currentIndex: dcb.highlightedIndex
    }
}
```
`QQuickComboBox` uses `delegateModel` internally to create and manage the visual representation of the active item in the ComboBox button. Binding a custom `ListView` directly to `dcb.delegateModel` creates a race condition where the combo box steals delegates from the list view pool. This causes list items to be instantiated as uninitialized dummy `QQuickItem` instances with 0 children, causing missing text and blank gaps.

---

## 3. Implemented Solution

Two complementary guardrails resolve this permanently across all environments:

### Guardrail 1: Enforce Cross-Platform `Basic` Style Default
In [`src/main.cpp`](../../src/main.cpp), before initializing the QML engine:
```cpp
#include <QQuickStyle>

// Enforce cross-platform "Basic" style so desktop style plugins (e.g. KDE org.kde.desktop / breeze)
// do not override custom dark QML delegates, popups, and components.
if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle("Basic");
}
```
Setting `Basic` style ensures Qt Quick Controls provides clean, predictable foundational sizing and behavior without desktop style plugins injecting native KStyle or Breeze hooks into custom QML widgets.

### Guardrail 2: Theme-Agnostic Pure QML Delegates & Direct `model` Binding
All ComboBox components in the codebase now bypass `ItemDelegate` and `delegateModel` entirely:
1. Bind the `ListView` directly to `combo.model` (not `combo.delegateModel`).
2. Use a self-contained `Rectangle` with `HoverHandler`, `TapHandler`, and child `Text`.
3. Compute popup height dynamically using `comboList.contentHeight`.

```qml
component DarkComboBox : ComboBox {
    id: dcb
    implicitHeight: 40

    contentItem: Text {
        leftPadding: 10
        rightPadding: 28
        text: dcb.displayText
        font.pointSize: 9.5
        color: dcb.enabled ? "#FFFFFF" : "#888888"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    popup: Popup {
        id: comboPopup
        y: dcb.height + 2
        width: dcb.width
        implicitHeight: Math.min(280, comboList.contentHeight + 10)
        padding: 4

        contentItem: ListView {
            id: comboList
            clip: true
            implicitHeight: contentHeight
            model: dcb.model
            currentIndex: dcb.currentIndex

            delegate: Rectangle {
                id: itemDel
                required property var modelData
                required property int index

                readonly property bool isCurrent: dcb.currentIndex === index
                readonly property bool isHighlighted: dcb.highlightedIndex === index || (dcb.highlightedIndex < 0 && isCurrent)
                readonly property bool isHovered: itemHover.hovered

                width: ListView.view ? ListView.view.width : dcb.width
                implicitHeight: 36
                radius: 4
                color: isHighlighted ? "#8E2DE2" : (isHovered ? "#2D3238" : "transparent")
                Behavior on color { ColorAnimation { duration: 100 } }

                HoverHandler {
                    id: itemHover
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    cursorShape: Qt.PointingHandCursor
                    onTapped: {
                        dcb.currentIndex = index;
                        dcb.popup.close();
                        dcb.activated(index);
                    }
                }

                Text {
                    anchors.fill: parent
                    leftPadding: 8
                    rightPadding: 8
                    text: (dcb.textRole && modelData && modelData[dcb.textRole] !== undefined)
                          ? modelData[dcb.textRole]
                          : ((typeof modelData === "string") ? modelData : (modelData && modelData.name !== undefined ? modelData.name : ""))
                    color: (itemDel.isHighlighted || itemDel.isHovered) ? "#FFFFFF" : "#CCCCCC"
                    font.pointSize: 9.5
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
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
```

---

## 4. Key Rules for Agents Writing QML Controls

1. **Never use `delegateModel` on a custom `ListView` inside a `ComboBox` popup**: Always bind to `model: combo.model`.
2. **Never rely on `ItemDelegate` for custom-styled lists**: Prefer `Rectangle` + `HoverHandler` + `TapHandler`.
3. **Always set `implicitHeight: Math.min(MAX_HEIGHT, list.contentHeight + PADDING)`**: This ensures 1-item lists size tightly to ~46px while multi-item lists scroll cleanly up to the maximum height.
4. **Preserve `dcb.activated(index)`**: When the user taps a row, always call `combo.currentIndex = index; combo.popup.close(); combo.activated(index);` to maintain signal consistency with standard QML ComboBox event handlers.
