// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: row

    required property var theme
    required property int index
    required property string title
    required property string project
    required property string dueText
    required property int estimatedMinutes
    required property int priorityScore
    required property string priorityReason
    property bool compact: false
    property bool showReason: true

    signal completionRequested(int taskIndex)

    implicitHeight: compact ? 68 : 84
    radius: theme.radius
    color: rowHover.hovered ? theme.surfaceHover : theme.surface
    border.color: theme.border
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 14
        spacing: 10

        CheckBox {
            id: completedCheck
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: row.compact ? 9 : 13
            indicator: Rectangle {
                implicitWidth: 18
                implicitHeight: 18
                radius: row.theme.radius > 5 ? 5 : 3
                color: completedCheck.checked ? row.theme.accent : "transparent"
                border.color: completedCheck.checked ? row.theme.accent : row.theme.borderStrong

                Text {
                    anchors.centerIn: parent
                    visible: completedCheck.checked
                    text: "✓"
                    color: "white"
                    font.pixelSize: 11
                    font.weight: Font.Bold
                }
            }
            onClicked: row.completionRequested(row.index)
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: row.compact ? 8 : 11
            Layout.bottomMargin: row.compact ? 8 : 10
            spacing: row.compact ? 3 : 5

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: row.title
                    color: row.theme.textPrimary
                    elide: Text.ElideRight
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }

                Rectangle {
                    implicitWidth: scoreLabel.implicitWidth + 16
                    implicitHeight: 25
                    radius: 13
                    color: row.priorityScore >= 80
                        ? row.theme.dangerSoft
                        : row.theme.accentSoft

                    Text {
                        id: scoreLabel
                        anchors.centerIn: parent
                        text: row.priorityScore
                        color: row.priorityScore >= 80 ? row.theme.danger : row.theme.accent
                        font.pixelSize: 11
                        font.weight: Font.Bold
                    }
                }
            }

            Text {
                visible: row.showReason && !row.compact
                Layout.fillWidth: true
                text: row.priorityReason
                color: row.theme.textSecondary
                elide: Text.ElideRight
                font.pixelSize: 12
            }

            RowLayout {
                spacing: 12

                Text {
                    text: row.dueText
                    color: row.dueText.indexOf("Overdue") === 0
                        ? row.theme.danger
                        : row.theme.textMuted
                    font.pixelSize: 11
                    font.weight: Font.Medium
                }

                Text {
                    text: row.estimatedMinutes + " min"
                    color: row.theme.textMuted
                    font.pixelSize: 11
                }

                Text {
                    visible: row.project.length > 0
                    text: row.project
                    color: row.theme.secondaryAccent
                    font.pixelSize: 11
                    font.weight: Font.Medium
                }
            }
        }
    }

    HoverHandler { id: rowHover }
}
