// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var categoryModel
    required property var theme
    property int editingIndex: -1

    function startNewCategory(clearStatus) {
        editingIndex = -1
        categoryList.currentIndex = -1
        nameField.clear()
        notesField.clear()
        if (clearStatus === undefined || clearStatus)
            categoryModel.clearStatus()
        nameField.forceActiveFocus()
    }

    function startEditing(index, name, notes) {
        editingIndex = index
        categoryList.currentIndex = index
        nameField.text = name
        notesField.text = notes
        categoryModel.clearStatus()
        nameField.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 3
                Text {
                    text: "Categories"
                    color: root.theme.textPrimary
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "Group related tasks and keep useful context in category notes."
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: root.categoryModel.categoryCount + (root.categoryModel.categoryCount === 1
                    ? " category" : " categories")
                color: root.theme.textMuted
                font.pixelSize: 11
            }
            AppButton {
                theme: root.theme
                primary: true
                text: "+ New category"
                onClicked: root.startNewCategory()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 390
                Layout.fillHeight: true
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                Item {
                    anchors.fill: parent
                    anchors.margins: 12

                    ListView {
                        id: categoryList
                        anchors.fill: parent
                        spacing: 7
                        clip: true
                        model: root.categoryModel
                        currentIndex: -1

                        delegate: Rectangle {
                            id: categoryRow

                            required property int index
                            required property string name
                            required property string notes
                            required property int taskCount

                            width: ListView.view.width
                            height: 82
                            radius: root.theme.radius
                            color: categoryList.currentIndex === categoryRow.index
                                ? root.theme.accentSoft
                                : categoryHover.hovered ? root.theme.surfaceHover : root.theme.surfaceRaised
                            border.color: categoryList.currentIndex === categoryRow.index
                                ? root.theme.accent : root.theme.border

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 5

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        Layout.fillWidth: true
                                        text: categoryRow.name
                                        color: root.theme.textPrimary
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        text: categoryRow.taskCount + (categoryRow.taskCount === 1
                                            ? " task" : " tasks")
                                        color: root.theme.accent
                                        font.pixelSize: 10
                                        font.weight: Font.DemiBold
                                    }
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: categoryRow.notes.length > 0
                                        ? categoryRow.notes : "No notes yet"
                                    color: categoryRow.notes.length > 0
                                        ? root.theme.textSecondary : root.theme.textMuted
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }
                            }

                            HoverHandler { id: categoryHover }
                            TapHandler {
                                onTapped: root.startEditing(
                                    categoryRow.index,
                                    categoryRow.name,
                                    categoryRow.notes)
                            }
                        }
                    }

                    ColumnLayout {
                        anchors.centerIn: parent
                        visible: root.categoryModel.categoryCount === 0
                        width: Math.min(280, parent.width - 32)
                        spacing: 9
                        Text {
                            Layout.fillWidth: true
                            text: "No categories yet"
                            color: root.theme.textPrimary
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "Create one to group tasks by area, context, or responsibility."
                            color: root.theme.textSecondary
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
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
                    anchors.margins: 22
                    spacing: 13

                    Text {
                        text: root.editingIndex >= 0 ? "Edit category" : "Create a category"
                        color: root.theme.textPrimary
                        font.pixelSize: 19
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.editingIndex >= 0
                            ? "Changes appear on every assigned task."
                            : "Names stay short; notes can hold definitions, links, or working context."
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: "NAME"
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    TextField {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: "For example: Health, Finance, Deep work"
                        maximumLength: 120
                        selectByMouse: true
                    }

                    Text {
                        text: "NOTES"
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 180
                        clip: true

                        TextArea {
                            id: notesField
                            placeholderText: "What belongs here? Add context that will help later…"
                            wrapMode: TextEdit.Wrap
                            selectByMouse: true
                            color: root.theme.textPrimary
                            placeholderTextColor: root.theme.textMuted
                            background: Rectangle {
                                color: root.theme.surfaceRaised
                                border.color: notesField.activeFocus
                                    ? root.theme.accent : root.theme.border
                                radius: root.theme.radius
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: root.categoryModel.statusMessage.length > 0
                        text: root.categoryModel.statusMessage
                        color: root.categoryModel.statusMessage.indexOf("Could not") === 0
                            || root.categoryModel.statusMessage.indexOf("needs") >= 0
                            || root.categoryModel.statusMessage.indexOf("already") >= 0
                            ? root.theme.danger : root.theme.success
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.fillWidth: true }
                        AppButton {
                            visible: root.editingIndex >= 0
                            theme: root.theme
                            quiet: true
                            text: "New category"
                            onClicked: root.startNewCategory()
                        }
                        AppButton {
                            theme: root.theme
                            primary: true
                            text: root.editingIndex >= 0 ? "Save changes" : "Create category"
                            enabled: nameField.text.trim().length > 0
                            onClicked: {
                                const saved = root.editingIndex >= 0
                                    ? root.categoryModel.updateCategory(
                                        root.editingIndex, nameField.text, notesField.text)
                                    : root.categoryModel.addCategory(nameField.text, notesField.text)
                                if (saved)
                                    root.startNewCategory(false)
                            }
                        }
                    }
                }
            }
        }
    }
}
