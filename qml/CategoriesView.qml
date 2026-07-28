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
    property var selectedSubcategories: []
    property int editingSubcategoryIndex: -1
    property string pendingDeleteType: ""
    property string pendingDeleteName: ""
    property int pendingDeleteCategoryIndex: -1
    property int pendingDeleteSubcategoryIndex: -1

    function startNewCategory(clearStatus) {
        editingIndex = -1
        categoryList.currentIndex = -1
        nameField.clear()
        notesField.clear()
        selectedSubcategories = []
        if (clearStatus === undefined || clearStatus)
            categoryModel.clearStatus()
        nameField.forceActiveFocus()
    }

    function startEditing(index, name, notes, subcategories) {
        editingIndex = index
        categoryList.currentIndex = index
        nameField.text = name
        notesField.text = notes
        selectedSubcategories = subcategories
        categoryModel.clearStatus()
        nameField.forceActiveFocus()
    }

    function openSubcategoryEditor(index, name, notes) {
        editingSubcategoryIndex = index
        subcategoryNameField.text = name || ""
        subcategoryNotesField.text = notes || ""
        categoryModel.clearStatus()
        subcategoryDialog.open()
    }

    function requestCategoryDelete() {
        pendingDeleteType = "category"
        pendingDeleteName = nameField.text
        pendingDeleteCategoryIndex = editingIndex
        pendingDeleteSubcategoryIndex = -1
        deleteDialog.open()
    }

    function requestSubcategoryDelete(index, name) {
        pendingDeleteType = "subcategory"
        pendingDeleteName = name
        pendingDeleteCategoryIndex = editingIndex
        pendingDeleteSubcategoryIndex = index
        deleteDialog.open()
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
                    text: qsTr("Categories")
                    color: root.theme.textPrimary
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                }
                Text {
                    text: qsTr("Group related tasks and keep useful context in category notes.")
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: qsTr("%1 categories · %2 subcategories")
                    .arg(root.categoryModel.categoryCount)
                    .arg(root.categoryModel.subcategoryCount)
                color: root.theme.textMuted
                font.pixelSize: 11
            }
            AppButton {
                theme: root.theme
                primary: true
                text: qsTr("+ New category")
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
                            required property int subcategoryCount
                            required property var subcategories

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
                                        text: qsTr("%1 sub · %2 tasks")
                                            .arg(categoryRow.subcategoryCount)
                                            .arg(categoryRow.taskCount)
                                        color: root.theme.accent
                                        font.pixelSize: 10
                                        font.weight: Font.DemiBold
                                    }
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: categoryRow.notes.length > 0
                                        ? categoryRow.notes : qsTr("No notes yet")
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
                                    categoryRow.notes,
                                    categoryRow.subcategories)
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
                            text: qsTr("No categories yet")
                            color: root.theme.textPrimary
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Create one to group tasks by area, context, or responsibility.")
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
                        text: root.editingIndex >= 0
                            ? qsTr("Edit category") : qsTr("Create a category")
                        color: root.theme.textPrimary
                        font.pixelSize: 19
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.editingIndex >= 0
                            ? qsTr("Changes appear on every assigned task.")
                            : qsTr("Names stay short; notes can hold definitions, links, or working context.")
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: qsTr("NAME")
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    TextField {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: qsTr("For example: Health, Finance, Deep work")
                        maximumLength: 120
                        selectByMouse: true
                    }

                    Text {
                        text: qsTr("NOTES")
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1.1
                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.editingIndex >= 0 ? 118 : 260
                        clip: true

                        TextArea {
                            id: notesField
                            placeholderText: qsTr("What belongs here? Add context that will help later…")
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

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editingIndex >= 0

                        Text {
                            text: qsTr("SUBCATEGORIES")
                            color: root.theme.textMuted
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            font.letterSpacing: 1.1
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            theme: root.theme
                            quiet: true
                            text: qsTr("+ Add subcategory")
                            onClicked: root.openSubcategoryEditor(-1, "", "")
                        }
                    }

                    ListView {
                        id: subcategoryList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 120
                        visible: root.editingIndex >= 0
                            && root.selectedSubcategories.length > 0
                        spacing: 6
                        clip: true
                        model: root.selectedSubcategories

                        delegate: Rectangle {
                            id: subcategoryRow

                            required property int index
                            required property var modelData

                            width: ListView.view.width
                            height: 62
                            radius: root.theme.radius
                            color: subcategoryHover.hovered
                                ? root.theme.surfaceHover : root.theme.surfaceRaised
                            border.color: root.theme.border

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 9
                                spacing: 10

                                Rectangle {
                                    implicitWidth: 7
                                    implicitHeight: 7
                                    radius: 4
                                    color: root.theme.secondaryAccent
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 3
                                    Text {
                                        Layout.fillWidth: true
                                        text: subcategoryRow.modelData.name
                                        color: root.theme.textPrimary
                                        font.pixelSize: 12
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: subcategoryRow.modelData.notes.length > 0
                                            ? subcategoryRow.modelData.notes : qsTr("No notes")
                                        color: root.theme.textMuted
                                        font.pixelSize: 10
                                        elide: Text.ElideRight
                                    }
                                }
                                Text {
                                    text: qsTr("%1 tasks").arg(subcategoryRow.modelData.taskCount)
                                    color: root.theme.secondaryAccent
                                    font.pixelSize: 10
                                }
                                AppButton {
                                    theme: root.theme
                                    quiet: true
                                    text: qsTr("Edit")
                                    onClicked: root.openSubcategoryEditor(
                                        subcategoryRow.index,
                                        subcategoryRow.modelData.name,
                                        subcategoryRow.modelData.notes)
                                }
                                AppButton {
                                    theme: root.theme
                                    quiet: true
                                    text: "×"
                                    ToolTip.visible: hovered
                                    ToolTip.text: qsTr("Delete subcategory")
                                    onClicked: root.requestSubcategoryDelete(
                                        subcategoryRow.index, subcategoryRow.modelData.name)
                                }
                            }
                            HoverHandler { id: subcategoryHover }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: root.editingIndex >= 0
                            && root.selectedSubcategories.length === 0
                        text: qsTr("No subcategories yet. Add one to divide this category into smaller contexts.")
                        color: root.theme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: root.categoryModel.statusMessage.length > 0
                        text: root.categoryModel.statusMessage
                        color: root.categoryModel.statusMessage.indexOf(qsTr("Could not")) === 0
                            || root.categoryModel.statusMessage.indexOf(qsTr("needs")) >= 0
                            || root.categoryModel.statusMessage.indexOf(qsTr("already")) >= 0
                            ? root.theme.danger : root.theme.success
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        AppButton {
                            visible: root.editingIndex >= 0
                            theme: root.theme
                            quiet: true
                            text: qsTr("Delete category…")
                            onClicked: root.requestCategoryDelete()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            visible: root.editingIndex >= 0
                            theme: root.theme
                            quiet: true
                            text: qsTr("New category")
                            onClicked: root.startNewCategory()
                        }
                        AppButton {
                            theme: root.theme
                            primary: true
                            text: root.editingIndex >= 0
                                ? qsTr("Save changes") : qsTr("Create category")
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

    Dialog {
        id: subcategoryDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 500
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
            spacing: 12

            Text {
                text: root.editingSubcategoryIndex >= 0
                    ? qsTr("Edit subcategory") : qsTr("Add subcategory")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("This subcategory belongs to %1.").arg(nameField.text)
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            TextField {
                id: subcategoryNameField
                Layout.fillWidth: true
                placeholderText: qsTr("Subcategory name")
                maximumLength: 120
                selectByMouse: true
            }
            TextArea {
                id: subcategoryNotesField
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                placeholderText: qsTr("Notes for this subcategory…")
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                color: root.theme.textPrimary
                placeholderTextColor: root.theme.textMuted
                background: Rectangle {
                    color: root.theme.surface
                    border.color: subcategoryNotesField.activeFocus
                        ? root.theme.accent : root.theme.border
                    radius: root.theme.radius
                }
            }
            Text {
                Layout.fillWidth: true
                visible: root.categoryModel.statusMessage.length > 0
                text: root.categoryModel.statusMessage
                color: root.theme.danger
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
                    onClicked: subcategoryDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: root.editingSubcategoryIndex >= 0
                        ? qsTr("Save changes") : qsTr("Add")
                    enabled: subcategoryNameField.text.trim().length > 0
                    onClicked: {
                        const saved = root.editingSubcategoryIndex >= 0
                            ? root.categoryModel.updateSubcategory(
                                root.editingIndex,
                                root.editingSubcategoryIndex,
                                subcategoryNameField.text,
                                subcategoryNotesField.text)
                            : root.categoryModel.addSubcategory(
                                root.editingIndex,
                                subcategoryNameField.text,
                                subcategoryNotesField.text)
                        if (saved) {
                            root.selectedSubcategories =
                                root.categoryModel.subcategoriesAt(root.editingIndex)
                            subcategoryDialog.close()
                        }
                    }
                }
            }
        }

        onOpened: subcategoryNameField.forceActiveFocus()
        onClosed: {
            root.editingSubcategoryIndex = -1
            subcategoryNameField.clear()
            subcategoryNotesField.clear()
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
            spacing: 12
            Text {
                text: root.pendingDeleteType === "category"
                    ? qsTr("Delete this category?") : qsTr("Delete this subcategory?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: root.pendingDeleteType === "category"
                    ? qsTr("“%1” and its subcategories will be deleted. Existing tasks will remain uncategorized.")
                        .arg(root.pendingDeleteName)
                    : qsTr("“%1” will be deleted. Existing tasks will remain in their parent category.")
                        .arg(root.pendingDeleteName)
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
                        const removed = root.pendingDeleteType === "category"
                            ? root.categoryModel.deleteCategory(
                                root.pendingDeleteCategoryIndex)
                            : root.categoryModel.deleteSubcategory(
                                root.pendingDeleteCategoryIndex,
                                root.pendingDeleteSubcategoryIndex)
                        if (!removed)
                            return
                        if (root.pendingDeleteType === "category") {
                            root.startNewCategory(false)
                        } else {
                            root.selectedSubcategories = root.categoryModel.subcategoriesAt(
                                root.editingIndex)
                        }
                        deleteDialog.close()
                    }
                }
            }
        }

        onClosed: {
            root.pendingDeleteType = ""
            root.pendingDeleteName = ""
            root.pendingDeleteCategoryIndex = -1
            root.pendingDeleteSubcategoryIndex = -1
        }
    }
}
