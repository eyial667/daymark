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

    function formattedHour(hour) {
        if (appSettings.use24HourClock)
            return String(hour).padStart(2, "0") + ":00"
        const displayHour = hour > 12 ? hour - 12 : hour
        return displayHour + (hour < 12 ? " AM" : " PM")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Text {
            text: Qt.formatDate(new Date(), "dddd, MMMM d").toUpperCase()
            color: root.theme.textSecondary
            font.pixelSize: 11
            font.weight: Font.DemiBold
            font.letterSpacing: 1.8
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 28

            Text {
                text: root.taskModel.totalEstimatedMinutes
                        <= root.appSettings.dailyCapacityMinutes
                    ? "◉  Your day is realistic"
                    : "◉  Your day needs editing"
                color: root.taskModel.totalEstimatedMinutes
                        <= root.appSettings.dailyCapacityMinutes
                    ? root.theme.accent
                    : root.theme.danger
                font.pixelSize: 13
            }
            Rectangle { implicitWidth: 1; implicitHeight: 28; color: root.theme.border }
            Text {
                text: "☆  " + Math.min(3, root.taskModel.activeCount) + " priorities"
                color: root.theme.textPrimary
                font.pixelSize: 13
            }
            Rectangle { implicitWidth: 1; implicitHeight: 28; color: root.theme.border }
            Text {
                text: "◷  " + root.taskModel.plannedDuration + " planned"
                color: root.theme.secondaryAccent
                font.pixelSize: 13
            }
            Item { Layout.fillWidth: true }
            AppButton {
                theme: root.theme
                text: "+ Add task"
                onClicked: root.addRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 96
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.borderStrong

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22
                anchors.rightMargin: 18
                spacing: 20

                Text {
                    text: "Start here"
                    color: root.theme.accent
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "→"
                    color: root.theme.textMuted
                    font.pixelSize: 20
                }
                Text {
                    Layout.fillWidth: true
                    text: root.taskModel.topTaskTitle
                    color: root.theme.textPrimary
                    elide: Text.ElideRight
                    font.pixelSize: 24
                    font.family: "serif"
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: "Start focus  →"
                    enabled: root.taskModel.activeCount > 0
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
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
                    anchors.margins: 12
                    spacing: 9

                    Text {
                        text: "PRIORITY ORDER"
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ListView {
                            anchors.fill: parent
                            spacing: 5
                            clip: true
                            model: root.taskModel

                            delegate: TaskRow {
                                id: taskRow
                                width: ListView.view.width
                                theme: root.theme
                                compact: true
                                showReason: root.appSettings.showPriorityReasons
                                onCompletionRequested: rowIndex =>
                                    root.completionRequested(rowIndex, taskRow.title)
                            }
                        }

                        EmptyState {
                            anchors.centerIn: parent
                            visible: root.taskModel.activeCount === 0
                            theme: root.theme
                        }
                    }

                    AppButton {
                        Layout.fillWidth: true
                        theme: root.theme
                        quiet: true
                        text: "+  Add task"
                        onClicked: root.addRequested()
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 390
                Layout.fillHeight: true
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        text: "Agenda"
                        color: root.theme.textPrimary
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 1
                        clip: true
                        model: root.taskModel

                        delegate: Rectangle {
                            id: agendaRow

                            required property int index
                            required property string title
                            required property int estimatedMinutes

                            width: ListView.view.width
                            height: 48
                            color: agendaRow.index === 0 ? root.theme.accentSoft : "transparent"
                            border.color: agendaRow.index === 0 ? root.theme.accent : root.theme.border
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 11

                                Text {
                                    text: root.formattedHour(9 + agendaRow.index)
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                    Layout.preferredWidth: 48
                                }
                                Rectangle {
                                    implicitWidth: 5
                                    implicitHeight: 5
                                    radius: 3
                                    color: agendaRow.index < 3
                                        ? root.theme.accent
                                        : root.theme.secondaryAccent
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: agendaRow.title
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                    font.pixelSize: 11
                                }
                                Text {
                                    text: agendaRow.estimatedMinutes + "m"
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 72
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 14
                spacing: 14

                Text {
                    text: "☾"
                    color: root.theme.secondaryAccent
                    font.pixelSize: 23
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "Evening review"
                        color: root.theme.secondaryAccent
                        font.pixelSize: 14
                        font.family: "serif"
                    }
                    Text {
                        text: "Reflect, close the day, and plan tomorrow."
                        color: root.theme.textMuted
                        font.pixelSize: 10
                    }
                }
                AppButton {
                    theme: root.theme
                    text: "Begin review  →"
                    enabled: false
                }
            }
        }
    }
}
