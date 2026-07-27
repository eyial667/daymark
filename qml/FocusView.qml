// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var focusSession
    required property var taskModel
    required property var theme
    signal backRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.width < 720 ? 16 : 28
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text {
                    text: qsTr("Focus")
                    color: root.theme.textPrimary
                    font.pixelSize: 27
                    font.weight: Font.DemiBold
                }
                Text {
                    text: qsTr("One task, one visible timer, no hidden state.")
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }
            }
            AppButton {
                theme: root.theme
                text: qsTr("Back to Today")
                onClicked: root.backRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.width < 720 ? 250 : 300
            radius: root.theme.radius + 4
            color: root.theme.surface
            border.color: root.focusSession.running
                ? root.theme.accent : root.theme.borderStrong
            border.width: root.focusSession.running ? 2 : 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.width < 720 ? 18 : 28
                spacing: 12

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: root.focusSession.hasTask
                        ? qsTr("CURRENT FOCUS") : qsTr("NO TASK SELECTED")
                    color: root.theme.accent
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    font.letterSpacing: 1.3
                }
                Text {
                    Layout.fillWidth: true
                    text: root.focusSession.hasTask
                        ? root.focusSession.taskTitle
                        : qsTr("Choose a task from today’s queue below.")
                    color: root.theme.textPrimary
                    font.pixelSize: root.width < 720 ? 20 : 26
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: root.focusSession.hasTask
                        ? root.focusSession.remainingText : "--:--"
                    color: root.focusSession.finished
                        ? root.theme.success : root.theme.textPrimary
                    font.pixelSize: root.width < 720 ? 48 : 68
                    font.weight: Font.Light
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 8
                    radius: 4
                    color: root.theme.surfaceRaised

                    Rectangle {
                        width: parent.width * (root.focusSession.totalSeconds > 0
                            ? 1 - root.focusSession.remainingSeconds
                                / root.focusSession.totalSeconds
                            : 0)
                        height: parent.height
                        radius: parent.radius
                        color: root.focusSession.finished
                            ? root.theme.success : root.theme.accent
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 10

                    AppButton {
                        theme: root.theme
                        primary: true
                        text: root.focusSession.running ? qsTr("Pause") : qsTr("Start")
                        enabled: root.focusSession.hasTask
                        onClicked: {
                            if (root.focusSession.running)
                                root.focusSession.pause()
                            else
                                root.focusSession.start()
                        }
                    }
                    AppButton {
                        theme: root.theme
                        text: qsTr("Reset")
                        enabled: root.focusSession.hasTask
                        onClicked: root.focusSession.reset()
                    }
                    AppButton {
                        theme: root.theme
                        quiet: true
                        text: qsTr("Clear")
                        enabled: root.focusSession.hasTask && !root.focusSession.running
                        onClicked: root.focusSession.clear()
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                text: qsTr("TODAY’S TASKS")
                color: root.theme.textMuted
                font.pixelSize: 10
                font.weight: Font.Bold
                font.letterSpacing: 1.1
            }
            Text {
                text: qsTr("%1 available").arg(root.taskModel.activeCount)
                color: root.theme.textMuted
                font.pixelSize: 10
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.border

            ListView {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 7
                clip: true
                model: root.taskModel

                delegate: Rectangle {
                    id: focusTask

                    required property string taskId
                    required property string title
                    required property int estimatedMinutes

                    width: ListView.view.width
                    height: 54
                    radius: root.theme.radius
                    color: root.focusSession.taskId === focusTask.taskId
                        ? root.theme.accentSoft : root.theme.surfaceRaised
                    border.color: root.focusSession.taskId === focusTask.taskId
                        ? root.theme.accent : root.theme.border

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 8
                        spacing: 10
                        Text {
                            Layout.fillWidth: true
                            text: focusTask.title
                            color: root.theme.textPrimary
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                        }
                        Text {
                            text: qsTr("%1 min").arg(focusTask.estimatedMinutes)
                            color: root.theme.textMuted
                            font.pixelSize: 10
                        }
                        AppButton {
                            theme: root.theme
                            text: root.focusSession.taskId === focusTask.taskId
                                ? qsTr("Selected") : qsTr("Focus")
                            enabled: !root.focusSession.running
                                && root.focusSession.taskId !== focusTask.taskId
                            onClicked: root.focusSession.prepareTask(
                                focusTask.taskId,
                                focusTask.title,
                                focusTask.estimatedMinutes)
                        }
                    }
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
