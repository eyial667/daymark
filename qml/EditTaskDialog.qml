// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    required property var taskModel
    required property var theme
    property string taskId: ""
    property bool isInToday: false

    function openForTask(id) {
        const details = dialog.taskModel.taskDetails(id)
        if (!details || details.taskId === undefined)
            return
        dialog.taskId = details.taskId
        dialog.isInToday = details.isInToday
        titleField.text = details.title
        notesField.text = details.notes
        dueField.text = details.dueDate
        importanceField.currentIndex = details.importance - 1
        estimateField.value = details.estimatedMinutes
        dialog.open()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 550
    modal: true
    focus: true
    padding: 24
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        radius: dialog.theme.radius + 4
        color: dialog.theme.surfaceRaised
        border.color: dialog.theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 15

        Text {
            text: qsTr("Edit task")
            color: dialog.theme.textPrimary
            font.pixelSize: 21
            font.weight: Font.DemiBold
        }

        Text {
            // preferredWidth 0 keeps the translated text from widening the
            // fixed-width dialog instead of wrapping inside it.
            Layout.fillWidth: true
            Layout.preferredWidth: 0
            text: dialog.isInToday
                ? qsTr("This task is in today’s plan. Changes reorder it immediately.")
                : qsTr("Changes reorder the queue immediately. The task keeps its age and category.")
            color: dialog.theme.textSecondary
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            placeholderText: qsTr("What needs to be done?")
            selectByMouse: true
            onAccepted: saveButton.clicked()
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Text {
                text: qsTr("Notes")
                color: dialog.theme.textSecondary
                font.pixelSize: 11
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 92
                radius: dialog.theme.radius
                color: dialog.theme.surface
                border.color: notesField.activeFocus
                    ? dialog.theme.accent : dialog.theme.border

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true

                    TextArea {
                        id: notesField
                        placeholderText: qsTr("Context, links, or the next concrete step")
                        color: dialog.theme.textPrimary
                        placeholderTextColor: dialog.theme.textMuted
                        font.pixelSize: 12
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        background: null
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: 0
            visible: dialog.taskModel.statusMessage.length > 0
            text: dialog.taskModel.statusMessage
            color: dialog.theme.danger
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: qsTr("Due date")
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                DateField {
                    id: dueField
                    Layout.fillWidth: true
                    theme: dialog.theme
                    placeholderText: qsTr("YYYY-MM-DD")
                    onAccepted: saveButton.clicked()
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 120
                spacing: 5

                Text {
                    text: qsTr("Importance")
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                ComboBox {
                    id: importanceField
                    Layout.fillWidth: true
                    model: [1, 2, 3, 4, 5]
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 150
                spacing: 5

                Text {
                    text: qsTr("Estimate")
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                SpinBox {
                    id: estimateField
                    Layout.fillWidth: true
                    from: 5
                    to: 480
                    stepSize: 5
                    value: 30
                    editable: true
                    font.pixelSize: 14
                    textFromValue: (value, locale) => qsTr("%1 min").arg(value)
                    valueFromText: (text, locale) => {
                        const parsed = parseInt(text)
                        return Number.isNaN(parsed) ? estimateField.value : parsed
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: 0
            text: qsTr("Clear the due date to remove the deadline. Use the row menu to postpone by a set number of days.")
            color: dialog.theme.textMuted
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 10

            Item { Layout.fillWidth: true }

            AppButton {
                theme: dialog.theme
                text: qsTr("Cancel")
                quiet: true
                onClicked: dialog.close()
            }

            AppButton {
                id: saveButton
                theme: dialog.theme
                text: qsTr("Save changes")
                primary: true
                enabled: titleField.text.trim().length > 0
                onClicked: {
                    if (dialog.taskModel.updateTask(
                            dialog.taskId,
                            titleField.text,
                            dueField.text,
                            importanceField.currentValue,
                            estimateField.value,
                            notesField.text)) {
                        dialog.close()
                    }
                }
            }
        }
    }

    onOpened: {
        dialog.taskModel.clearStatus()
        titleField.forceActiveFocus()
        titleField.selectAll()
    }
    onClosed: {
        dialog.taskId = ""
        dialog.isInToday = false
        titleField.clear()
        notesField.clear()
        dueField.clear()
    }
}
