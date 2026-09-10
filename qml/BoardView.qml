import QtQuick
import QtQuick.Layouts

Item {
    id: root
    focus: true

    signal returnToMenu()

    // Center board square
    Rectangle {
        id: boardContainer
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, parent.height - 32)
        height: width
        color: theme.darkerBackground
        radius: 12
        border.color: theme.selection
        border.width: 3

        // Pause overlay
        Rectangle {
            anchors.fill: parent
            z: 10
            visible: game.isPaused
            color: Qt.rgba(theme.darkerBackground.r, theme.darkerBackground.g, theme.darkerBackground.b, 0.85)
            radius: 12

            Column {
                anchors.centerIn: parent
                spacing: 16

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "PAUSED"
                    font.pixelSize: 28
                    font.bold: true
                    color: theme.foreground
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Press P or click Resume to continue"
                    font.pixelSize: 14
                    color: theme.muted
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: game.resumeTimer()
            }
        }

        // 3x3 Block Grid
        Grid {
            id: blockGrid
            anchors.fill: parent
            anchors.margins: 4
            columns: 3
            rows: 3
            spacing: 4

            Repeater {
                model: 9 // 9 boxes
                Rectangle {
                    id: boxRect
                    required property int index
                    property int boxRow: Math.floor(index / 3)
                    property int boxCol: index % 3

                    width: (blockGrid.width - 8) / 3
                    height: (blockGrid.height - 8) / 3
                    color: "transparent"
                    border.color: theme.selection
                    border.width: 1
                    radius: 6

                    // 3x3 Cells inside each Box
                    Grid {
                        id: cellGrid
                        anchors.fill: parent
                        anchors.margins: 2
                        columns: 3
                        rows: 3
                        spacing: 2

                        Repeater {
                            model: 9 // 9 cells in this box
                            CellTile {
                                required property int index

                                row: boxRow * 3 + Math.floor(index / 3)
                                col: boxCol * 3 + (index % 3)

                                width: (cellGrid.width - 4) / 3
                                height: (cellGrid.height - 4) / 3
                            }
                        }
                    }
                }
            }
        }
    }

    // Keyboard Event Handling
    Keys.onPressed: function(event) {
        // Return to Menu on Escape
        if (event.key === Qt.Key_Escape) {
            game.returnToMenu();
            root.returnToMenu();
            event.accepted = true;
            return;
        }

        // Pause toggle
        if (event.key === Qt.Key_P) {
            game.togglePause();
            event.accepted = true;
            return;
        }

        if (game.isPaused) {
            if (event.key === Qt.Key_Space || event.key === Qt.Key_Return) {
                game.resumeTimer();
                event.accepted = true;
            }
            return;
        }

        // Arrow and Vim navigation
        if (event.key === Qt.Key_Up || event.key === Qt.Key_K) {
            game.moveSelection(-1, 0);
            event.accepted = true;
            return;
        }
        if (event.key === Qt.Key_Down || event.key === Qt.Key_J) {
            game.moveSelection(1, 0);
            event.accepted = true;
            return;
        }
        if (event.key === Qt.Key_Left || event.key === Qt.Key_H) {
            game.moveSelection(0, -1);
            event.accepted = true;
            return;
        }
        if (event.key === Qt.Key_Right || event.key === Qt.Key_L) {
            game.moveSelection(0, 1);
            event.accepted = true;
            return;
        }

        // Toggle Notes Mode
        if (event.key === Qt.Key_N || event.key === Qt.Key_Space) {
            game.toggleNotesMode();
            event.accepted = true;
            return;
        }

        // Undo
        if (event.key === Qt.Key_U || (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier))) {
            game.undo();
            event.accepted = true;
            return;
        }

        // Clear cell / notes
        if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete || event.key === Qt.Key_0) {
            game.clearSelected();
            event.accepted = true;
            return;
        }

        // Digits 1-9 (Standard numbers and Numpad)
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
            var num = event.key - Qt.Key_0;
            game.enterNumber(num);
            event.accepted = true;
            return;
        }

        // Left-hand Keypad mapping (from original elementary OS game):
        // q, w, e -> 7, 8, 9
        // a, s, d -> 4, 5, 6
        // z, x, c / y, x, c -> 1, 2, 3
        var keyChar = event.text.toLowerCase();
        var leftHandMap = {
            'q': 7, 'w': 8, 'e': 9,
            'a': 4, 's': 5, 'd': 6,
            'y': 1, 'z': 1, 'x': 2, 'c': 3
        };

        if (leftHandMap[keyChar] !== undefined) {
            game.enterNumber(leftHandMap[keyChar]);
            event.accepted = true;
            return;
        }
    }
}
