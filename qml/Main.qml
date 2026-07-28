pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    required property var taskModel
    required property var todayTaskModel
    required property var categoryModel
    required property var goalModel
    required property var meetingModel
    required property var mentalMapModel
    required property var dailyNoteModel
    required property var focusSession
    required property var appSettings
    required property var dataTransfer
    required property var updateService

    property int currentPage: 0
    property int pendingCompletionIndex: -1
    property string pendingCompletionTitle: ""
    property var pendingCompletionModel: null
    property string pendingCategoryTaskId: ""
    property string pendingCategoryTitle: ""
    property string pendingCategoryId: ""
    property string pendingSubcategoryId: ""
    property bool planningPromptOpen: false
    property bool dailyNoteOpen: false
    property string feedbackMessage: ""

    function requestTaskCompletion(model, taskIndex, taskTitle) {
        if (!appSettings.confirmTaskCompletion) {
            model.completeTask(taskIndex)
            return
        }
        pendingCompletionModel = model
        pendingCompletionIndex = taskIndex
        pendingCompletionTitle = taskTitle
        completionDialog.open()
    }

    function requestTaskCategory(taskId, taskTitle, categoryId, subcategoryId) {
        pendingCategoryTaskId = taskId
        pendingCategoryTitle = taskTitle
        pendingCategoryId = categoryId
        pendingSubcategoryId = subcategoryId
        const assignmentIndex = categoryModel.indexOfAssignment(categoryId, subcategoryId)
        categoryAssignmentField.currentIndex = assignmentIndex >= 0
            ? assignmentIndex + 1 : 0
        categoryAssignmentDialog.open()
    }

    function showFeedback(message) {
        if (message.length === 0)
            return
        feedbackMessage = message
        feedbackTimer.restart()
    }

    function openFocusForTopTask() {
        if (todayTaskModel.activeCount > 0) {
            focusSession.prepareTask(
                todayTaskModel.topTaskId,
                todayTaskModel.topTaskTitle,
                todayTaskModel.topTaskEstimatedMinutes)
        }
        currentPage = 5
    }

    onCurrentPageChanged: {
        if (currentPage !== 0) {
            planningPromptOpen = false
            if (dailyNoteOpen)
                dailyNoteModel.saveNow()
            dailyNoteOpen = false
        }
    }

    width: 1440
    height: 900
    minimumWidth: 640
    minimumHeight: 600
    visible: true
    title: qsTr("Daymark — %1").arg(interfaceTheme.modeName)
    color: interfaceTheme.background

    palette.window: interfaceTheme.background
    palette.windowText: interfaceTheme.textPrimary
    palette.base: interfaceTheme.surface
    palette.alternateBase: interfaceTheme.surfaceRaised
    palette.text: interfaceTheme.textPrimary
    palette.button: interfaceTheme.surfaceRaised
    palette.buttonText: interfaceTheme.textPrimary
    palette.brightText: "#FFFFFF"
    palette.light: interfaceTheme.surfaceHover
    palette.midlight: interfaceTheme.borderStrong
    palette.mid: interfaceTheme.border
    palette.dark: interfaceTheme.sidebar
    palette.shadow: interfaceTheme.shadow
    palette.highlight: interfaceTheme.accent
    palette.highlightedText: "#FFFFFF"
    palette.link: interfaceTheme.accent
    palette.linkVisited: interfaceTheme.secondaryAccent
    palette.placeholderText: interfaceTheme.textMuted
    palette.toolTipBase: interfaceTheme.surfaceRaised
    palette.toolTipText: interfaceTheme.textPrimary

    Theme {
        id: interfaceTheme
        style: Number(window.appSettings.interfaceStyle)
        colorMode: Number(window.appSettings.colorMode)
        accentPreset: Number(window.appSettings.accentPreset)
    }

    Shortcut {
        sequence: "Ctrl+N"
        onActivated: quickAdd.open()
    }

    Connections {
        target: window.taskModel
        function onStatusMessageChanged() {
            window.showFeedback(window.taskModel.statusMessage)
        }
    }

    Connections {
        target: window.todayTaskModel
        function onStatusMessageChanged() {
            window.showFeedback(window.todayTaskModel.statusMessage)
        }
    }

    Connections {
        target: window.goalModel
        function onStatusMessageChanged() {
            window.showFeedback(window.goalModel.statusMessage)
        }
    }

    Connections {
        target: window.meetingModel
        function onStatusMessageChanged() {
            window.showFeedback(window.meetingModel.statusMessage)
        }
    }

    Connections {
        target: window.focusSession
        function onStatusMessageChanged() {
            window.showFeedback(window.focusSession.statusMessage)
        }
    }

    Connections {
        target: window.mentalMapModel
        function onStatusMessageChanged() {
            window.showFeedback(window.mentalMapModel.statusMessage)
        }
    }

    Connections {
        target: window.dailyNoteModel
        function onStatusMessageChanged() {
            window.showFeedback(window.dailyNoteModel.statusMessage)
        }
    }

    Connections {
        target: window.updateService
        function onUpdateFound() {
            updateDialog.open()
        }
    }

    Timer {
        id: feedbackTimer
        interval: 2800
        onTriggered: {
            window.feedbackMessage = ""
            window.taskModel.clearStatus()
            window.todayTaskModel.clearStatus()
            window.goalModel.clearStatus()
            window.meetingModel.clearStatus()
            window.focusSession.clearStatus()
            window.mentalMapModel.clearStatus()
            window.dailyNoteModel.clearStatus()
        }
    }

    Shortcut {
        sequence: "Ctrl+1"
        onActivated: window.appSettings.interfaceStyle = 0
    }

    Shortcut {
        sequence: "Ctrl+2"
        onActivated: window.appSettings.interfaceStyle = 1
    }

    Shortcut {
        sequence: "Ctrl+3"
        onActivated: window.appSettings.interfaceStyle = 2
    }

    Shortcut {
        sequence: "Ctrl+4"
        onActivated: window.appSettings.interfaceStyle = 3
    }

    Shortcut {
        sequence: "Ctrl+,"
        onActivated: window.currentPage = 8
    }

    Shortcut {
        sequence: "Ctrl+Shift+L"
        onActivated: window.appSettings.colorMode =
            window.appSettings.colorMode === 0 ? 1 : 0
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            Layout.fillHeight: true
            Layout.preferredWidth: window.width < 850 ? 164 : 212
            theme: interfaceTheme
            appSettings: window.appSettings
            currentIndex: window.currentPage
            onPageRequested: index => window.currentPage = index
            onAddRequested: quickAdd.open()
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: interfaceTheme.background

            StackLayout {
                anchors.fill: parent
                currentIndex: window.currentPage

                Item {
                    StackLayout {
                        anchors.fill: parent
                        currentIndex: Number(window.appSettings.interfaceStyle)

                        MidnightCommandView {
                            taskModel: window.todayTaskModel
                            appSettings: window.appSettings
                            theme: interfaceTheme
                            onAddRequested: quickAdd.open()
                            onSuggestionRequested: window.planningPromptOpen = true
                            onFocusRequested: window.openFocusForTopTask()
                            onCompletionRequested: (taskIndex, taskTitle) =>
                                window.requestTaskCompletion(
                                    window.todayTaskModel, taskIndex, taskTitle)
                            onCategoryRequested: (taskId, taskTitle, categoryId, subcategoryId) =>
                                window.requestTaskCategory(
                                    taskId, taskTitle, categoryId, subcategoryId)
                        }

                        SpatialMapView {
                            mapModel: window.mentalMapModel
                            theme: interfaceTheme
                            showSuggestionAction: true
                            onAddRequested: quickAdd.open()
                            onSuggestionRequested: window.planningPromptOpen = true
                            onAddInCategoryRequested: (categoryId, subcategoryId) =>
                                quickAdd.openForAssignment(categoryId, subcategoryId)
                            onManageCategoriesRequested: window.currentPage = 3
                            onManageGoalsRequested: window.currentPage = 4
                        }

                        QuietFocusView {
                            taskModel: window.todayTaskModel
                            appSettings: window.appSettings
                            theme: interfaceTheme
                            onAddRequested: quickAdd.open()
                            onSuggestionRequested: window.planningPromptOpen = true
                            onFocusRequested: window.openFocusForTopTask()
                            onReviewRequested: window.currentPage = 6
                            onCompletionRequested: (taskIndex, taskTitle) =>
                                window.requestTaskCompletion(
                                    window.todayTaskModel, taskIndex, taskTitle)
                            onCategoryRequested: (taskId, taskTitle, categoryId, subcategoryId) =>
                                window.requestTaskCategory(
                                    taskId, taskTitle, categoryId, subcategoryId)
                        }

                        HybridView {
                            taskModel: window.todayTaskModel
                            appSettings: window.appSettings
                            theme: interfaceTheme
                            onAddRequested: quickAdd.open()
                            onSuggestionRequested: window.planningPromptOpen = true
                            onFocusRequested: window.openFocusForTopTask()
                            onMapRequested: window.currentPage = 2
                            onCompletionRequested: (taskIndex, taskTitle) =>
                                window.requestTaskCompletion(
                                    window.todayTaskModel, taskIndex, taskTitle)
                            onCategoryRequested: (taskId, taskTitle, categoryId, subcategoryId) =>
                                window.requestTaskCategory(
                                    taskId, taskTitle, categoryId, subcategoryId)
                        }
                    }

                    DayPlanningPrompt {
                        anchors.fill: parent
                        z: 20
                        visible: window.planningPromptOpen
                        todayTaskModel: window.todayTaskModel
                        categoryModel: window.categoryModel
                        goalModel: window.goalModel
                        appSettings: window.appSettings
                        theme: interfaceTheme
                        onAddRequested: quickAdd.open()
                        onCategoryTaskRequested: (categoryId, subcategoryId) =>
                            quickAdd.openForAssignment(categoryId, subcategoryId)
                        onCloseRequested: window.planningPromptOpen = false
                    }

                    AppButton {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 18
                        z: 18
                        visible: !window.planningPromptOpen && !window.dailyNoteOpen
                        theme: interfaceTheme
                        primary: window.dailyNoteModel.dirty
                        text: qsTr("✎  Day note")
                        onClicked: window.dailyNoteOpen = true
                    }

                    DailyNotePanel {
                        anchors.fill: parent
                        z: 30
                        visible: window.dailyNoteOpen
                        dailyNoteModel: window.dailyNoteModel
                        theme: interfaceTheme
                        onCloseRequested: window.dailyNoteOpen = false
                    }
                }

                TodoListView {
                    taskModel: window.taskModel
                    appSettings: window.appSettings
                    theme: interfaceTheme
                    onAddRequested: quickAdd.open()
                    onCompletionRequested: (taskIndex, taskTitle) =>
                        window.requestTaskCompletion(
                            window.taskModel, taskIndex, taskTitle)
                    onPlanRequested: taskId =>
                        window.taskModel.planTaskForToday(taskId)
                    onCategoryRequested: (taskId, taskTitle, categoryId, subcategoryId) =>
                        window.requestTaskCategory(
                            taskId, taskTitle, categoryId, subcategoryId)
                }

                SpatialMapView {
                    mapModel: window.mentalMapModel
                    theme: interfaceTheme
                    onAddRequested: quickAdd.open()
                    onAddInCategoryRequested: (categoryId, subcategoryId) =>
                        quickAdd.openForAssignment(categoryId, subcategoryId)
                    onManageCategoriesRequested: window.currentPage = 3
                    onManageGoalsRequested: window.currentPage = 4
                }

                CategoriesView {
                    categoryModel: window.categoryModel
                    theme: interfaceTheme
                }

                GoalsView {
                    goalModel: window.goalModel
                    theme: interfaceTheme
                }

                FocusView {
                    focusSession: window.focusSession
                    taskModel: window.todayTaskModel
                    theme: interfaceTheme
                    onBackRequested: window.currentPage = 0
                }

                ReviewView {
                    todayTaskModel: window.todayTaskModel
                    dailyNoteModel: window.dailyNoteModel
                    appSettings: window.appSettings
                    theme: interfaceTheme
                    onBackRequested: window.currentPage = 0
                    onCompletionRequested: (taskIndex, taskTitle) =>
                        window.requestTaskCompletion(
                            window.todayTaskModel, taskIndex, taskTitle)
                    onCategoryRequested: (taskId, taskTitle, categoryId, subcategoryId) =>
                        window.requestTaskCategory(
                            taskId, taskTitle, categoryId, subcategoryId)
                }

                MeetingsView {
                    meetingModel: window.meetingModel
                    theme: interfaceTheme
                }

                SettingsView {
                    appSettings: window.appSettings
                    dataTransfer: window.dataTransfer
                    updateService: window.updateService
                    theme: interfaceTheme
                }
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 18
        z: 100
        width: Math.min(620, feedbackText.implicitWidth + 38)
        implicitHeight: 42
        radius: interfaceTheme.radius
        color: interfaceTheme.surfaceRaised
        border.color: interfaceTheme.accent
        border.width: 1
        opacity: window.feedbackMessage.length > 0 ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: 140 }
        }

        Text {
            id: feedbackText
            anchors.centerIn: parent
            width: Math.min(580, implicitWidth)
            text: window.feedbackMessage
            color: interfaceTheme.textPrimary
            font.pixelSize: 12
            font.weight: Font.Medium
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: window.feedbackMessage = ""
        }
    }

    QuickAddDialog {
        id: quickAdd
        taskModel: window.taskModel
        categoryModel: window.categoryModel
        appSettings: window.appSettings
        theme: interfaceTheme
        onManageCategoriesRequested: window.currentPage = 3
    }

    UpdateDialog {
        id: updateDialog
        updateService: window.updateService
        theme: interfaceTheme
    }

    Dialog {
        id: categoryAssignmentDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 440
        modal: true
        focus: true
        padding: 22
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: interfaceTheme.radius + 3
            color: interfaceTheme.surfaceRaised
            border.color: interfaceTheme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 13

            Text {
                text: qsTr("Assign a category")
                color: interfaceTheme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: window.pendingCategoryTitle
                color: interfaceTheme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            ComboBox {
                id: categoryAssignmentField
                Layout.fillWidth: true
                model: [qsTr("No category")].concat(window.categoryModel.assignmentNames)
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 5

                AppButton {
                    theme: interfaceTheme
                    quiet: true
                    text: qsTr("Manage categories")
                    onClicked: {
                        categoryAssignmentDialog.close()
                        window.currentPage = 3
                    }
                }
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: interfaceTheme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: categoryAssignmentDialog.close()
                }
                AppButton {
                    theme: interfaceTheme
                    primary: true
                    text: qsTr("Save")
                    onClicked: {
                        const categoryId = categoryAssignmentField.currentIndex > 0
                            ? window.categoryModel.categoryIdForAssignment(
                                categoryAssignmentField.currentIndex - 1)
                            : ""
                        const subcategoryId = categoryAssignmentField.currentIndex > 0
                            ? window.categoryModel.subcategoryIdForAssignment(
                                categoryAssignmentField.currentIndex - 1)
                            : ""
                        if (window.taskModel.assignTaskCategory(
                                window.pendingCategoryTaskId,
                                categoryId,
                                subcategoryId)) {
                            categoryAssignmentDialog.close()
                        }
                    }
                }
            }
        }

        onClosed: {
            window.pendingCategoryTaskId = ""
            window.pendingCategoryTitle = ""
            window.pendingCategoryId = ""
            window.pendingSubcategoryId = ""
        }
    }

    Dialog {
        id: completionDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 420
        modal: true
        focus: true
        padding: 22
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            radius: interfaceTheme.radius + 3
            color: interfaceTheme.surfaceRaised
            border.color: interfaceTheme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 13

            Text {
                text: qsTr("Complete this task?")
                color: interfaceTheme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: window.pendingCompletionTitle
                color: interfaceTheme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 5

                Item { Layout.fillWidth: true }
                AppButton {
                    theme: interfaceTheme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: completionDialog.close()
                }
                AppButton {
                    theme: interfaceTheme
                    primary: true
                    text: qsTr("Mark complete")
                    onClicked: {
                        if (window.pendingCompletionModel)
                            window.pendingCompletionModel.completeTask(
                                window.pendingCompletionIndex)
                        completionDialog.close()
                    }
                }
            }
        }

        onClosed: {
            window.pendingCompletionIndex = -1
            window.pendingCompletionTitle = ""
            window.pendingCompletionModel = null
        }
    }
}
