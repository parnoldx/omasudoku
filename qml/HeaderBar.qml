import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "Components"

Rectangle {
    id: root

    height: 56
    color: theme.darkerBackground

    signal goHome()
    signal newGame()

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: theme.selection
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 10

        // Home / Back button
        CustomButton {
            id: backBtn
            iconText: "←"
            text: "Menu"
            fontSize: 13
            implicitHeight: 34
            implicitWidth: 80
            onClicked: root.goHome()
        }

        // Score Badge
        Rectangle {
            visible: game.inGame
            implicitHeight: 34
            implicitWidth: scoreText.implicitWidth + 24
            radius: 17
            color: Qt.rgba(theme.yellow.r, theme.yellow.g, theme.yellow.b, 0.15)
            border.color: Qt.rgba(theme.yellow.r, theme.yellow.g, theme.yellow.b, 0.4)
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 6
                Text {
                    text: "★"
                    font.pixelSize: 13
                    color: theme.yellow
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    id: scoreText
                    text: game.points.toLocaleString()
                    font.pixelSize: 14
                    font.bold: true
                    color: theme.yellow
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Factor / Multiplier Badge
        Rectangle {
            id: factorBadge
            visible: game.inGame
            implicitHeight: 34
            implicitWidth: factorText.implicitWidth + 24
            radius: 17
            color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.15)
            border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.4)
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 4
                Text {
                    text: "mult"
                    font.pixelSize: 11
                    color: theme.muted
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    id: factorText
                    text: "x" + game.factor
                    font.pixelSize: 14
                    font.bold: true
                    color: theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Pulse on factor change
            Connections {
                target: game
                function onFactorChanged() {
                    factorPulseAnim.restart();
                }
            }

            SequentialAnimation {
                id: factorPulseAnim
                PropertyAnimation { target: factorBadge; property: "scale"; to: 1.15; duration: 120 }
                PropertyAnimation { target: factorBadge; property: "scale"; to: 1.0; duration: 180 }
            }
        }

        Item { Layout.fillWidth: true }

        // Fails / Series Indicator (Center)
        Rectangle {
            visible: game.inGame
            implicitHeight: 34
            implicitWidth: failsContent.implicitWidth + 20
            radius: 17
            color: {
                if (game.fails > 3) return Qt.rgba(theme.red.r, theme.red.g, theme.red.b, 0.2);
                if (game.fails > 0) return Qt.rgba(theme.orange.r, theme.orange.g, theme.orange.b, 0.15);
                return Qt.rgba(theme.green.r, theme.green.g, theme.green.b, 0.15);
            }
            border.width: 1
            border.color: {
                if (game.fails > 3) return theme.red;
                if (game.fails > 0) return theme.orange;
                return theme.green;
            }

            Row {
                id: failsContent
                anchors.centerIn: parent
                spacing: 6

                Text {
                    text: {
                        if (game.fails === 0) return "✓ Perfect";
                        if (game.fails === 1) return "✗ (1 fail)";
                        if (game.fails === 2) return "✗ ✗ (2 fails)";
                        if (game.fails === 3) return "✗ ✗ ✗ (last chance)";
                        return "⚡ BROKEN SERIES (" + game.fails + ")";
                    }
                    font.pixelSize: 12
                    font.bold: true
                    color: {
                        if (game.fails > 3) return theme.red;
                        if (game.fails > 0) return theme.orange;
                        return theme.green;
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Timer Badge
        Rectangle {
            visible: game.inGame
            implicitHeight: 34
            implicitWidth: timerText.implicitWidth + 20
            radius: 17
            color: theme.darkBackground
            border.color: theme.selection
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 6
                Text {
                    text: "⏱"
                    font.pixelSize: 12
                    color: theme.muted
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    id: timerText
                    text: game.formattedTime
                    font.pixelSize: 13
                    font.bold: true
                    font.family: "Monospace"
                    color: theme.foreground
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Notes Button
        CustomButton {
            visible: game.inGame
            iconText: "✏️"
            text: "Notes"
            isActive: game.notesMode
            fontSize: 12
            implicitHeight: 34
            implicitWidth: 80
            onClicked: game.toggleNotesMode()
        }

        // Undo Button
        CustomButton {
            visible: game.inGame
            iconText: "↺"
            text: ""
            fontSize: 14
            implicitHeight: 34
            implicitWidth: 38
            onClicked: game.undo()
        }

        // Pause Button
        CustomButton {
            visible: game.inGame
            iconText: game.isPaused ? "▶" : "⏸"
            text: ""
            fontSize: 14
            implicitHeight: 34
            implicitWidth: 38
            onClicked: game.togglePause()
        }

        // New Game
        CustomButton {
            visible: game.inGame
            text: "New"
            fontSize: 12
            implicitHeight: 34
            implicitWidth: 60
            onClicked: root.newGame()
        }
    }
}
