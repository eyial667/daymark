// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: row

    required property var theme
    required property int index
    required property string taskId
    required property string title
    required property string categoryId
    required property string categoryName
    required property string subcategoryId
    required property string subcategoryName
    required property bool isInToday
    required property string dueText
    required property string notes
    required property int estimatedMinutes
    required property int priorityScore
    required property string priorityReason
    property bool compact: false
    property bool showReason: true
    property bool showPlanAction: false
    readonly property string categoryPath: subcategoryName.length > 0
        ? categoryName + " / " + subcategoryName : categoryName

    signal completionRequested(int taskIndex)
    signal deletionRequested(int taskIndex)
    signal categoryRequested(
        string taskId,
        string currentCategoryId,
        string currentSubcategoryId)
    signal planRequested(string taskId)
    signal editRequested(string taskId)
    signal postponeRequested(string taskId, int days)

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
                    text: qsTr("✓")
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

                Text {
                    visible: row.notes.length > 0
                    text: qsTr("✎")
                    color: row.theme.textMuted
                    font.pixelSize: 12
                    ToolTip.visible: notesHover.hovered
                    ToolTip.text: row.notes

                    HoverHandler { id: notesHover }
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

                Button {
                    id: dueButton

                    implicitWidth: dueLabel.implicitWidth + 16
                    implicitHeight: 24
                    leftPadding: 8
                    rightPadding: 8
                    text: row.dueText

                    contentItem: Text {
                        id: dueLabel
                        text: dueButton.text
                        color: row.dueText.indexOf(qsTr("Overdue")) === 0
                            ? row.theme.danger
                            : row.theme.textMuted
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 12
                        color: dueButton.hovered
                            ? row.theme.surfaceHover : "transparent"
                        border.color: dueButton.hovered || dueButton.activeFocus
                            ? row.theme.border : "transparent"
                    }
                    ToolTip.visible: hovered && !postponeMenu.visible
                    ToolTip.text: qsTr("Postpone this task")
                    onClicked: postponeMenu.open()

                    Menu {
                        id: postponeMenu

                        y: dueButton.height + 3
                        padding: 4

                        background: Rectangle {
                            implicitWidth: 190
                            radius: row.theme.radius
                            color: row.theme.surfaceRaised
                            border.color: row.theme.borderStrong
                        }

                        delegate: MenuItem {
                            id: postponeItem

                            implicitHeight: 32

                            contentItem: Text {
                                text: postponeItem.text
                                color: row.theme.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                radius: row.theme.radius > 5 ? 5 : 3
                                color: postponeItem.highlighted
                                    ? row.theme.surfaceHover : "transparent"
                            }
                        }

                        MenuItem {
                            text: qsTr("Postpone to tomorrow")
                            onTriggered: row.postponeRequested(row.taskId, 1)
                        }
                        MenuItem {
                            text: qsTr("Postpone by three days")
                            onTriggered: row.postponeRequested(row.taskId, 3)
                        }
                        MenuItem {
                            text: qsTr("Postpone by a week")
                            onTriggered: row.postponeRequested(row.taskId, 7)
                        }
                    }
                }

                Rectangle {
                    implicitWidth: durationText.implicitWidth + 16
                    implicitHeight: 24
                    radius: 12
                    color: row.theme.accentSoft
                    border.color: row.theme.borderStrong

                    Text {
                        id: durationText
                        anchors.centerIn: parent
                        text: qsTr("%1 min").arg(row.estimatedMinutes)
                        color: row.theme.accent
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }
                }

                Button {
                    id: categoryButton

                    implicitWidth: Math.min(142, categoryText.implicitWidth + 18)
                    implicitHeight: 24
                    leftPadding: 9
                    rightPadding: 9
                    text: row.categoryPath.length > 0
                        ? row.categoryPath : qsTr("Uncategorized")

                    contentItem: Text {
                        id: categoryText
                        text: categoryButton.text
                        color: row.categoryName.length > 0
                            ? row.theme.secondaryAccent : row.theme.textMuted
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 12
                        color: categoryButton.hovered
                            ? row.theme.surfaceHover : row.theme.surfaceRaised
                        border.color: row.categoryName.length > 0
                            ? row.theme.secondaryAccent : row.theme.border
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Change category")
                    onClicked: row.categoryRequested(
                        row.taskId,
                        row.categoryId,
                        row.subcategoryId)
                }
            }
        }

        AppButton {
            visible: row.showPlanAction && !row.isInToday
            Layout.preferredWidth: visible ? 102 : 0
            implicitHeight: 32
            leftPadding: 10
            rightPadding: 10
            theme: row.theme
            primary: true
            text: qsTr("Add to Today")
            onClicked: row.planRequested(row.taskId)
        }

        AppButton {
            Layout.preferredWidth: 34
            implicitHeight: 32
            leftPadding: 8
            rightPadding: 8
            theme: row.theme
            quiet: true
            text: "✎"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit task")
            onClicked: row.editRequested(row.taskId)
        }

        AppButton {
            Layout.preferredWidth: 34
            implicitHeight: 32
            leftPadding: 8
            rightPadding: 8
            theme: row.theme
            quiet: true
            text: "×"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Delete task")
            onClicked: row.deletionRequested(row.index)
        }
    }

    HoverHandler { id: rowHover }
}
