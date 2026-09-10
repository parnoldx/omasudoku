import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "Components"

Item {
    id: root

    signal startGame(string difficultyKey)
    signal resumeGame()

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: contentCol.implicitHeight + 48
        clip: true

        ColumnLayout {
            id: contentCol
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 48, 640)
            spacing: 24

            Item { height: 16 }

            // Hero Header
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                // Grid Logo Badge (Jeweled with Omarchy theme colors)
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 72
                    height: 72
                    radius: 16
                    color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.15)
                    border.color: theme.accent
                    border.width: 2

                    Grid {
                        anchors.centerIn: parent
                        columns: 3
                        rows: 3
                        spacing: 3

                        Repeater {
                            model: 9
                            Rectangle {
                                width: 14
                                height: 14
                                radius: 3
                                color: {
                                    if (index === 0) return theme.green;
                                    if (index === 2) return theme.cyan;
                                    if (index === 4) return theme.yellow;
                                    if (index === 6) return theme.orange;
                                    if (index === 8) return theme.magenta;
                                    return theme.selection;
                                }
                            }
                        }
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "SUDOKU"
                    font.pixelSize: 32
                    font.bold: true
                    font.letterSpacing: 4
                    color: theme.foreground
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Classic arcade puzzle game for Omarchy"
                    font.pixelSize: 14
                    color: theme.muted
                }
            }

            // Resume Game Card (if saved game exists)
            Rectangle {
                visible: game.canResume
                Layout.fillWidth: true
                implicitHeight: 64
                radius: 12
                color: Qt.rgba(theme.green.r, theme.green.g, theme.green.b, 0.15)
                border.color: theme.green
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    spacing: 12

                    Text {
                        text: "▶"
                        font.pixelSize: 20
                        color: theme.green
                    }

                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: "Resume Unfinished Game"
                            font.pixelSize: 15
                            font.bold: true
                            color: theme.foreground
                        }
                        Text {
                            text: "Pick up right where you left off"
                            font.pixelSize: 12
                            color: theme.muted
                        }
                    }

                    Item { Layout.fillWidth: true }

                    CustomButton {
                        text: "Resume"
                        isPrimary: true
                        customColor: theme.green
                        customTextColor: theme.darkerBackground
                        fontSize: 13
                        implicitHeight: 36
                        implicitWidth: 100
                        onClicked: root.resumeGame()
                    }
                }
            }

            Text {
                text: "CHOOSE DIFFICULTY"
                font.pixelSize: 12
                font.bold: true
                font.letterSpacing: 1.5
                color: theme.muted
                Layout.leftMargin: 4
            }

            // 4 Difficulty Cards Grid
            GridLayout {
                Layout.fillWidth: true
                columns: parent.width > 500 ? 2 : 1
                rowSpacing: 12
                columnSpacing: 12

                // Helper component for difficulty cards
                Repeater {
                    model: [
                        { key: "simple", name: "Easy", clues: "~40 clues", factor: "x28", color: theme.green, desc: "Relaxed pacing, great for quick sessions" },
                        { key: "easy", name: "Medium", clues: "~34 clues", factor: "x56", color: theme.cyan, desc: "Balanced challenge for regular players" },
                        { key: "intermediate", name: "Hard", clues: "~29 clues", factor: "x112", color: theme.orange, desc: "Requires careful deduction & notes" },
                        { key: "expert", name: "Master", clues: "~25 clues", factor: "x156", color: theme.magenta, desc: "Intense challenge for puzzle masters" }
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 110
                        radius: 12
                        color: cardMouse.containsMouse ? Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.12) : theme.darkBackground
                        border.color: cardMouse.containsMouse ? modelData.color : Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.3)
                        border.width: cardMouse.containsMouse ? 2 : 1
                        clip: true

                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        // Left vertical accent stripe
                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 4
                            color: modelData.color
                            opacity: cardMouse.containsMouse ? 1.0 : 0.7
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 14
                            anchors.topMargin: 14
                            anchors.bottomMargin: 14
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Text {
                                    text: modelData.name
                                    font.pixelSize: 17
                                    font.bold: true
                                    color: modelData.color
                                }

                                Item { Layout.fillWidth: true }

                                // Factor badge
                                Rectangle {
                                    implicitHeight: 22
                                    implicitWidth: factorTxt.implicitWidth + 12
                                    radius: 11
                                    color: Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.2)
                                    Text {
                                        id: factorTxt
                                        anchors.centerIn: parent
                                        text: modelData.factor
                                        font.pixelSize: 11
                                        font.bold: true
                                        color: modelData.color
                                    }
                                }
                            }

                            Text {
                                text: modelData.desc
                                font.pixelSize: 12
                                color: theme.muted
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Item { Layout.fillHeight: true }

                            RowLayout {
                                Layout.fillWidth: true
                                Text {
                                    text: modelData.clues
                                    font.pixelSize: 11
                                    color: theme.muted
                                }

                                Item { Layout.fillWidth: true }

                                Text {
                                    property int hs: game.getHighscoreFor(modelData.key)
                                    text: hs > 0 ? ("Best: " + hs.toLocaleString()) : "No record"
                                    font.pixelSize: 11
                                    font.bold: hs > 0
                                    color: hs > 0 ? theme.yellow : theme.muted
                                }
                            }
                        }

                        MouseArea {
                            id: cardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.startGame(modelData.key)
                        }
                    }
                }
            }

            // Keyboard Shortcuts Card
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 90
                radius: 12
                color: theme.darkerBackground
                border.color: theme.selection
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        text: "KEYBOARD CONTROLS"
                        font.pixelSize: 10
                        font.bold: true
                        font.letterSpacing: 1
                        color: theme.muted
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            spacing: 2
                            Text { text: "Navigation"; font.pixelSize: 11; font.bold: true; color: theme.foreground }
                            Text { text: "Arrows / H J K L"; font.pixelSize: 11; color: theme.muted }
                        }

                        Rectangle { width: 1; height: 28; color: theme.selection }

                        ColumnLayout {
                            spacing: 2
                            Text { text: "Left-Hand Keypad"; font.pixelSize: 11; font.bold: true; color: theme.accent }
                            Text { text: "QWE (789) ASD (456) ZXC (123)"; font.pixelSize: 11; color: theme.muted }
                        }

                        Rectangle { width: 1; height: 28; color: theme.selection }

                        ColumnLayout {
                            spacing: 2
                            Text { text: "Pencil Notes & Undo"; font.pixelSize: 11; font.bold: true; color: theme.foreground }
                            Text { text: "N or Space (Notes) • U (Undo)"; font.pixelSize: 11; color: theme.muted }
                        }
                    }
                }
            }

            Item { height: 16 }
        }
    }
}
