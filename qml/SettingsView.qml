// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: root

    required property var appSettings
    required property var dataTransfer
    required property var theme

    property bool folderOpenFailed: false
    property url pendingImportUrl: ""

    function durationLabel(minutes) {
        const hours = Math.floor(minutes / 60)
        const remaining = minutes % 60
        if (hours === 0)
            return remaining + " min"
        return remaining === 0
            ? hours + (hours === 1 ? " hour" : " hours")
            : hours + "h " + remaining + "m"
    }

    component SettingCopy: ColumnLayout {
        id: copy

        required property string heading
        required property string description

        spacing: 3

        Text {
            Layout.fillWidth: true
            text: copy.heading
            color: root.theme.textPrimary
            font.pixelSize: 13
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            text: copy.description
            color: root.theme.textMuted
            font.pixelSize: 10
            lineHeight: 1.25
            wrapMode: Text.WordWrap
        }
    }

    component SettingsSwitch: Switch {
        id: control

        implicitWidth: 44
        implicitHeight: 26
        padding: 0

        indicator: Rectangle {
            implicitWidth: 44
            implicitHeight: 24
            radius: 12
            color: control.checked ? root.theme.accent : root.theme.surfaceHover
            border.color: control.checked ? root.theme.accent : root.theme.borderStrong

            Rectangle {
                x: control.checked ? parent.width - width - 3 : 3
                anchors.verticalCenter: parent.verticalCenter
                width: 18
                height: 18
                radius: 9
                color: control.checked ? "#FFFFFF" : root.theme.textSecondary

                Behavior on x {
                    NumberAnimation { duration: 110 }
                }
            }
        }

        contentItem: Item {}
    }

    component SettingDivider: Rectangle {
        implicitHeight: 1
        color: root.theme.border
    }

    component SectionHeading: ColumnLayout {
        id: sectionHeading

        required property string heading
        required property string description

        spacing: 4

        Text {
            Layout.fillWidth: true
            text: sectionHeading.heading
            color: root.theme.textPrimary
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }
        Text {
            Layout.fillWidth: true
            text: sectionHeading.description
            color: root.theme.textSecondary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    ScrollView {
        id: settingsScroll

        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Item {
            width: settingsScroll.availableWidth
            implicitHeight: settingsColumn.implicitHeight + 56

            ColumnLayout {
                id: settingsColumn

                width: Math.min(900, parent.width - 56)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 26
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 4

                    ColumnLayout {
                        spacing: 3
                        Text {
                            text: "Settings"
                            color: root.theme.textPrimary
                            font.pixelSize: 27
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: "Shape Daymark around the way you plan and work."
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        implicitWidth: savedLabel.implicitWidth + 22
                        implicitHeight: 30
                        radius: 15
                        color: root.theme.accentSoft
                        border.color: root.theme.borderStrong

                        Text {
                            id: savedLabel
                            anchors.centerIn: parent
                            text: "✓  Saved automatically"
                            color: root.theme.accent
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: appearanceContent.implicitHeight + 32
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        id: appearanceContent
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 13

                        SectionHeading {
                            Layout.fillWidth: true
                            heading: "Appearance"
                            description: "Choose the application’s color mode, structure, and accent."
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Interface"
                                description: "Changes the Today layout without changing your tasks."
                            }

                            ComboBox {
                                id: interfaceSelector
                                Layout.preferredWidth: 250
                                model: [
                                    "A · Midnight Command",
                                    "B · Spatial Map",
                                    "C · Quiet Focus",
                                    "D · Daymark Hybrid"
                                ]
                                currentIndex: Number(root.appSettings.interfaceStyle)
                                onActivated: root.appSettings.interfaceStyle = currentIndex
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Accent color"
                                description: "Use each interface’s signature color or choose one globally."
                            }

                            RowLayout {
                                spacing: 9

                                Rectangle {
                                    implicitWidth: 18
                                    implicitHeight: 18
                                    radius: 9
                                    color: root.theme.accent
                                }

                                ComboBox {
                                    id: accentSelector
                                    Layout.preferredWidth: 190
                                    model: ["Match interface", "Violet", "Cyan", "Amber", "Green"]
                                    currentIndex: Number(root.appSettings.accentPreset)
                                    onActivated: root.appSettings.accentPreset = currentIndex
                                }
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Color mode"
                                description: "Switch the complete application palette between dark and light."
                            }

                            RowLayout {
                                spacing: 9

                                Text {
                                    text: root.appSettings.colorMode === 0 ? "☾" : "☀"
                                    color: root.theme.accent
                                    font.pixelSize: 18
                                }

                                ComboBox {
                                    id: colorModeSelector
                                    Layout.preferredWidth: 190
                                    model: ["Dark", "Light"]
                                    currentIndex: Number(root.appSettings.colorMode)
                                    onActivated: root.appSettings.colorMode = currentIndex
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: planningContent.implicitHeight + 32
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        id: planningContent
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 13

                        SectionHeading {
                            Layout.fillWidth: true
                            heading: "Planning defaults"
                            description: "These values drive workload guidance and prefill new tasks."
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Daily task capacity"
                                description: "How much focused task time can realistically fit in one day."
                            }

                            SpinBox {
                                id: capacityControl
                                Layout.preferredWidth: 160
                                from: 60
                                to: 960
                                stepSize: 30
                                editable: true
                                value: root.appSettings.dailyCapacityMinutes
                                textFromValue: (value, locale) => value + " min"
                                valueFromText: (text, locale) => {
                                    const parsed = parseInt(text)
                                    return Number.isNaN(parsed) ? capacityControl.value : parsed
                                }
                                onValueModified: root.appSettings.dailyCapacityMinutes = value
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Default task estimate"
                                description: "The initial duration shown when capturing a task."
                            }

                            SpinBox {
                                Layout.preferredWidth: 160
                                from: 5
                                to: 480
                                stepSize: 5
                                editable: true
                                value: root.appSettings.defaultEstimatedMinutes
                                textFromValue: (value, locale) => value + " min"
                                valueFromText: (text, locale) => {
                                    const parsed = parseInt(text)
                                    return Number.isNaN(parsed)
                                        ? root.appSettings.defaultEstimatedMinutes
                                        : parsed
                                }
                                onValueModified: root.appSettings.defaultEstimatedMinutes = value
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Default importance"
                                description: "The starting importance for newly captured tasks."
                            }

                            ComboBox {
                                Layout.preferredWidth: 160
                                model: ["1 · Low", "2", "3 · Normal", "4", "5 · Critical"]
                                currentIndex: root.appSettings.defaultImportance - 1
                                onActivated: root.appSettings.defaultImportance = currentIndex + 1
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: behaviorContent.implicitHeight + 32
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        id: behaviorContent
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 13

                        SectionHeading {
                            Layout.fillWidth: true
                            heading: "Behavior"
                            description: "Control how information and irreversible-looking actions are presented."
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "24-hour clock"
                                description: "Display agenda and timeline hours as 09:00 and 14:00."
                            }

                            SettingsSwitch {
                                checked: root.appSettings.use24HourClock
                                onToggled: root.appSettings.use24HourClock = checked
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Show priority explanations"
                                description: "Keep the reason behind Daymark’s ranking visible in detailed queues."
                            }

                            SettingsSwitch {
                                checked: root.appSettings.showPriorityReasons
                                onToggled: root.appSettings.showPriorityReasons = checked
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Confirm task completion"
                                description: "Ask before a task is marked complete and removed from the active queue."
                            }

                            SettingsSwitch {
                                checked: root.appSettings.confirmTaskCompletion
                                onToggled: root.appSettings.confirmTaskCompletion = checked
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: dataContent.implicitHeight + 32
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        id: dataContent
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 13

                        SectionHeading {
                            Layout.fillWidth: true
                            heading: "Local data & privacy"
                            description: "Your tasks, categories, goals, and preferences stay on this machine. Export a portable copy whenever you want."
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Portable Daymark data"
                                description: "Export tasks, categories and notes, completed history, long-term goals, milestones, and preferences. Imports merge with data already on this computer."
                            }

                            AppButton {
                                theme: root.theme
                                text: "Import"
                                onClicked: {
                                    root.dataTransfer.clearStatus()
                                    importFileDialog.open()
                                }
                            }

                            AppButton {
                                theme: root.theme
                                primary: true
                                text: "Export"
                                onClicked: {
                                    root.dataTransfer.clearStatus()
                                    exportFileDialog.open()
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            visible: root.dataTransfer.statusMessage.length > 0
                            text: root.dataTransfer.statusMessage
                            color: root.theme.accent
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Text {
                                    text: "Data folder"
                                    color: root.theme.textPrimary
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: root.appSettings.dataDirectory
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                    wrapMode: Text.WrapAnywhere
                                }
                                Text {
                                    visible: root.folderOpenFailed
                                    text: "The folder could not be opened by the desktop."
                                    color: root.theme.danger
                                    font.pixelSize: 10
                                }
                            }

                            AppButton {
                                theme: root.theme
                                text: "Open data folder"
                                onClicked: root.folderOpenFailed = !root.appSettings.openDataDirectory()
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: "Reset preferences"
                                description: "Restore settings only. Your tasks, categories, goals, milestones, and local database will not be changed."
                            }

                            AppButton {
                                theme: root.theme
                                text: "Reset to defaults"
                                onClicked: resetDialog.open()
                            }
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: exportFileDialog

        title: "Export Daymark data"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Daymark data (*.daymark.json)"]
        defaultSuffix: "daymark.json"
        onAccepted: root.dataTransfer.exportData(selectedFile)
    }

    FileDialog {
        id: importFileDialog

        title: "Import Daymark data"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Daymark data (*.daymark.json)", "JSON files (*.json)"]
        onAccepted: {
            root.pendingImportUrl = selectedFile
            importConfirmationDialog.open()
        }
    }

    Dialog {
        id: importConfirmationDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 450
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
            spacing: 13

            Text {
                text: "Import this Daymark data?"
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: "Tasks, categories, goals, milestones, and preferences from the selected file will be merged into this computer. Existing records are kept."
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
                    text: "Cancel"
                    onClicked: importConfirmationDialog.close()
                }

                AppButton {
                    theme: root.theme
                    primary: true
                    text: "Import and merge"
                    onClicked: {
                        root.dataTransfer.importData(root.pendingImportUrl)
                        importConfirmationDialog.close()
                    }
                }
            }
        }

        onClosed: root.pendingImportUrl = ""
    }

    Dialog {
        id: resetDialog

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
            spacing: 13

            Text {
                text: "Reset all preferences?"
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: "Color mode, interface, accent, planning defaults, and behavior settings will be restored. Tasks and goals stay untouched."
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 5

                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: "Cancel"
                    onClicked: resetDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: "Reset preferences"
                    onClicked: {
                        root.appSettings.resetDefaults()
                        resetDialog.close()
                    }
                }
            }
        }
    }
}
