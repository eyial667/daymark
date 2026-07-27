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
    signal suggestionRequested()
    signal focusRequested()
    signal completionRequested(int taskIndex, string taskTitle)
    signal categoryRequested(
        string taskId,
        string taskTitle,
        string categoryId,
        string subcategoryId)

    function formattedHour(hour) {
        if (appSettings.use24HourClock)
            return String(hour).padStart(2, "0") + ":00"
        const displayHour = hour > 12 ? hour - 12 : hour
        return displayHour + (hour < 12 ? " AM" : " PM")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                visible: root.width >= 700
                Layout.fillWidth: true
                implicitHeight: 42
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    spacing: 10

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("MIDNIGHT COMMAND  ·  TODAY’S PRIORITY QUEUE")
                        color: root.theme.textSecondary
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }
                }
            }

            Text {
                visible: root.width >= 700
                text: Qt.formatDate(new Date(), qsTr("MMM d, ddd"))
                color: root.theme.textSecondary
                font.pixelSize: 12
            }

            AppButton {
                theme: root.theme
                primary: true
                text: qsTr("Give me something to do")
                onClicked: root.suggestionRequested()
            }

            AppButton {
                theme: root.theme
                text: qsTr("+ Add task")
                onClicked: root.addRequested()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 218
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        Text {
                            text: qsTr("NEXT UP")
                            color: root.theme.accent
                            font.pixelSize: 10
                            font.weight: Font.Bold
                            font.letterSpacing: 1.2
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root.taskModel.topTaskTitle
                            color: root.theme.textPrimary
                            elide: Text.ElideRight
                            font.pixelSize: 30
                            font.weight: Font.DemiBold
                        }

                        Text {
                            visible: root.taskModel.activeCount > 0
                            text: qsTr("Highest impact action in your current queue.")
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                        }

                        Item { Layout.fillHeight: true }

                        RowLayout {
                            visible: root.taskModel.activeCount > 0
                            spacing: 8

                            Rectangle {
                                implicitWidth: 88
                                implicitHeight: 28
                                radius: root.theme.radius
                                color: root.theme.accentSoft
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Priority 1")
                                    color: root.theme.accent
                                    font.pixelSize: 10
                                    font.weight: Font.DemiBold
                                }
                            }

                            Rectangle {
                                implicitWidth: 108
                                implicitHeight: 28
                                radius: root.theme.radius
                                color: root.theme.surfaceRaised
                                border.color: root.theme.border
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("%1 queued").arg(root.taskModel.plannedDuration)
                                    color: root.theme.textSecondary
                                    font.pixelSize: 10
                                }
                            }

                            Item { Layout.fillWidth: true }

                            AppButton {
                                theme: root.theme
                                primary: true
                                text: qsTr("Open focus  →")
                                enabled: root.taskModel.activeCount > 0
                                onClicked: root.focusRequested()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: qsTr("Priority queue")
                        color: root.theme.accent
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: qsTr("%1 tasks · %2")
                            .arg(root.taskModel.activeCount)
                            .arg(root.taskModel.plannedDuration)
                        color: root.theme.textMuted
                        font.pixelSize: 10
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        anchors.fill: parent
                        spacing: 6
                        clip: true
                        model: root.taskModel
                        boundsBehavior: Flickable.StopAtBounds

                        delegate: TaskRow {
                            id: taskRow
                            width: ListView.view.width
                            theme: root.theme
                            compact: true
                            showReason: root.appSettings.showPriorityReasons
                            onCompletionRequested: rowIndex =>
                                root.completionRequested(rowIndex, taskRow.title)
                            onCategoryRequested: (taskId, categoryId, subcategoryId) =>
                                root.categoryRequested(
                                    taskId, taskRow.title, categoryId, subcategoryId)
                        }
                    }

                    EmptyState {
                        anchors.centerIn: parent
                        visible: root.taskModel.activeCount === 0
                        theme: root.theme
                    }
                }
            }

            ColumnLayout {
                visible: root.appSettings.showTimeline
                Layout.preferredWidth: visible ? 286 : 0
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Today’s timeline")
                                color: root.theme.accent
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: qsTr("%1 items").arg(root.taskModel.activeCount)
                                color: root.theme.textMuted
                            }
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 5
                            clip: true
                            model: root.taskModel

                            delegate: Rectangle {
                                id: timelineRow

                                required property int index
                                required property string title
                                required property int estimatedMinutes

                                width: ListView.view.width
                                height: 48
                                radius: root.theme.radius
                                color: index === 0 ? root.theme.accentSoft : root.theme.surfaceRaised
                                border.color: index === 0 ? root.theme.accent : root.theme.border

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    spacing: 9

                                    Text {
                                        text: root.formattedHour(9 + timelineRow.index)
                                        color: root.theme.textMuted
                                        font.pixelSize: 9
                                    }
                                    Rectangle {
                                        implicitWidth: 5
                                        implicitHeight: 5
                                        radius: 3
                                        color: timelineRow.index === 0
                                            ? root.theme.accent
                                            : root.theme.borderStrong
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 1
                                        Text {
                                            Layout.fillWidth: true
                                            text: timelineRow.title
                                            color: root.theme.textPrimary
                                            elide: Text.ElideRight
                                            font.pixelSize: 10
                                            font.weight: Font.Medium
                                        }
                                        Text {
                                            text: qsTr("%1 min").arg(timelineRow.estimatedMinutes)
                                            color: root.theme.accent
                                            font.pixelSize: 11
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 124
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("WORKLOAD")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: root.taskModel.plannedDuration
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 7
                            radius: 4
                            color: root.theme.surfaceRaised

                            Rectangle {
                                width: Math.min(
                                    parent.width,
                                    parent.width * root.taskModel.totalEstimatedMinutes
                                        / root.appSettings.dailyCapacityMinutes)
                                height: parent.height
                                radius: parent.radius
                                color: root.theme.accent
                            }
                        }

                        Text {
                            text: root.taskModel.totalEstimatedMinutes
                                    <= root.appSettings.dailyCapacityMinutes
                                ? qsTr("Within your daily capacity")
                                : qsTr("Today is over capacity")
                            color: root.taskModel.totalEstimatedMinutes
                                    <= root.appSettings.dailyCapacityMinutes
                                ? root.theme.success
                                : root.theme.danger
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }
    }
}
