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
    property string initialCategoryId: ""
    property string initialSubcategoryId: ""
    signal manageCategoriesRequested()

    function openForAssignment(categoryId, subcategoryId) {
        initialCategoryId = categoryId
        initialSubcategoryId = subcategoryId
        open()
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
            text: qsTr("Add a task")
            color: dialog.theme.textPrimary
            font.pixelSize: 21
            font.weight: Font.DemiBold
        }

        Text {
            text: qsTr("Capture it now. Daymark will place it in priority order.")
            color: dialog.theme.textSecondary
            font.pixelSize: 12
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            placeholderText: qsTr("What needs to be done?")
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
                    text: qsTr("Category or subcategory")
                    color: dialog.theme.textSecondary
                    font.pixelSize: 11
                }

                ComboBox {
                    id: categoryField
                    Layout.fillWidth: true
                    model: [qsTr("No category")].concat(dialog.categoryModel.assignmentNames)
                    currentIndex: 0
                }
            }

            AppButton {
                Layout.alignment: Qt.AlignBottom
                theme: dialog.theme
                quiet: true
                text: qsTr("Manage")
                onClicked: {
                    dialog.close()
                    dialog.manageCategoriesRequested()
                }
            }
        }

        Text {
            Layout.fillWidth: true
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
                    currentIndex: 2
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

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            CheckBox {
                id: planTodayField
                text: qsTr("Add directly to Today")
                checked: false
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Leave this off for To-do. A task appears in Today automatically when its deadline arrives.")
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
                text: qsTr("Cancel")
                quiet: true
                onClicked: dialog.close()
            }

            AppButton {
                id: saveButton
                theme: dialog.theme
                text: qsTr("Add task")
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
                        dialog.close()
                    }
                }
            }
        }
    }

    onOpened: {
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
        planTodayField.checked = false
        dialog.taskModel.clearStatus()
        const assignmentIndex = dialog.categoryModel.indexOfAssignment(
            dialog.initialCategoryId, dialog.initialSubcategoryId)
        categoryField.currentIndex = assignmentIndex >= 0 ? assignmentIndex + 1 : 0
        titleField.forceActiveFocus()
    }
    onClosed: {
        titleField.clear()
        categoryField.currentIndex = 0
        dueField.clear()
        importanceField.currentIndex = dialog.appSettings.defaultImportance - 1
        estimateField.value = dialog.appSettings.defaultEstimatedMinutes
        planTodayField.checked = false
        dialog.initialCategoryId = ""
        dialog.initialSubcategoryId = ""
    }
}
