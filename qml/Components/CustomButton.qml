import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string text: ""
    property string iconText: ""
    property bool isPrimary: false
    property bool isOutlined: false
    property bool isActive: false
    property color customColor: "transparent"
    property color customTextColor: "transparent"
    property int radius: 8
    property int fontSize: 14

    signal clicked()

    implicitWidth: contentRow.implicitWidth + 24
    implicitHeight: 40

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: root.radius
        border.width: root.isOutlined || root.isActive ? 2 : (mouseArea.containsMouse ? 1 : 0)

        border.color: {
            if (root.isActive) return theme.accent;
            if (root.isOutlined) return theme.accent;
            if (mouseArea.containsMouse) return theme.selection;
            return "transparent";
        }

        color: {
            if (root.customColor !== "transparent") {
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

        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: root.iconText.length > 0 && root.text.length > 0 ? 8 : 0

            Text {
                text: root.iconText
                visible: root.iconText.length > 0
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize + 2
                color: root.isPrimary ? theme.background :
                       (root.customTextColor !== "transparent" ? root.customTextColor :
                       (root.isActive ? theme.accent : theme.foreground))
            }

            Text {
                text: root.text
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: root.fontSize
                font.bold: root.isPrimary || root.isActive
                color: root.isPrimary ? theme.background :
                       (root.customTextColor !== "transparent" ? root.customTextColor :
                       (root.isActive ? theme.accent : theme.foreground))
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
