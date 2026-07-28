// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var mapModel
    required property var theme
    property bool showSuggestionAction: false
    property int mapMode: 0
    property string selectedType: ""
    property string selectedId: ""
    property var selectedObject: null
    property string connectionSourceId: ""
    property var groupChoices: []
    property real pendingWorldX: 0
    property real pendingWorldY: 0
    property string pendingGroupId: ""
    property real pendingNoteX: 20
    property real pendingNoteY: 72
    property string pendingConnectionTargetId: ""
    property string pendingDeleteType: ""
    property string pendingDeleteId: ""
    property string pendingDeleteName: ""
    property string pendingDeleteParentId: ""
    property int pendingDeleteIndex: -1
    readonly property string currentKind: mapMode === 0 ? "cloud" : "constellation"
    readonly property var colorKeys: ["accent", "secondary", "success", "danger", "neutral"]
    readonly property var colorNames: [
        qsTr("Accent"), qsTr("Secondary"), qsTr("Success"),
        qsTr("Urgent"), qsTr("Neutral")]
    signal addRequested()
    signal addInCategoryRequested(string categoryId, string subcategoryId)
    signal manageCategoriesRequested()
    signal manageGoalsRequested()
    signal suggestionRequested()

    function colorIndex(color) {
        const index = root.colorKeys.indexOf(color)
        return index >= 0 ? index : 0
    }

    function objectById(type, objectId) {
        const source = type === "group" ? root.mapModel.groups
            : type === "note" ? root.mapModel.notes : root.mapModel.connections
        const key = type === "group" ? "groupId"
            : type === "note" ? "noteId" : "connectionId"
        for (let index = 0; index < source.length; ++index) {
            if (source[index][key] === objectId)
                return source[index]
        }
        return null
    }

    function selectObject(type, objectData) {
        root.selectedType = type
        root.selectedObject = objectData
        root.selectedId = type === "group" ? objectData.groupId
            : type === "note" ? objectData.noteId : objectData.connectionId
        populateInspector()
    }

    function clearSelection() {
        root.selectedType = ""
        root.selectedId = ""
        root.selectedObject = null
    }

    function refreshSelection() {
        if (root.selectedId.length === 0)
            return
        const refreshed = objectById(root.selectedType, root.selectedId)
        if (!refreshed) {
            clearSelection()
            return
        }
        root.selectedObject = refreshed
        populateInspector()
    }

    function populateInspector() {
        if (!root.selectedObject)
            return
        if (root.selectedType === "group") {
            groupTitleField.text = root.selectedObject.title
            groupColorField.currentIndex = root.colorIndex(root.selectedObject.color)
        } else if (root.selectedType === "note") {
            noteTitleField.text = root.selectedObject.title
            noteBodyField.text = root.selectedObject.body
            noteTagsField.text = root.selectedObject.tags
            noteLinkField.text = root.selectedObject.externalLink
            notePriorityField.value = root.selectedObject.priority
            noteColorField.currentIndex = root.colorIndex(root.selectedObject.color)
        }
    }

    function groupsForCurrentView() {
        const revision = root.mapModel.revision
        const result = []
        for (let index = 0; index < root.mapModel.groups.length; ++index) {
            const group = root.mapModel.groups[index]
            if (group.kind === root.currentKind)
                result.push(group)
        }
        return result
    }

    function currentGroupCount() {
        return groupsForCurrentView().length
    }

    function selectedGroupId() {
        if (root.selectedType === "group" && root.selectedObject)
            return root.selectedObject.groupId
        if (root.selectedType === "note" && root.selectedObject)
            return root.selectedObject.groupId
        return ""
    }

    function requestGroupAdd() {
        const center = root.mapMode === 0
            ? cloudCanvas.viewportCenter() : constellationCanvas.viewportCenter()
        root.pendingWorldX = center.x - (root.currentKind === "cloud" ? 190 : 260)
        root.pendingWorldY = center.y - (root.currentKind === "cloud" ? 140 : 200)
        newGroupTitle.text = ""
        groupDialog.open()
        newGroupTitle.forceActiveFocus()
    }

    function openNoteDialog(groupId, x, y) {
        root.groupChoices = groupsForCurrentView()
        if (root.groupChoices.length === 0) {
            requestGroupAdd()
            return
        }
        root.pendingGroupId = groupId
        let selectedIndex = 0
        for (let index = 0; index < root.groupChoices.length; ++index) {
            if (root.groupChoices[index].groupId === groupId) {
                selectedIndex = index
                break
            }
        }
        noteGroupField.currentIndex = selectedIndex
        const chosen = root.groupChoices[selectedIndex]
        const suggested = root.mapMode === 0
            ? cloudCanvas.suggestedNotePosition(chosen.groupId)
            : constellationCanvas.suggestedNotePosition(chosen.groupId)
        root.pendingNoteX = x >= 0 ? x : suggested.x
        root.pendingNoteY = y >= 0 ? y : suggested.y
        newNoteTitle.text = ""
        noteDialog.open()
        newNoteTitle.forceActiveFocus()
    }

    function requestIdeaAdd() {
        openNoteDialog(selectedGroupId(), -1, -1)
    }

    function requestDelete(type, objectId, name, parentId, itemIndex) {
        root.pendingDeleteType = type
        root.pendingDeleteId = objectId
        root.pendingDeleteName = name
        root.pendingDeleteParentId = parentId || ""
        root.pendingDeleteIndex = itemIndex === undefined ? -1 : itemIndex
        deleteDialog.open()
    }

    function deleteSelected() {
        if (!root.selectedObject)
            return
        if (root.selectedType === "group")
            requestDelete("group", root.selectedId, root.selectedObject.title, "", -1)
        else if (root.selectedType === "note") {
            if (root.selectedObject.center)
                requestDelete("group", root.selectedObject.groupId,
                    qsTr("this constellation"), "", -1)
            else
                requestDelete("note", root.selectedId, root.selectedObject.title, "", -1)
        } else if (root.selectedType === "connection")
            requestDelete("connection", root.selectedId,
                root.selectedObject.label || qsTr("this directed connection"), "", -1)
    }

    onMapModeChanged: {
        clearSelection()
        connectionSourceId = ""
    }

    Connections {
        target: root.mapModel
        function onMapChanged() { root.refreshSelection() }
    }

    Shortcut {
        sequence: "Ctrl+Return"
        enabled: root.visible
        onActivated: root.requestIdeaAdd()
    }
    Shortcut {
        sequence: "Ctrl+Shift+N"
        enabled: root.visible
        onActivated: root.requestGroupAdd()
    }
    Shortcut {
        sequence: StandardKey.Delete
        enabled: root.visible && root.selectedObject !== null
        onActivated: root.deleteSelected()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.width < 720 ? 12 : 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 9

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: qsTr("Brainstorm map")
                    color: root.theme.textPrimary
                    font.pixelSize: 25
                    font.weight: Font.DemiBold
                }
                Text {
                    visible: root.width >= 760
                    text: qsTr("One evolving canvas · %1 groups and %2 ideas")
                        .arg(root.mapModel.groups.length).arg(root.mapModel.notes.length)
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }
            }
            AppButton {
                visible: root.showSuggestionAction && root.width >= 980
                theme: root.theme
                text: qsTr("Suggest work")
                onClicked: root.suggestionRequested()
            }
            AppButton {
                theme: root.theme
                text: qsTr("+ Idea")
                onClicked: root.requestIdeaAdd()
            }
            AppButton {
                theme: root.theme
                primary: true
                text: root.currentKind === "cloud"
                    ? qsTr("+ Cloud") : qsTr("+ Constellation")
                onClicked: root.requestGroupAdd()
            }
            AppButton {
                visible: root.width >= 940
                theme: root.theme
                quiet: true
                text: qsTr("+ Task")
                onClicked: root.addRequested()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 7
            Text {
                visible: root.width >= 700
                text: qsTr("VIEW")
                color: root.theme.textMuted
                font.pixelSize: 9
                font.weight: Font.Bold
                font.letterSpacing: 1.2
            }
            AppButton {
                theme: root.theme
                primary: root.mapMode === 0
                quiet: root.mapMode !== 0
                text: qsTr("☁  Clouds")
                onClicked: root.mapMode = 0
            }
            AppButton {
                theme: root.theme
                primary: root.mapMode === 1
                quiet: root.mapMode !== 1
                text: qsTr("✦  Constellations")
                onClicked: root.mapMode = 1
            }
            Item { Layout.fillWidth: true }
            Text {
                visible: root.width >= 760
                text: qsTr("Ctrl+Enter idea · Ctrl+Shift+N group")
                color: root.theme.textMuted
                font.pixelSize: 9
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius + 6
            color: root.theme.background
            border.color: root.theme.borderStrong
            clip: true

            MentalMapClouds {
                id: cloudCanvas
                anchors.fill: parent
                anchors.margins: 2
                visible: root.mapMode === 0
                mapModel: root.mapModel
                theme: root.theme
                selectedType: root.selectedType
                selectedId: root.selectedId
                onObjectSelected: (type, data) => root.selectObject(type, data)
                onSelectionCleared: root.clearSelection()
                onAddNoteRequested: (groupId, x, y) => root.openNoteDialog(groupId, x, y)
            }

            MentalMapConstellation {
                id: constellationCanvas
                anchors.fill: parent
                anchors.margins: 2
                visible: root.mapMode === 1
                mapModel: root.mapModel
                theme: root.theme
                selectedType: root.selectedType
                selectedId: root.selectedId
                onObjectSelected: (type, data) => root.selectObject(type, data)
                onSelectionCleared: root.clearSelection()
                onAddNoteRequested: (groupId, x, y) => root.openNoteDialog(groupId, x, y)
            }

            Column {
                anchors.centerIn: parent
                visible: root.currentGroupCount() === 0
                spacing: 10
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.currentKind === "cloud" ? "☁" : "✦"
                    color: root.theme.accent
                    font.pixelSize: 38
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.currentKind === "cloud"
                        ? qsTr("Create a cloud to gather meeting notes")
                        : qsTr("Create a constellation around a central idea")
                    color: root.theme.textPrimary
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                AppButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    theme: root.theme
                    primary: true
                    text: root.currentKind === "cloud"
                        ? qsTr("Create cloud") : qsTr("Create constellation")
                    onClicked: root.requestGroupAdd()
                }
            }

            Rectangle {
                id: inspector
                visible: root.selectedObject !== null
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                width: Math.min(372, parent.width - 28)
                radius: root.theme.radius + 6
                color: root.theme.surfaceRaised
                border.color: root.theme.accent
                border.width: 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 15
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: inspector.width - 30
                        spacing: 9

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                Layout.fillWidth: true
                                text: root.selectedType === "group"
                                    ? (root.currentKind === "cloud" ? qsTr("CLOUD") : qsTr("CONSTELLATION"))
                                    : root.selectedType === "note" ? qsTr("IDEA")
                                    : qsTr("DIRECTED CONNECTION")
                                color: root.theme.accent
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1.1
                            }
                            AppButton {
                                theme: root.theme
                                quiet: true
                                text: "×"
                                onClicked: root.clearSelection()
                            }
                        }

                        ColumnLayout {
                            visible: root.selectedType === "group"
                            Layout.fillWidth: true
                            spacing: 8
                            Text { text: qsTr("Title"); color: root.theme.textMuted; font.pixelSize: 10 }
                            TextField {
                                id: groupTitleField
                                Layout.fillWidth: true
                                maximumLength: 120
                            }
                            Text { text: qsTr("Color"); color: root.theme.textMuted; font.pixelSize: 10 }
                            ComboBox {
                                id: groupColorField
                                Layout.fillWidth: true
                                model: root.colorNames
                            }
                            Text {
                                Layout.fillWidth: true
                                text: root.selectedObject
                                    ? qsTr("%1 ideas · moving this group carries every idea with it")
                                        .arg(root.selectedObject.noteCount) : ""
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                            AppButton {
                                Layout.fillWidth: true
                                theme: root.theme
                                primary: true
                                text: qsTr("Save group")
                                onClicked: root.mapModel.updateGroup(
                                    root.selectedId,
                                    groupTitleField.text,
                                    root.colorKeys[groupColorField.currentIndex])
                            }
                        }

                        ColumnLayout {
                            visible: root.selectedType === "note"
                            Layout.fillWidth: true
                            spacing: 7
                            Text { text: qsTr("Short title"); color: root.theme.textMuted; font.pixelSize: 10 }
                            TextField {
                                id: noteTitleField
                                Layout.fillWidth: true
                                maximumLength: 160
                            }
                            Text { text: qsTr("Notes"); color: root.theme.textMuted; font.pixelSize: 10 }
                            TextArea {
                                id: noteBodyField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 82
                                wrapMode: TextEdit.Wrap
                                placeholderText: qsTr("Context, decisions, useful detail…")
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Text { text: qsTr("Color"); color: root.theme.textMuted; font.pixelSize: 10 }
                                    ComboBox {
                                        id: noteColorField
                                        Layout.fillWidth: true
                                        model: root.colorNames
                                    }
                                }
                                ColumnLayout {
                                    Text { text: qsTr("Priority"); color: root.theme.textMuted; font.pixelSize: 10 }
                                    SpinBox {
                                        id: notePriorityField
                                        from: 0
                                        to: 5
                                        editable: true
                                    }
                                }
                            }
                            Text { text: qsTr("Tags"); color: root.theme.textMuted; font.pixelSize: 10 }
                            TextField {
                                id: noteTagsField
                                Layout.fillWidth: true
                                placeholderText: qsTr("research, decision, follow-up")
                            }
                            Text { text: qsTr("Link"); color: root.theme.textMuted; font.pixelSize: 10 }
                            RowLayout {
                                Layout.fillWidth: true
                                TextField {
                                    id: noteLinkField
                                    Layout.fillWidth: true
                                    placeholderText: "https://"
                                }
                                AppButton {
                                    visible: noteLinkField.text.length > 0
                                    theme: root.theme
                                    quiet: true
                                    text: qsTr("Open")
                                    onClicked: Qt.openUrlExternally(noteLinkField.text)
                                }
                            }
                            AppButton {
                                Layout.fillWidth: true
                                theme: root.theme
                                primary: true
                                text: qsTr("Save idea")
                                onClicked: root.mapModel.updateNote(
                                    root.selectedId,
                                    noteTitleField.text,
                                    noteBodyField.text,
                                    root.colorKeys[noteColorField.currentIndex],
                                    noteTagsField.text,
                                    noteLinkField.text,
                                    notePriorityField.value)
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 1
                                color: root.theme.border
                            }
                            Text {
                                text: qsTr("CHECKLIST")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1
                            }
                            Repeater {
                                model: root.selectedObject ? root.selectedObject.checklist : []
                                delegate: RowLayout {
                                    id: checklistRow
                                    required property var modelData
                                    required property int index
                                    Layout.fillWidth: true
                                    CheckBox {
                                        checked: checklistRow.modelData.completed
                                        onToggled: root.mapModel.toggleChecklistItem(
                                            root.selectedId, checklistRow.index, checked)
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: checklistRow.modelData.text
                                        color: checklistRow.modelData.completed
                                            ? root.theme.textMuted : root.theme.textPrimary
                                        font.pixelSize: 10
                                        font.strikeout: checklistRow.modelData.completed
                                        wrapMode: Text.WordWrap
                                    }
                                    AppButton {
                                        theme: root.theme
                                        quiet: true
                                        text: "×"
                                        onClicked: root.requestDelete(
                                            "checklist", "", checklistRow.modelData.text,
                                            root.selectedId, checklistRow.index)
                                    }
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                TextField {
                                    id: checklistField
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("New checklist item")
                                    onAccepted: {
                                        if (root.mapModel.addChecklistItem(root.selectedId, text))
                                            text = ""
                                    }
                                }
                                AppButton {
                                    theme: root.theme
                                    text: qsTr("Add")
                                    onClicked: {
                                        if (root.mapModel.addChecklistItem(
                                                root.selectedId, checklistField.text))
                                            checklistField.text = ""
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 1
                                color: root.theme.border
                            }
                            Text {
                                text: qsTr("TASK LINK")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: root.selectedObject
                                    && root.selectedObject.linkedTaskId.length > 0
                                text: root.selectedObject
                                    ? (root.selectedObject.linkedTaskCompleted
                                        ? qsTr("✓ Completed: %1").arg(root.selectedObject.linkedTaskTitle)
                                        : qsTr("Linked: %1").arg(root.selectedObject.linkedTaskTitle)) : ""
                                color: root.selectedObject && root.selectedObject.linkedTaskCompleted
                                    ? root.theme.success : root.theme.textSecondary
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                            AppButton {
                                Layout.fillWidth: true
                                visible: root.selectedObject
                                    && root.selectedObject.linkedTaskId.length === 0
                                theme: root.theme
                                text: qsTr("Convert to linked task")
                                onClicked: root.mapModel.createTaskForNote(root.selectedId)
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 1
                                color: root.theme.border
                            }
                            Text {
                                text: qsTr("CONNECTIONS")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: root.connectionSourceId.length > 0
                                text: root.connectionSourceId === root.selectedId
                                    ? qsTr("This idea is the connection source. Select another idea.")
                                    : qsTr("Ready to connect the source to this idea.")
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                AppButton {
                                    Layout.fillWidth: true
                                    theme: root.theme
                                    primary: root.connectionSourceId.length > 0
                                        && root.connectionSourceId !== root.selectedId
                                    text: root.connectionSourceId.length === 0
                                        ? qsTr("Start connection here")
                                        : root.connectionSourceId === root.selectedId
                                            ? qsTr("Cancel connection")
                                            : qsTr("Connect → here")
                                    onClicked: {
                                        if (root.connectionSourceId.length === 0) {
                                            root.connectionSourceId = root.selectedId
                                        } else if (root.connectionSourceId === root.selectedId) {
                                            root.connectionSourceId = ""
                                        } else {
                                            root.pendingConnectionTargetId = root.selectedId
                                            connectionLabelField.text = ""
                                            connectionDialog.open()
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.selectedType === "connection"
                            Layout.fillWidth: true
                            spacing: 8
                            Text {
                                Layout.fillWidth: true
                                text: root.selectedObject && root.selectedObject.label.length > 0
                                    ? root.selectedObject.label : qsTr("Unlabelled connection")
                                color: root.theme.textPrimary
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.fillWidth: true
                                text: qsTr("The arrow runs from the source idea to the destination idea.")
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                        }

                        AppButton {
                            Layout.fillWidth: true
                            Layout.topMargin: 8
                            theme: root.theme
                            text: root.selectedType === "group"
                                ? qsTr("Delete group…")
                                : root.selectedType === "note" && root.selectedObject
                                    && root.selectedObject.center
                                    ? qsTr("Delete constellation…")
                                    : root.selectedType === "note"
                                        ? qsTr("Delete idea…") : qsTr("Delete connection…")
                            onClicked: root.deleteSelected()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: groupDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 420
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
                text: root.currentKind === "cloud"
                    ? qsTr("Create a cloud") : qsTr("Create a constellation")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: root.currentKind === "cloud"
                    ? qsTr("A movable region for a cluster of short meeting notes.")
                    : qsTr("A central idea with movable surrounding notes.")
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            TextField {
                id: newGroupTitle
                Layout.fillWidth: true
                placeholderText: root.currentKind === "cloud"
                    ? qsTr("Cloud title") : qsTr("Central idea")
                maximumLength: 120
                onAccepted: createGroupButton.clicked()
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: groupDialog.close()
                }
                AppButton {
                    id: createGroupButton
                    theme: root.theme
                    primary: true
                    text: qsTr("Create")
                    onClicked: {
                        const id = root.mapModel.addGroup(
                            root.currentKind, newGroupTitle.text,
                            root.pendingWorldX, root.pendingWorldY)
                        if (id.length > 0)
                            groupDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: noteDialog
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
            spacing: 12
            Text {
                text: qsTr("Capture an idea")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            ComboBox {
                id: noteGroupField
                Layout.fillWidth: true
                model: root.groupChoices.map(group => group.title)
                onActivated: index => {
                    const group = root.groupChoices[index]
                    root.pendingGroupId = group.groupId
                    const point = root.mapMode === 0
                        ? cloudCanvas.suggestedNotePosition(group.groupId)
                        : constellationCanvas.suggestedNotePosition(group.groupId)
                    root.pendingNoteX = point.x
                    root.pendingNoteY = point.y
                }
            }
            TextField {
                id: newNoteTitle
                Layout.fillWidth: true
                placeholderText: qsTr("Keep it short")
                maximumLength: 160
                onAccepted: createNoteButton.clicked()
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: noteDialog.close()
                }
                AppButton {
                    id: createNoteButton
                    theme: root.theme
                    primary: true
                    text: qsTr("Add idea")
                    onClicked: {
                        const group = root.groupChoices[noteGroupField.currentIndex]
                        if (!group)
                            return
                        const id = root.mapModel.addNote(
                            group.groupId, newNoteTitle.text,
                            root.pendingNoteX, root.pendingNoteY)
                        if (id.length > 0)
                            noteDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: connectionDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 420
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
                text: qsTr("Create directed connection")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            TextField {
                id: connectionLabelField
                Layout.fillWidth: true
                placeholderText: qsTr("Optional label, e.g. leads to")
                maximumLength: 120
                onAccepted: createConnectionButton.clicked()
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: connectionDialog.close()
                }
                AppButton {
                    id: createConnectionButton
                    theme: root.theme
                    primary: true
                    text: qsTr("Connect →")
                    onClicked: {
                        const id = root.mapModel.addConnection(
                            root.connectionSourceId,
                            root.pendingConnectionTargetId,
                            connectionLabelField.text)
                        if (id.length > 0) {
                            root.connectionSourceId = ""
                            root.pendingConnectionTargetId = ""
                            connectionDialog.close()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: deleteDialog
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
            border.color: root.theme.danger
        }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                text: qsTr("Delete permanently?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: root.pendingDeleteType === "group"
                    ? qsTr("“%1” and all of its ideas and connections will be deleted.")
                        .arg(root.pendingDeleteName)
                    : qsTr("“%1” will be deleted. This cannot be undone.")
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
                        let removed = false
                        if (root.pendingDeleteType === "group")
                            removed = root.mapModel.deleteGroup(root.pendingDeleteId)
                        else if (root.pendingDeleteType === "note")
                            removed = root.mapModel.deleteNote(root.pendingDeleteId)
                        else if (root.pendingDeleteType === "connection")
                            removed = root.mapModel.deleteConnection(root.pendingDeleteId)
                        else if (root.pendingDeleteType === "checklist")
                            removed = root.mapModel.deleteChecklistItem(
                                root.pendingDeleteParentId, root.pendingDeleteIndex)
                        if (removed) {
                            if (root.pendingDeleteType !== "checklist")
                                root.clearSelection()
                            deleteDialog.close()
                        }
                    }
                }
            }
        }
        onClosed: {
            root.pendingDeleteType = ""
            root.pendingDeleteId = ""
            root.pendingDeleteName = ""
            root.pendingDeleteParentId = ""
            root.pendingDeleteIndex = -1
        }
    }
}
