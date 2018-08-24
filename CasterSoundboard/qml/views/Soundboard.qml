import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Flickable {
    id: soundboardFlickable
    anchors.fill: parent
    anchors.margins: 8
    contentHeight: soundboardGrid.height
    //contentWidth: soundboardGrid.width
    clip: true
    flickableDirection: Flickable.VerticalFlick
    ScrollBar.vertical: ScrollBar {
                id: soundboardScrollbar
                width: 30
                active: true
                onActiveChanged: {
                                if (!active)
                                    active = true;//Keep scrollbar always visible
                            }
            }

    onWidthChanged: soundboardGrid.columns = soundboardGrid.computeNeededColumns(200, soundboardFlickable.width, soundboardGrid.spacing, soundboardScrollbar.width)

    Grid {
            id: soundboardGrid
            spacing: 8
            columns: soundboardGrid.computeNeededColumns(200, parent.width, soundboardGrid.spacing, soundboardScrollbar.width)

            function computeNeededColumns(playerWidth, containerWidth, gridSpacing, scrollbarWidth){
                var neededColumnsWithoutSpacing = Math.floor(containerWidth / playerWidth);
                var adjustedGridWidth = containerWidth - ((neededColumnsWithoutSpacing - 1) * gridSpacing) - (gridSpacing + scrollbarWidth);
                var neededColumns = Math.floor(adjustedGridWidth / playerWidth);
                neededColumns = ((neededColumns <= 0) ? 1 : neededColumns);
                return neededColumns;
            }

            Repeater {
                model: 32
                Player {}
            }
    }
}
