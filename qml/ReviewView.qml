// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var todayTaskModel
    required property var dailyNoteModel
    required property var appSettings
    required property var theme
    signal backRequested()
    signal completionRequested(int taskIndex, string taskTitle)
    signal categoryRequested(
        string taskId,
        string taskTitle,
        string categoryId,
        string subcategoryId)

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
                    text: "Daily review"
                    color: root.theme.textPrimary
                    font.pixelSize: 27
                    font.weight: Font.DemiBold
                }
                Text {
                    text: Qt.formatDate(new Date(), "dddd, MMMM d")
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }
            }
            AppButton {
                theme: root.theme
                text: "Refresh"
                onClicked: {
                    root.todayTaskModel.reload()
                    root.dailyNoteModel.refreshDay()
                }
            }
            AppButton {
                theme: root.theme
                primary: true
                text: "Back to Today"
                onClicked: root.backRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 142
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.dailyNoteModel.hasPreviousNote
                ? root.theme.secondaryAccent : root.theme.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: "YESTERDAY’S NOTE"
                        color: root.theme.secondaryAccent
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    Text {
                        text: root.dailyNoteModel.previousDayLabel
                        color: root.theme.textMuted
                        font.pixelSize: 10
                    }
                }

                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: width
                    contentHeight: previousNoteText.implicitHeight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    Text {
                        id: previousNoteText

                        width: parent.width
                        text: root.dailyNoteModel.hasPreviousNote
                            ? root.dailyNoteModel.previousText
                            : "No note was left yesterday. Today’s dashboard note will appear here tomorrow."
                        color: root.dailyNoteModel.hasPreviousNote
                            ? root.theme.textPrimary : root.theme.textMuted
                        font.pixelSize: 12
                        lineHeight: 1.35
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.width < 760 ? 1 : 2
            columnSpacing: 12
            rowSpacing: 12

            MetricCard {
                Layout.fillWidth: true
                theme: root.theme
                label: "COMPLETED TODAY"
                value: root.todayTaskModel.completedTodayCount
                accent: root.theme.success
            }
            MetricCard {
                Layout.fillWidth: true
                theme: root.theme
                label: "STILL ON TODAY"
                value: root.todayTaskModel.activeCount
                accent: root.theme.accent
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.width < 820 ? 1 : 2
            columnSpacing: 12
            rowSpacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: root.width < 820 ? 180 : -1
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    Text {
                        text: "DONE"
                        color: root.theme.success
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6
                        clip: true
                        model: root.todayTaskModel.completedTodayTitles

                        delegate: Rectangle {
                            required property string modelData
                            width: ListView.view.width
                            height: 40
                            radius: root.theme.radius
                            color: root.theme.successSoft
                            border.color: root.theme.border
                            Text {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                text: "✓  " + parent.modelData
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                    Text {
                        visible: root.todayTaskModel.completedTodayCount === 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Nothing completed yet today."
                        color: root.theme.textMuted
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
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
                    spacing: 10

                    Text {
                        text: "REMAINING"
                        color: root.theme.accent
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6
                        clip: true
                        model: root.todayTaskModel

                        delegate: TaskRow {
                            id: reviewTask
                            width: ListView.view.width
                            theme: root.theme
                            compact: true
                            showReason: root.appSettings.showPriorityReasons
                            onCompletionRequested: rowIndex =>
                                root.completionRequested(rowIndex, reviewTask.title)
                            onCategoryRequested: (taskId, categoryId, subcategoryId) =>
                                root.categoryRequested(
                                    taskId,
                                    reviewTask.title,
                                    categoryId,
                                    subcategoryId)
                        }
                    }
                    Text {
                        visible: root.todayTaskModel.activeCount === 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Today is clear."
                        color: root.theme.success
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
