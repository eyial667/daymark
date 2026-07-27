// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: emptyState

    required property var theme
    property string title: "Your queue is clear"
    property string detail: "Add a task to let Daymark plan the next action."

    spacing: 8

    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        implicitWidth: 44
        implicitHeight: 44
        radius: 22
        color: emptyState.theme.accentSoft
        border.color: emptyState.theme.borderStrong

        Text {
            anchors.centerIn: parent
            text: "✓"
            color: emptyState.theme.accent
            font.pixelSize: 19
            font.weight: Font.Bold
        }
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        text: emptyState.title
        color: emptyState.theme.textPrimary
        font.pixelSize: 16
        font.weight: Font.DemiBold
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        text: emptyState.detail
        color: emptyState.theme.textMuted
        font.pixelSize: 11
    }
}
