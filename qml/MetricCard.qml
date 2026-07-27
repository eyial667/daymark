// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: card

    required property var theme
    property string label: ""
    property string value: ""
    property color accent: theme.accent

    implicitHeight: 92
    radius: theme.radius
    color: theme.surface
    border.color: theme.border
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 5
            Layout.fillHeight: true
            radius: 3
            color: card.accent
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            Text {
                text: card.label
                color: card.theme.textMuted
                font.pixelSize: 10
                font.weight: Font.Medium
            }

            Text {
                Layout.fillWidth: true
                text: card.value
                color: card.theme.textPrimary
                elide: Text.ElideRight
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }
        }
    }
}
