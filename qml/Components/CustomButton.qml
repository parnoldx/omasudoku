import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

Item {
    id: root

    property string text: ""
    property string iconName: ""
    property string iconText: ""
    property int iconSize: 18
    property bool isPrimary: false
    property bool isOutlined: false
    property bool isActive: false
    property string customColor: ""
    property string customTextColor: ""
    property int radius: 10
    property int fontSize: 14

    signal clicked()

    readonly property color contentColor: {
        if (root.isPrimary) return theme.background;
        if (root.customTextColor.length > 0) return root.customTextColor;
        if (root.isActive) return theme.accent;
        if (mouseArea.containsMouse) return theme.foreground;
        return theme.lightForeground;
    }

    implicitWidth: {
        var hasIcon = root.iconName.length > 0 || root.iconText.length > 0;
        if (root.text.length > 0 && hasIcon) {
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
            spacing: (root.iconName.length > 0 || root.iconText.length > 0) && root.text.length > 0 ? 8 : 0

            // Vector Icon support
            Item {
                id: vectorIconItem
                visible: root.iconName.length > 0
                width: root.iconSize
                height: root.iconSize
                anchors.verticalCenter: parent.verticalCenter

                Shape {
                    id: vectorShape
                    width: 24
                    height: 24
                    scale: root.iconSize / 24.0
                    transformOrigin: Item.Center
                    anchors.centerIn: parent
                    layer.enabled: true
                    layer.samples: 4

                    ShapePath {
                        fillColor: root.contentColor
                        strokeWidth: 0

                        PathSvg {
                            path: {
                                switch (root.iconName) {
                                    case "undo":
                                        return "M 12.5 8 c -2.65 0 -5.05 1 -6.9 2.6 L 2 7 v 9 h 9 l -3.62 -3.62 c 1.39 -1.2 3.16 -1.98 5.12 -1.98 c 3.84 0 7.02 2.7 7.74 6.3 l 2.44 -0.5 C 21.71 11.53 17.56 8 12.5 8 Z";
                                    case "pause":
                                        return "M 6 5 h 3.5 v 14 H 6 Z M 14.5 5 h 3.5 v 14 H 14.5 Z";
                                    case "play":
                                        return "M 8 5 v 14 l 11 -7 Z";
                                    case "pencil":
                                        return "M 3 17.25 V 21 h 3.75 L 17.81 9.94 l -3.75 -3.75 L 3 17.25 Z M 20.71 7.04 c 0.39 -0.39 0.39 -1.02 0 -1.41 l -2.34 -2.34 c -0.39 -0.39 -1.02 -0.39 -1.41 0 l -1.83 1.83 3.75 3.75 1.83 -1.83 Z";
                                    case "back":
                                        return "M 20 11 H 7.83 l 5.59 -5.59 L 12 4 l -8 8 l 8 8 l 1.41 -1.41 L 7.83 13 H 20 v -2 Z";
                                    case "clear":
                                        return "M 22 3 H 7 c -0.69 0 -1.23 0.35 -1.59 0.88 L 0 12 l 5.41 8.11 c 0.36 0.53 0.9 0.89 1.59 0.89 h 15 c 1.1 0 2 -0.9 2 -2 V 5 c 0 -1.1 -0.9 -2 -2 -2 Z m -3 12.59 L 17.59 17 L 14 13.41 L 10.41 17 L 9 15.59 L 12.59 12 L 9 8.41 L 10.41 7 L 14 10.59 L 17.59 7 L 19 8.41 L 15.41 12 L 19 15.59 Z";
                                    default:
                                        return "";
                                }
                            }
                        }
                    }
                }
            }

            // Fallback Text Icon
            Text {
                textFormat: Text.PlainText
                text: root.iconText
                visible: root.iconName.length === 0 && root.iconText.length > 0
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize + (root.text.length === 0 ? 4 : 1)
                font.bold: true
                font.family: "JetBrainsMono Nerd Font, Liberation Sans, monospace"
                color: root.contentColor
            }

            // Text Label
            Text {
                textFormat: Text.PlainText
                text: root.text
                visible: root.text.length > 0
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize
                font.bold: root.isPrimary || root.isActive
                font.family: "JetBrainsMono Nerd Font, Liberation Sans, monospace"
                color: root.contentColor
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
