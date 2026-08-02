// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var taskModel
    required property var appSettings
    required property var theme
    signal addRequested()
    signal completionRequested(int taskIndex, string taskTitle)
    signal deletionRequested(int taskIndex, string taskTitle)
    signal planRequested(string taskId)
    signal categoryRequested(
        string taskId,
        string taskTitle,
        string categoryId,
        string subcategoryId)
    signal editRequested(string taskId)
    signal postponeRequested(string taskId, int days)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 3
                Text {
                    text: qsTr("To-do")
                    color: root.theme.textPrimary
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                }
                Text {
                    text: qsTr("Every open task, kept in priority order.")
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                visible: root.width >= 650
                implicitWidth: queueSummary.implicitWidth + 24
                implicitHeight: 32
                radius: 16
                color: root.theme.accentSoft
                border.color: root.theme.borderStrong
                Text {
                    id: queueSummary
                    anchors.centerIn: parent
                    text: qsTr("%1 open · %2")
                        .arg(root.taskModel.activeCount)
                        .arg(root.taskModel.plannedDuration)
                    color: root.theme.accent
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
            }
            AppButton {
                theme: root.theme
                primary: true
                text: qsTr("+ Add task")
                onClicked: root.addRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.border

            Item {
                anchors.fill: parent
                anchors.margins: 14

                ListView {
                    anchors.fill: parent
                    spacing: 7
                    clip: true
                    model: root.taskModel
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: TaskRow {
                        id: taskRow
                        width: ListView.view.width
                        theme: root.theme
                        showReason: root.appSettings.showPriorityReasons
                        showPlanAction: true
                        onCompletionRequested: rowIndex =>
                            root.completionRequested(rowIndex, taskRow.title)
                        onDeletionRequested: rowIndex =>
                            root.deletionRequested(rowIndex, taskRow.title)
                        onPlanRequested: taskId => root.planRequested(taskId)
                        onCategoryRequested: (taskId, categoryId, subcategoryId) =>
                            root.categoryRequested(
                                taskId, taskRow.title, categoryId, subcategoryId)
                        onEditRequested: taskId => root.editRequested(taskId)
                        onPostponeRequested: (taskId, days) =>
                            root.postponeRequested(taskId, days)
                    }
                }

                EmptyState {
                    anchors.centerIn: parent
                    visible: root.taskModel.activeCount === 0
                    theme: root.theme
                }
            }
        }
    }
}
