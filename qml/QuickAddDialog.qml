// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    required property var taskModel
    required property var appSettings
    required property var theme

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 470
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
            text: "Add a task"
            color: dialog.theme.textPrimary
            font.pixelSize: 21
            font.weight: Font.DemiBold
        }

        Text {
            text: "Capture it now. Daymark will place it in priority order."
            color: dialog.theme.textSecondary
            font.pixelSize: 12
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            placeholderText: "What needs to be done?"
            selectByMouse: true
            onAccepted: saveButton.clicked()
        }

        TextField {
            id: projectField
            Layout.fillWidth: true
            placeholderText: "Project (optional)"
            selectByMouse: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: "Due date"
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                TextField {
                    id: dueField
                    Layout.fillWidth: true
                    placeholderText: "YYYY-MM-DD"
                    selectByMouse: true
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 110
                spacing: 5

                Text {
                    text: "Importance"
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                ComboBox {
                    id: importanceField
                    Layout.fillWidth: true
                    model: [1, 2, 3, 4, 5]
                    currentIndex: 2
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 100
                spacing: 5

                Text {
                    text: "Minutes"
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
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 10

            Item { Layout.fillWidth: true }

            AppButton {
                theme: dialog.theme
                text: "Cancel"
                quiet: true
                onClicked: dialog.close()
            }

            AppButton {
                id: saveButton
                theme: dialog.theme
                text: "Add task"
                primary: true
                enabled: titleField.text.trim().length > 0
                onClicked: {
                    if (dialog.taskModel.addTask(
                            titleField.text,
                            dueField.text,
                            importanceField.currentValue,
                            estimateField.value,
                            projectField.text)) {
                        dialog.close()
                    }
                }
            }
        }
    }

    onOpened: {
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
        titleField.forceActiveFocus()
    }
    onClosed: {
        titleField.clear()
        projectField.clear()
        dueField.clear()
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
    }
}
