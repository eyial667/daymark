pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    required property var taskModel
    required property var appSettings

    property int currentPage: 0
    property int pendingCompletionIndex: -1
    property string pendingCompletionTitle: ""

    function requestTaskCompletion(taskIndex, taskTitle) {
        if (!appSettings.confirmTaskCompletion) {
            taskModel.completeTask(taskIndex)
            return
        }
        pendingCompletionIndex = taskIndex
        pendingCompletionTitle = taskTitle
        completionDialog.open()
    }

    width: 1440
    height: 900
    minimumWidth: 1040
    minimumHeight: 700
    visible: true
    title: "Daymark — " + interfaceTheme.modeName
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
        onActivated: window.currentPage = 6
    }

    Shortcut {
        sequence: "Ctrl+Shift+L"
        onActivated: window.appSettings.colorMode =
            window.appSettings.colorMode === 0 ? 1 : 0
    }

    component PlaceholderPage: Item {
        id: placeholderPage

        required property string pageTitle
        required property string detail

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(520, parent.width - 80)
            spacing: 14

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 52
                implicitHeight: 52
                radius: 16
                color: interfaceTheme.accentSoft
                border.color: interfaceTheme.borderStrong

                Text {
                    anchors.centerIn: parent
                    text: "◇"
                    color: interfaceTheme.accent
                    font.pixelSize: 24
                }
            }

            Label {
                Layout.fillWidth: true
                text: placeholderPage.pageTitle
                color: interfaceTheme.textPrimary
                font.pixelSize: 28
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                text: placeholderPage.detail
                color: interfaceTheme.textSecondary
                font.pixelSize: 14
                lineHeight: 1.4
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            Layout.fillHeight: true
            Layout.preferredWidth: 212
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

                StackLayout {
                    currentIndex: Number(window.appSettings.interfaceStyle)

                    MidnightCommandView {
                        taskModel: window.taskModel
                        appSettings: window.appSettings
                        theme: interfaceTheme
                        onAddRequested: quickAdd.open()
                        onCompletionRequested: (taskIndex, taskTitle) =>
                            window.requestTaskCompletion(taskIndex, taskTitle)
                    }

                    SpatialMapView {
                        taskModel: window.taskModel
                        appSettings: window.appSettings
                        theme: interfaceTheme
                        onAddRequested: quickAdd.open()
                        onCompletionRequested: (taskIndex, taskTitle) =>
                            window.requestTaskCompletion(taskIndex, taskTitle)
                    }

                    QuietFocusView {
                        taskModel: window.taskModel
                        appSettings: window.appSettings
                        theme: interfaceTheme
                        onAddRequested: quickAdd.open()
                        onCompletionRequested: (taskIndex, taskTitle) =>
                            window.requestTaskCompletion(taskIndex, taskTitle)
                    }

                    HybridView {
                        taskModel: window.taskModel
                        appSettings: window.appSettings
                        theme: interfaceTheme
                        onAddRequested: quickAdd.open()
                        onCompletionRequested: (taskIndex, taskTitle) =>
                            window.requestTaskCompletion(taskIndex, taskTitle)
                    }
                }

                PlaceholderPage {
                    pageTitle: "Inbox"
                    detail: "A frictionless landing place for uncategorized thoughts and tasks is coming next."
                }

                SpatialMapView {
                    taskModel: window.taskModel
                    appSettings: window.appSettings
                    theme: interfaceTheme
                    onAddRequested: quickAdd.open()
                    onCompletionRequested: (taskIndex, taskTitle) =>
                        window.requestTaskCompletion(taskIndex, taskTitle)
                }

                PlaceholderPage {
                    pageTitle: "Projects"
                    detail: "Project progress, milestones, and the tasks holding each outcome together."
                }

                PlaceholderPage {
                    pageTitle: "Focus"
                    detail: "A distraction-free timer and a single deliberate next action."
                }

                PlaceholderPage {
                    pageTitle: "Review"
                    detail: "Close the loop on your day, carry forward what matters, and make tomorrow lighter."
                }

                SettingsView {
                    appSettings: window.appSettings
                    theme: interfaceTheme
                }
            }
        }
    }

    QuickAddDialog {
        id: quickAdd
        taskModel: window.taskModel
        appSettings: window.appSettings
        theme: interfaceTheme
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
                text: "Complete this task?"
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
                    text: "Cancel"
                    onClicked: completionDialog.close()
                }
                AppButton {
                    theme: interfaceTheme
                    primary: true
                    text: "Mark complete"
                    onClicked: {
                        window.taskModel.completeTask(window.pendingCompletionIndex)
                        completionDialog.close()
                    }
                }
            }
        }

        onClosed: {
            window.pendingCompletionIndex = -1
            window.pendingCompletionTitle = ""
        }
    }
}
