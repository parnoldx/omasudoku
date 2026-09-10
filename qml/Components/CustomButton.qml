import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string text: ""
    property string iconText: ""
    property bool isPrimary: false
    property bool isOutlined: false
    property bool isActive: false
    property string customColor: ""
    property string customTextColor: ""
    property int radius: 10
    property int fontSize: 14

    signal clicked()

    implicitWidth: {
        if (root.text.length > 0 && root.iconText.length > 0) {
            return contentRow.implicitWidth + 24;
        } else if (root.text.length > 0) {
            return contentRow.implicitWidth + 24;
        } else {
            return implicitHeight;
        }
    }
    implicitHeight: 42

    Layout.preferredWidth: implicitWidth
    Layout.minimumWidth: implicitWidth
    Layout.preferredHeight: implicitHeight
    Layout.minimumHeight: implicitHeight

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: root.radius
        border.width: root.isOutlined || root.isActive ? 2 : 1

        border.color: {
            if (root.isActive) return theme.accent;
            if (root.isOutlined) return theme.accent;
            if (mouseArea.containsMouse) return theme.accent;
            return theme.selection;
        }

        color: {
            if (root.customColor.length > 0) {
                return mouseArea.pressed ? Qt.darker(root.customColor, 1.2) : root.customColor;
            }
            if (root.isPrimary) {
                return mouseArea.pressed ? Qt.darker(theme.accent, 1.2) :
                       mouseArea.containsMouse ? Qt.lighter(theme.accent, 1.1) : theme.accent;
            }
            if (root.isActive) {
                return Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.25);
            }
            if (mouseArea.pressed) {
                return theme.selection;
            }
            if (mouseArea.containsMouse) {
                return theme.lighterBackground;
            }
            return theme.darkBackground;
        }

        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: root.iconText.length > 0 && root.text.length > 0 ? 6 : 0

            Text {
                textFormat: Text.PlainText
                text: root.iconText
                visible: root.iconText.length > 0
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize + (root.text.length === 0 ? 4 : 1)
                font.bold: true
                font.family: "JetBrainsMono Nerd Font, Liberation Sans, monospace"
                color: {
                    if (root.isPrimary) return theme.background;
                    if (root.customTextColor.length > 0) return root.customTextColor;
                    if (root.isActive) return theme.accent;
                    if (mouseArea.containsMouse) return theme.foreground;
                    return theme.lightForeground;
                }
            }

            Text {
                textFormat: Text.PlainText
                text: root.text
                visible: root.text.length > 0
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize
                font.bold: root.isPrimary || root.isActive
                font.family: "JetBrainsMono Nerd Font, Liberation Sans, monospace"
                color: {
                    if (root.isPrimary) return theme.background;
                    if (root.customTextColor.length > 0) return root.customTextColor;
                    if (root.isActive) return theme.accent;
                    if (mouseArea.containsMouse) return theme.foreground;
                    return theme.lightForeground;
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
