// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

Button {
    id: control

    required property var theme
    property bool primary: false
    property bool quiet: false

    implicitHeight: 36
    focusPolicy: Qt.StrongFocus
    leftPadding: 14
    rightPadding: 14
    scale: pressed ? 0.98 : 1

    Behavior on scale {
        NumberAnimation { duration: 70 }
    }

    contentItem: Text {
        text: control.text
        color: control.enabled
            ? (control.primary ? "#FFFFFF" : control.theme.textPrimary)
            : control.theme.textMuted
        font.pixelSize: 12
        font.weight: control.primary ? Font.DemiBold : Font.Medium
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: control.theme.radius
        color: !control.enabled ? control.theme.surface
            : control.pressed ? Qt.darker(control.theme.accent, 1.18)
            : control.primary ? control.theme.accent
            : control.hovered ? control.theme.surfaceHover
            : control.quiet ? "transparent"
            : control.theme.surfaceRaised
        border.color: control.activeFocus ? control.theme.accent
            : control.primary ? control.theme.accent
            : control.quiet ? "transparent"
            : control.theme.border
        border.width: control.activeFocus ? 2 : 1
    }

    HoverHandler {
        cursorShape: control.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
