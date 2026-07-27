// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var taskModel
    required property var categoryModel
    required property var appSettings
    required property var theme
    property bool showSuggestionAction: false
    property bool selectedAll: true
    property string selectedCategoryId: ""
    property string selectedSubcategoryId: ""
    property string selectedName: "All tasks"
    property string selectedNotes: "Every task in this queue."
    readonly property var visibleTasks: {
        const currentRevision = root.taskModel.revision
        return root.taskModel.tasksForAssignment(
            root.selectedCategoryId,
            root.selectedSubcategoryId,
            root.selectedAll)
    }
    signal addRequested()
    signal addInCategoryRequested(string categoryId, string subcategoryId)
    signal manageCategoriesRequested()
    signal suggestionRequested()
    signal completionRequested(int taskIndex, string taskTitle)
    signal categoryRequested(
        string taskId,
        string taskTitle,
        string categoryId,
        string subcategoryId)

    function selectArea(categoryId, subcategoryId, name, notes, allTasks) {
        selectedCategoryId = categoryId
        selectedSubcategoryId = subcategoryId
        selectedName = name
        selectedNotes = notes.length > 0
            ? notes : "No notes for this work area yet."
        selectedAll = allTasks
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.width < 720 ? 14 : 22
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Work map"
                    color: root.theme.textPrimary
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                }
                Text {
                    visible: root.width >= 700
                    text: "Browse real categories and subcategories, then act on their tasks."
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }
            AppButton {
                visible: root.showSuggestionAction && root.width >= 620
                theme: root.theme
                primary: true
                text: "Give me something to do"
                onClicked: root.suggestionRequested()
            }
            AppButton {
                theme: root.theme
                text: "+ Add task"
                onClicked: root.addRequested()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.width < 760 ? 1 : 2
            columnSpacing: 12
            rowSpacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: root.width >= 760
                Layout.preferredWidth: root.width >= 760 ? 290 : -1
                Layout.preferredHeight: root.width < 760 ? 210 : -1
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 7

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: "WORK AREAS"
                            color: root.theme.secondaryAccent
                            font.pixelSize: 10
                            font.weight: Font.Bold
                            font.letterSpacing: 1.1
                        }
                        AppButton {
                            theme: root.theme
                            quiet: true
                            text: "Manage"
                            onClicked: root.manageCategoriesRequested()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        AppButton {
                            Layout.fillWidth: true
                            theme: root.theme
                            primary: root.selectedAll
                            text: "All tasks"
                            onClicked: root.selectArea(
                                "", "", "All tasks", "Every task in this queue.", true)
                        }
                        AppButton {
                            Layout.fillWidth: true
                            theme: root.theme
                            primary: !root.selectedAll
                                && root.selectedCategoryId.length === 0
                            text: "Uncategorized"
                            onClicked: root.selectArea(
                                "", "", "Uncategorized", "Tasks without a work area.", false)
                        }
                    }

                    ListView {
                        id: areaList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 7
                        clip: true
                        model: root.categoryModel

                        delegate: ColumnLayout {
                            id: categoryNode

                            required property string categoryId
                            required property string name
                            required property string notes
                            required property int taskCount
                            required property var subcategories

                            width: ListView.view.width
                            spacing: 4

                            AppButton {
                                Layout.fillWidth: true
                                theme: root.theme
                                primary: !root.selectedAll
                                    && root.selectedCategoryId === categoryNode.categoryId
                                    && root.selectedSubcategoryId.length === 0
                                text: categoryNode.name + "  ·  " + categoryNode.taskCount
                                onClicked: root.selectArea(
                                    categoryNode.categoryId,
                                    "",
                                    categoryNode.name,
                                    categoryNode.notes,
                                    false)
                            }

                            Repeater {
                                model: categoryNode.subcategories

                                delegate: AppButton {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 18
                                    theme: root.theme
                                    quiet: root.selectedSubcategoryId !== modelData.subcategoryId
                                    primary: root.selectedSubcategoryId === modelData.subcategoryId
                                    text: "↳ " + modelData.name + "  ·  " + modelData.taskCount
                                    onClicked: root.selectArea(
                                        categoryNode.categoryId,
                                        modelData.subcategoryId,
                                        categoryNode.name + " / " + modelData.name,
                                        modelData.notes,
                                        false)
                                }
                            }
                        }
                    }

                    Text {
                        visible: root.categoryModel.categoryCount === 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "No categories yet. Use Manage to create one."
                        color: root.theme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
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
                    anchors.margins: 12
                    spacing: 9

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                Layout.fillWidth: true
                                text: root.selectedName
                                color: root.theme.textPrimary
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: root.selectedNotes
                                color: root.theme.textMuted
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                        Rectangle {
                            implicitWidth: taskCountText.implicitWidth + 18
                            implicitHeight: 28
                            radius: 14
                            color: root.theme.accentSoft
                            Text {
                                id: taskCountText
                                anchors.centerIn: parent
                                text: root.visibleTasks.length + " tasks"
                                color: root.theme.accent
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                            }
                        }
                        AppButton {
                            visible: !root.selectedAll
                                && root.selectedCategoryId.length > 0
                            theme: root.theme
                            text: "+ Add here"
                            onClicked: root.addInCategoryRequested(
                                root.selectedCategoryId,
                                root.selectedSubcategoryId)
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6
                        clip: true
                        model: root.visibleTasks

                        delegate: Item {
                            id: mappedTask

                            required property int index
                            required property var modelData
                            width: ListView.view.width
                            height: taskRow.implicitHeight

                            TaskRow {
                                id: taskRow
                                anchors.fill: parent
                                theme: root.theme
                                index: mappedTask.index
                                taskIndex: mappedTask.modelData.sourceRow
                                taskId: mappedTask.modelData.taskId
                                title: mappedTask.modelData.title
                                categoryId: mappedTask.modelData.categoryId
                                categoryName: mappedTask.modelData.categoryName
                                subcategoryId: mappedTask.modelData.subcategoryId
                                subcategoryName: mappedTask.modelData.subcategoryName
                                dueText: mappedTask.modelData.dueText
                                estimatedMinutes: mappedTask.modelData.estimatedMinutes
                                priorityScore: mappedTask.modelData.priorityScore
                                priorityReason: mappedTask.modelData.priorityReason
                                compact: true
                                showReason: root.appSettings.showPriorityReasons
                                onCompletionRequested: rowIndex =>
                                    root.completionRequested(rowIndex, taskRow.title)
                                onCategoryRequested: (taskId, categoryId, subcategoryId) =>
                                    root.categoryRequested(
                                        taskId,
                                        taskRow.title,
                                        categoryId,
                                        subcategoryId)
                            }
                        }
                    }

                    Text {
                        visible: root.visibleTasks.length === 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "No tasks in this area."
                        color: root.theme.textMuted
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
