// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    required property var taskModel
    required property var categoryModel
    required property var appSettings
    required property var theme
    signal manageCategoriesRequested()

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

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: "Category or subcategory"
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                ComboBox {
                    id: categoryField
                    Layout.fillWidth: true
                    model: ["No category"].concat(dialog.categoryModel.assignmentNames)
                    currentIndex: 0
                }
            }

            AppButton {
                Layout.alignment: Qt.AlignBottom
                theme: dialog.theme
                quiet: true
                text: "Manage"
                onClicked: {
                    dialog.close()
                    dialog.manageCategoriesRequested()
                }
            }
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

                DateField {
                    id: dueField
                    Layout.fillWidth: true
                    theme: dialog.theme
                    placeholderText: "YYYY-MM-DD"
                    onAccepted: saveButton.clicked()
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 120
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
                Layout.preferredWidth: 150
                spacing: 5

                Text {
                    text: "Estimate"
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
                    textFromValue: (value, locale) => value + " min"
                    valueFromText: (text, locale) => {
                        const parsed = parseInt(text)
                        return Number.isNaN(parsed) ? estimateField.value : parsed
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            CheckBox {
                id: planTodayField
                text: "Add to Today"
                checked: true
            }
            Text {
                Layout.fillWidth: true
                text: "Turn this off to keep the task in To-do until you choose it."
                color: dialog.theme.textMuted
                font.pixelSize: 10
                wrapMode: Text.WordWrap
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
                            categoryField.currentIndex > 0
                                ? dialog.categoryModel.categoryIdForAssignment(
                                    categoryField.currentIndex - 1)
                                : "",
                            categoryField.currentIndex > 0
                                ? dialog.categoryModel.subcategoryIdForAssignment(
                                    categoryField.currentIndex - 1)
                                : "",
                            planTodayField.checked)) {
                        dialog.categoryModel.reload()
                        dialog.close()
                    }
                }
            }
        }
    }

    onOpened: {
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
        planTodayField.checked = true
        titleField.forceActiveFocus()
    }
    onClosed: {
        titleField.clear()
        categoryField.currentIndex = 0
        dueField.clear()
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
        planTodayField.checked = true
    }
}
