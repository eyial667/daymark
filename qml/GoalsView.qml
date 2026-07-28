// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var goalModel
    required property var theme

    property int selectedGoalRow: -1
    property string selectedGoalTitle: ""
    property string pendingDeleteType: ""
    property int pendingDeleteMilestoneIndex: -1
    property string pendingDeleteName: ""

    function openMilestoneDialog(goalRow, goalTitle) {
        selectedGoalRow = goalRow
        selectedGoalTitle = goalTitle
        milestoneDialog.open()
    }

    function openCompletionDialog(goalRow, goalTitle) {
        selectedGoalRow = goalRow
        selectedGoalTitle = goalTitle
        completionDialog.open()
    }

    function requestDelete(type, goalRow, milestoneIndex, name) {
        pendingDeleteType = type
        selectedGoalRow = goalRow
        pendingDeleteMilestoneIndex = milestoneIndex
        pendingDeleteName = name
        deleteDialog.open()
    }

    ScrollView {
        id: goalsScroll

        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Item {
            width: goalsScroll.availableWidth
            implicitHeight: pageColumn.implicitHeight + 64

            ColumnLayout {
                id: pageColumn

                width: Math.min(980, parent.width - 56)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 28
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 18

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTr("Long-term goals")
                            color: root.theme.textPrimary
                            font.pixelSize: 28
                            font.weight: Font.DemiBold
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Turn distant outcomes into smaller milestones with clear target dates.")
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }

                    AppButton {
                        theme: root.theme
                        primary: true
                        text: qsTr("+  Add goal")
                        onClicked: goalDialog.open()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    MetricCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        theme: root.theme
                        label: qsTr("ACTIVE GOALS")
                        value: root.goalModel.goalCount.toString()
                    }

                    MetricCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        theme: root.theme
                        label: qsTr("MILESTONES")
                        value: root.goalModel.totalMilestoneCount.toString()
                        accent: root.theme.secondaryAccent
                    }

                    MetricCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        theme: root.theme
                        label: qsTr("COMPLETED STEPS")
                        value: root.goalModel.completedMilestoneCount.toString()
                        accent: root.theme.success
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: statusText.implicitHeight + 22
                    visible: root.goalModel.statusMessage.length > 0
                    radius: root.theme.radius
                    color: root.theme.accentSoft
                    border.color: root.theme.borderStrong

                    Text {
                        id: statusText
                        anchors.fill: parent
                        anchors.margins: 11
                        text: root.goalModel.statusMessage
                        color: root.theme.textPrimary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.goalModel.clearStatus()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 210
                    visible: root.goalModel.goalCount === 0
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.centerIn: parent
                        width: Math.min(480, parent.width - 48)
                        spacing: 9

                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            implicitWidth: 48
                            implicitHeight: 48
                            radius: 24
                            color: root.theme.accentSoft
                            border.color: root.theme.borderStrong

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("◇")
                                color: root.theme.accent
                                font.pixelSize: 22
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Choose an outcome worth working toward")
                            color: root.theme.textPrimary
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Add a goal, then break it into milestones small enough to see and finish.")
                            color: root.theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }

                Repeater {
                    model: root.goalModel

                    delegate: Rectangle {
                        id: goalCard

                        required property int index
                        required property string title
                        required property string description
                        required property string targetText
                        required property real progress
                        required property string progressText
                        required property var milestones

                        Layout.fillWidth: true
                        implicitHeight: goalContent.implicitHeight + 32
                        radius: root.theme.radius + 2
                        color: root.theme.surface
                        border.color: root.theme.border

                        ColumnLayout {
                            id: goalContent

                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        Layout.fillWidth: true
                                        text: goalCard.title
                                        color: root.theme.textPrimary
                                        font.pixelSize: 18
                                        font.weight: Font.DemiBold
                                        wrapMode: Text.WordWrap
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        visible: goalCard.description.length > 0
                                        text: goalCard.description
                                        color: root.theme.textSecondary
                                        font.pixelSize: 11
                                        lineHeight: 1.25
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                Rectangle {
                                    implicitWidth: targetLabel.implicitWidth + 20
                                    implicitHeight: 30
                                    radius: 15
                                    color: root.theme.surfaceRaised
                                    border.color: root.theme.borderStrong

                                    Text {
                                        id: targetLabel
                                        anchors.centerIn: parent
                                        text: goalCard.targetText
                                        color: goalCard.targetText.startsWith(qsTr("Overdue"))
                                            ? root.theme.danger
                                            : root.theme.textSecondary
                                        font.pixelSize: 10
                                        font.weight: Font.Medium
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Rectangle {
                                    Layout.fillWidth: true
                                    implicitHeight: 7
                                    radius: 4
                                    color: root.theme.surfaceHover

                                    Rectangle {
                                        width: parent.width * goalCard.progress
                                        height: parent.height
                                        radius: parent.radius
                                        color: root.theme.accent

                                        Behavior on width {
                                            NumberAnimation { duration: 160 }
                                        }
                                    }
                                }

                                Text {
                                    text: goalCard.progressText
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 1
                                color: root.theme.border
                            }

                            Text {
                                text: qsTr("MILESTONES")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1.1
                            }

                            Text {
                                Layout.fillWidth: true
                                visible: goalCard.milestones.length === 0
                                text: qsTr("No milestones yet. Add the first smaller checkpoint.")
                                color: root.theme.textMuted
                                font.pixelSize: 11
                                font.italic: true
                            }

                            Repeater {
                                model: goalCard.milestones

                                delegate: Rectangle {
                                    id: milestoneRow

                                    required property int index
                                    required property var modelData

                                    Layout.fillWidth: true
                                    implicitHeight: 44
                                    radius: root.theme.radius
                                    color: root.theme.surfaceRaised
                                    border.color: root.theme.border

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 10
                                        anchors.rightMargin: 12
                                        spacing: 10

                                        CheckBox {
                                            id: milestoneCheck

                                            checked: Boolean(milestoneRow.modelData.completed)
                                            text: ""
                                            onToggled: root.goalModel.setMilestoneCompleted(
                                                goalCard.index,
                                                milestoneRow.index,
                                                checked)
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: milestoneRow.modelData.title
                                            color: milestoneCheck.checked
                                                ? root.theme.textMuted
                                                : root.theme.textPrimary
                                            font.pixelSize: 12
                                            font.strikeout: milestoneCheck.checked
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            text: milestoneRow.modelData.targetText
                                            color: milestoneRow.modelData.targetText.startsWith(qsTr("Overdue"))
                                                && !milestoneCheck.checked
                                                ? root.theme.danger
                                                : root.theme.textMuted
                                            font.pixelSize: 10
                                        }
                                        AppButton {
                                            theme: root.theme
                                            quiet: true
                                            text: "×"
                                            ToolTip.visible: hovered
                                            ToolTip.text: qsTr("Delete milestone")
                                            onClicked: root.requestDelete(
                                                "milestone",
                                                goalCard.index,
                                                milestoneRow.index,
                                                milestoneRow.modelData.title)
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 2
                                spacing: 8

                                AppButton {
                                    theme: root.theme
                                    text: qsTr("+  Add milestone")
                                    onClicked: root.openMilestoneDialog(
                                        goalCard.index,
                                        goalCard.title)
                                }

                                Item { Layout.fillWidth: true }

                                AppButton {
                                    theme: root.theme
                                    quiet: true
                                    text: qsTr("Mark goal achieved")
                                    onClicked: root.openCompletionDialog(
                                        goalCard.index,
                                        goalCard.title)
                                }
                                AppButton {
                                    theme: root.theme
                                    quiet: true
                                    text: qsTr("Delete goal…")
                                    onClicked: root.requestDelete(
                                        "goal", goalCard.index, -1, goalCard.title)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: goalDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 500
        modal: true
        focus: true
        padding: 24
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: root.theme.radius + 4
            color: root.theme.surfaceRaised
            border.color: root.theme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 13

            Text {
                text: qsTr("Add a long-term goal")
                color: root.theme.textPrimary
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            TextField {
                id: goalTitleField
                Layout.fillWidth: true
                placeholderText: qsTr("What outcome are you working toward?")
                selectByMouse: true
            }

            TextArea {
                id: goalDescriptionField
                Layout.fillWidth: true
                Layout.preferredHeight: 90
                placeholderText: qsTr("Why does it matter? (optional)")
                wrapMode: TextEdit.Wrap
                selectByMouse: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: qsTr("Target date")
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }

                DateField {
                    id: goalDateField
                    Layout.fillWidth: true
                    theme: root.theme
                    placeholderText: qsTr("YYYY-MM-DD (optional)")
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 5

                Item { Layout.fillWidth: true }

                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: goalDialog.close()
                }

                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Add goal")
                    enabled: goalTitleField.text.trim().length > 0
                    onClicked: {
                        if (root.goalModel.addGoal(
                                goalTitleField.text,
                                goalDescriptionField.text,
                                goalDateField.text)) {
                            goalDialog.close()
                        }
                    }
                }
            }
        }

        onOpened: goalTitleField.forceActiveFocus()
        onClosed: {
            goalTitleField.clear()
            goalDescriptionField.clear()
            goalDateField.clear()
        }
    }

    Dialog {
        id: milestoneDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 470
        modal: true
        focus: true
        padding: 24
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: root.theme.radius + 4
            color: root.theme.surfaceRaised
            border.color: root.theme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 13

            Text {
                text: qsTr("Add a milestone")
                color: root.theme.textPrimary
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: root.selectedGoalTitle
                color: root.theme.textSecondary
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            TextField {
                id: milestoneTitleField
                Layout.fillWidth: true
                placeholderText: qsTr("What smaller checkpoint comes next?")
                selectByMouse: true
            }

            DateField {
                id: milestoneDateField
                Layout.fillWidth: true
                theme: root.theme
                placeholderText: qsTr("Target date · YYYY-MM-DD (optional)")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 5

                Item { Layout.fillWidth: true }

                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: milestoneDialog.close()
                }

                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Add milestone")
                    enabled: milestoneTitleField.text.trim().length > 0
                    onClicked: {
                        if (root.goalModel.addMilestone(
                                root.selectedGoalRow,
                                milestoneTitleField.text,
                                milestoneDateField.text)) {
                            milestoneDialog.close()
                        }
                    }
                }
            }
        }

        onOpened: milestoneTitleField.forceActiveFocus()
        onClosed: {
            milestoneTitleField.clear()
            milestoneDateField.clear()
            root.selectedGoalRow = -1
            root.selectedGoalTitle = ""
        }
    }

    Dialog {
        id: completionDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 430
        modal: true
        focus: true
        padding: 22
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: root.theme.radius + 3
            color: root.theme.surfaceRaised
            border.color: root.theme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 13

            Text {
                text: qsTr("Mark this goal achieved?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: root.selectedGoalTitle
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("The goal will leave the active list. It remains in exports and your local history.")
                color: root.theme.textMuted
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: completionDialog.close()
                }

                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Mark achieved")
                    onClicked: {
                        root.goalModel.completeGoal(root.selectedGoalRow)
                        completionDialog.close()
                    }
                }
            }
        }

        onClosed: {
            root.selectedGoalRow = -1
            root.selectedGoalTitle = ""
        }
    }

    Dialog {
        id: deleteDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 440
        modal: true
        focus: true
        padding: 22
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: root.theme.radius + 3
            color: root.theme.surfaceRaised
            border.color: root.theme.danger
        }

        contentItem: ColumnLayout {
            spacing: 13
            Text {
                text: root.pendingDeleteType === "goal"
                    ? qsTr("Delete this goal?") : qsTr("Delete this milestone?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: root.pendingDeleteType === "goal"
                    ? qsTr("“%1” and every milestone beneath it will be permanently deleted.")
                        .arg(root.pendingDeleteName)
                    : qsTr("“%1” will be permanently deleted.").arg(root.pendingDeleteName)
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: deleteDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Delete")
                    onClicked: {
                        const removed = root.pendingDeleteType === "goal"
                            ? root.goalModel.deleteGoal(root.selectedGoalRow)
                            : root.goalModel.deleteMilestone(
                                root.selectedGoalRow,
                                root.pendingDeleteMilestoneIndex)
                        if (removed)
                            deleteDialog.close()
                    }
                }
            }
        }

        onClosed: {
            root.selectedGoalRow = -1
            root.selectedGoalTitle = ""
            root.pendingDeleteType = ""
            root.pendingDeleteMilestoneIndex = -1
            root.pendingDeleteName = ""
        }
    }
}
