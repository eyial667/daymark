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
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 2
                Text {
                    text: "Today"
                    color: root.theme.textPrimary
                    font.pixelSize: 25
                    font.weight: Font.DemiBold
                }
                Text {
                    text: Qt.formatDate(new Date(), "dddd, MMMM d") + "  ·  " + root.theme.modeName
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: root.taskModel.totalEstimatedMinutes
                        <= root.appSettings.dailyCapacityMinutes
                    ? "●  Day is realistic"
                    : "●  Over capacity"
                color: root.taskModel.totalEstimatedMinutes
                        <= root.appSettings.dailyCapacityMinutes
                    ? root.theme.success
                    : root.theme.danger
                font.pixelSize: 11
            }
            AppButton {
                theme: root.theme
                primary: true
                text: "+ Add task"
                onClicked: root.addRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 116
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.borderStrong

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 18

                Rectangle {
                    implicitWidth: 4
                    Layout.fillHeight: true
                    radius: 2
                    color: root.theme.accent
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: "NEXT ACTION"
                        color: root.theme.accent
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.2
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.taskModel.topTaskTitle
                        color: root.theme.textPrimary
                        elide: Text.ElideRight
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Text {
                        visible: root.taskModel.activeCount > 0
                        text: "Top-ranked by deadline, importance, age, and focus fit"
                        color: root.theme.textSecondary
                        font.pixelSize: 10
                    }
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: "Focus  →"
                    enabled: root.taskModel.activeCount > 0
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 9

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "Priority queue"
                        color: root.theme.textPrimary
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: root.taskModel.activeCount + " tasks  ·  " + root.taskModel.plannedDuration
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

                        delegate: TaskRow {
                            id: taskRow
                            width: ListView.view.width
                            theme: root.theme
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
            }

            ColumnLayout {
                Layout.preferredWidth: 322
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 216
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: "Goal path"
                                color: root.theme.secondaryAccent
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: "Open map  ↗"
                                color: root.theme.textMuted
                                font.pixelSize: 9
                            }
                        }

                        Repeater {
                            model: ["Work", "Product launch", "Website", root.taskModel.topTaskTitle]

                            delegate: RowLayout {
                                id: pathRow
                                required property int index
                                required property string modelData
                                Layout.fillWidth: true
                                spacing: 9

                                Rectangle {
                                    implicitWidth: 9
                                    implicitHeight: 9
                                    radius: 5
                                    color: pathRow.index === 3
                                        ? root.theme.accent
                                        : root.theme.secondaryAccent
                                }
                                Rectangle {
                                    visible: pathRow.index > 0
                                    implicitWidth: 17
                                    implicitHeight: 1
                                    color: root.theme.borderStrong
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: pathRow.modelData
                                    color: pathRow.index === 3
                                        ? root.theme.textPrimary
                                        : root.theme.textSecondary
                                    elide: Text.ElideRight
                                    font.pixelSize: 10
                                    font.weight: pathRow.index === 3 ? Font.DemiBold : Font.Normal
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 9

                        Text {
                            text: "Day shape"
                            color: root.theme.accent
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 4
                            clip: true
                            model: root.taskModel

                            delegate: RowLayout {
                                id: shapeRow

                                required property int index
                                required property string title
                                required property int estimatedMinutes

                                width: ListView.view.width
                                height: 34
                                spacing: 8

                                Text {
                                    text: root.formattedHour(9 + shapeRow.index)
                                    color: root.theme.textMuted
                                    font.pixelSize: 9
                                    Layout.preferredWidth: 34
                                }
                                Rectangle {
                                    Layout.fillWidth: true
                                    implicitHeight: 28
                                    radius: root.theme.radius
                                    color: shapeRow.index === 0
                                        ? root.theme.accentSoft
                                        : root.theme.surfaceRaised
                                    border.color: shapeRow.index === 0
                                        ? root.theme.accent
                                        : root.theme.border
                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 9
                                        anchors.rightMargin: 9
                                        text: shapeRow.title + "  ·  " + shapeRow.estimatedMinutes + "m"
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 9
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
