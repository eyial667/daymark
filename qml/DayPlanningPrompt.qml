// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var todayTaskModel
    required property var goalModel
    required property var appSettings
    required property var theme
    signal addRequested()

    Rectangle {
        anchors.fill: parent
        color: root.theme.background
        opacity: 0.96

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(760, parent.width - 56)
        height: promptContent.implicitHeight + 44
        radius: root.theme.radius + 5
        color: root.theme.surfaceRaised
        border.color: root.theme.borderStrong
        border.width: 1

        ColumnLayout {
            id: promptContent

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 22
            spacing: 14

            Text {
                text: "OPEN DAY"
                color: root.theme.accent
                font.pixelSize: 10
                font.weight: Font.Bold
                font.letterSpacing: 1.3
            }

            Text {
                Layout.fillWidth: true
                text: "Nothing is planned for today"
                color: root.theme.textPrimary
                font.pixelSize: 25
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: "Bring forward the best item from To-do, or use an unfinished milestone to make progress on a goal."
                color: root.theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 166
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 7

                        Text {
                            text: "FROM TO-DO"
                            color: root.theme.accent
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            font.letterSpacing: 1.1
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.todayTaskModel.hasBacklogSuggestion
                                ? root.todayTaskModel.backlogSuggestionTitle
                                : "No backlog task available"
                            color: root.theme.textPrimary
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: root.todayTaskModel.hasBacklogSuggestion
                                ? root.todayTaskModel.backlogSuggestionDetail
                                : "Capture a task whenever something comes to mind."
                            color: root.theme.textMuted
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                        AppButton {
                            Layout.alignment: Qt.AlignRight
                            theme: root.theme
                            primary: true
                            text: "Add to Today"
                            enabled: root.todayTaskModel.hasBacklogSuggestion
                            onClicked: root.todayTaskModel.planSuggestedTaskForToday()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 166
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 7

                        Text {
                            text: "FROM A GOAL"
                            color: root.theme.secondaryAccent
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            font.letterSpacing: 1.1
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.goalModel.hasMilestoneSuggestion
                                ? root.goalModel.suggestedMilestoneTitle
                                : "No unfinished milestone"
                            color: root.theme.textPrimary
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: root.goalModel.hasMilestoneSuggestion
                                ? root.goalModel.suggestedMilestoneGoal + " · "
                                    + root.goalModel.suggestedMilestoneTarget
                                : "Add a milestone to a goal to get a proposal here."
                            color: root.theme.textMuted
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                        AppButton {
                            Layout.alignment: Qt.AlignRight
                            theme: root.theme
                            text: "Work on milestone"
                            enabled: root.goalModel.hasMilestoneSuggestion
                            onClicked: root.goalModel.planSuggestedMilestone(
                                root.appSettings.defaultImportance,
                                root.appSettings.defaultEstimatedMinutes)
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: "Suggestions only change your plan after you choose one."
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: "+ Add something else"
                    onClicked: root.addRequested()
                }
            }
        }
    }
}
