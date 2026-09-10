import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "Components"

Rectangle {
    id: root

    implicitHeight: 68
    Layout.preferredHeight: 68
    Layout.minimumHeight: 68
    Layout.fillWidth: true
    color: theme.darkerBackground

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: theme.selection
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: 8

        // Undo
        CustomButton {
            iconText: "↺"
            text: ""
            fontSize: 16
            implicitWidth: 44
            implicitHeight: 44
            radius: 10
            onClicked: game.undo()
        }

        // Notes Toggle
        CustomButton {
            iconText: "✎"
            text: ""
            isActive: game.notesMode
            fontSize: 15
            implicitWidth: 44
            implicitHeight: 44
            radius: 10
            onClicked: game.toggleNotesMode()
        }

        // Numbers 1 to 9
        Repeater {
            model: 9
            Item {
                property int num: index + 1
                implicitWidth: 44
                implicitHeight: 44
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44

                // Check how many of this number are placed on board
                property int countPlaced: {
                    var count = 0;
                    var b = game.board;
                    for (var i = 0; i < 81; i++) {
                        if (b[i] === num) count++;
                    }
                    return count;
                }
                property bool isAllPlaced: countPlaced >= 9

                CustomButton {
                    anchors.fill: parent
                    text: parent.num.toString()
                    fontSize: 19
                    radius: 10
                    isActive: game.highlightNum === parent.num
                    opacity: parent.isAllPlaced ? 0.35 : 1.0
                    onClicked: game.enterNumber(parent.num)
                }

                // Small count indicator
                Text {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: parent.isAllPlaced ? "✓" : (9 - parent.countPlaced).toString()
                    font.pixelSize: 8
                    font.bold: true
                    color: parent.isAllPlaced ? theme.green : theme.muted
                }
            }
        }

        // Clear button
        CustomButton {
            iconText: "⌫"
            text: ""
            fontSize: 16
            implicitWidth: 44
            implicitHeight: 44
            radius: 10
            onClicked: game.clearSelected()
        }
    }
}
