import QtQuick

Rectangle {
    id: root

    property int row: 0
    property int col: 0
    property int index: row * 9 + col

    property int value: game.board[index] || 0
    property bool isInitial: game.initialClues[index] || false
    property var cellNotes: game.notes[index] || []

    property bool isSelected: game.selectedRow === row && game.selectedCol === col
    property bool isMatchingNumber: game.highlightNum > 0 && value === game.highlightNum
    property bool isInSameRowOrColOrBox: {
        if (game.selectedRow < 0 || game.selectedCol < 0) return false;
        if (isSelected) return false;
        if (game.selectedRow === row) return true;
        if (game.selectedCol === col) return true;
        var myBox = Math.floor(row / 3) * 3 + Math.floor(col / 3);
        var selBox = Math.floor(game.selectedRow / 3) * 3 + Math.floor(game.selectedCol / 3);
        return myBox === selBox;
    }

    property bool isFlashingWrong: false
    property bool isFlashingCorrect: false
    property bool isWaveCelebrating: false

    radius: 4

    // Background coloring based on state
    color: {
        if (isFlashingWrong) return Qt.rgba(theme.red.r, theme.red.g, theme.red.b, 0.4);
        if (isFlashingCorrect || isWaveCelebrating) return Qt.rgba(theme.green.r, theme.green.g, theme.green.b, 0.45);
        if (isSelected) return Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.3);
        if (isMatchingNumber) return Qt.rgba(theme.green.r, theme.green.g, theme.green.b, 0.25);
        if (isInSameRowOrColOrBox) return Qt.rgba(theme.selection.r, theme.selection.g, theme.selection.b, 0.35);
        if (mouseArea.containsMouse) return theme.lighterBackground;
        return theme.darkBackground;
    }

    border.width: isSelected ? 2 : (isMatchingNumber ? 1 : 0)
    border.color: {
        if (isFlashingWrong) return theme.red;
        if (isSelected) return theme.accent;
        if (isMatchingNumber) return theme.green;
        return "transparent";
    }

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on border.color { ColorAnimation { duration: 120 } }

    // Big Number display
    Text {
        id: numberText
        visible: root.value !== 0
        anchors.centerIn: parent
        text: root.value !== 0 ? root.value : ""
        font.pixelSize: Math.floor(root.height * 0.58)
        font.bold: true
        font.family: "Sans-Serif"
        color: {
            if (root.isInitial) return theme.foreground;
            return theme.accent;
        }

        scale: 1.0
        Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutBack } }
    }

    // 3x3 Notes Grid
    Grid {
        id: notesGrid
        visible: root.value === 0 && root.cellNotes.length > 0
        anchors.fill: parent
        anchors.margins: 4
        columns: 3
        rows: 3

        Repeater {
            model: 9
            Item {
                width: (notesGrid.width) / 3
                height: (notesGrid.height) / 3
                property int noteNum: index + 1
                property bool hasNote: root.cellNotes.indexOf(noteNum) !== -1

                Text {
                    anchors.centerIn: parent
                    text: parent.hasNote ? parent.noteNum : ""
                    font.pixelSize: Math.max(9, Math.floor(root.height * 0.22))
                    font.bold: false
                    color: parent.noteNum === game.highlightNum ? theme.green : theme.lightForeground
                }
            }
        }
    }

    // Shake animation for errors
    SequentialAnimation {
        id: shakeAnim
        PropertyAnimation { target: root; property: "x"; to: root.x - 5; duration: 40 }
        PropertyAnimation { target: root; property: "x"; to: root.x + 5; duration: 40 }
        PropertyAnimation { target: root; property: "x"; to: root.x - 4; duration: 40 }
        PropertyAnimation { target: root; property: "x"; to: root.x + 4; duration: 40 }
        PropertyAnimation { target: root; property: "x"; to: root.x; duration: 40 }
    }

    // Zoom pulse on correct entry
    SequentialAnimation {
        id: popAnim
        PropertyAnimation { target: numberText; property: "scale"; to: 1.35; duration: 120; easing.type: Easing.OutCubic }
        PropertyAnimation { target: numberText; property: "scale"; to: 1.0; duration: 150; easing.type: Easing.OutBack }
    }

    // Wave celebrate animation
    SequentialAnimation {
        id: waveAnim
        PropertyAction { target: root; property: "isWaveCelebrating"; value: true }
        PropertyAnimation { target: numberText; property: "scale"; to: 1.25; duration: 150 }
        PropertyAnimation { target: numberText; property: "scale"; to: 1.0; duration: 180 }
        PropertyAction { target: root; property: "isWaveCelebrating"; value: false }
    }

    Timer {
        id: flashWrongTimer
        interval: 350
        onTriggered: root.isFlashingWrong = false
    }

    Timer {
        id: flashCorrectTimer
        interval: 300
        onTriggered: root.isFlashingCorrect = false
    }

    Connections {
        target: game

        function onCellFlash(flashRow, flashCol, isCorrect) {
            if (flashRow === root.row && flashCol === root.col) {
                if (isCorrect) {
                    root.isFlashingCorrect = true;
                    flashCorrectTimer.restart();
                    popAnim.restart();
                } else {
                    root.isFlashingWrong = true;
                    flashWrongTimer.restart();
                    shakeAnim.restart();
                }
            }
        }

        function onRowCompleted(completedRow) {
            if (completedRow === root.row) {
                // staggered wave
                var delay = root.col * 40;
                waveTimer.interval = delay;
                waveTimer.restart();
            }
        }

        function onColCompleted(completedCol) {
            if (completedCol === root.col) {
                var delay = root.row * 40;
                waveTimer.interval = delay;
                waveTimer.restart();
            }
        }

        function onBoxCompleted(completedBox) {
            var myBox = Math.floor(root.row / 3) * 3 + Math.floor(root.col / 3);
            if (completedBox === myBox) {
                var boxOffset = (root.row % 3) * 3 + (root.col % 3);
                waveTimer.interval = boxOffset * 40;
                waveTimer.restart();
            }
        }
    }

    Timer {
        id: waveTimer
        onTriggered: waveAnim.restart()
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: game.selectCell(root.row, root.col)
    }
}
