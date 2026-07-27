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
    required property var updateService
    required property var theme

    property bool folderOpenFailed: false
    property url pendingImportUrl: ""

    function durationLabel(minutes) {
        const hours = Math.floor(minutes / 60)
        const remaining = minutes % 60
        if (hours === 0)
            return qsTr("%1 min").arg(remaining)
        return remaining === 0
            ? (hours === 1 ? qsTr("%1 hour").arg(hours) : qsTr("%1 hours").arg(hours))
            : qsTr("%1h %2m").arg(hours).arg(remaining)
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
                            text: qsTr("Settings")
                            color: root.theme.textPrimary
                            font.pixelSize: 27
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: qsTr("Shape Daymark around the way you plan and work.")
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
                            text: qsTr("✓  Saved automatically")
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
                            heading: qsTr("Appearance")
                            description: qsTr("Choose the application’s color mode, structure, and accent.")
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("Interface")
                                description: qsTr("Changes the Today layout without changing your tasks.")
                            }

                            ComboBox {
                                id: interfaceSelector
                                Layout.preferredWidth: 250
                                model: [
                                    qsTr("A · Midnight Command"),
                                    qsTr("B · Spatial Map"),
                                    qsTr("C · Quiet Focus"),
                                    qsTr("D · Daymark Hybrid")
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
                                heading: qsTr("Accent color")
                                description: qsTr("Use each interface’s signature color or choose one globally.")
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
                                    model: [qsTr("Match interface"), qsTr("Violet"),
                                        qsTr("Cyan"), qsTr("Amber"), qsTr("Green")]
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
                                heading: qsTr("Color mode")
                                description: qsTr("Switch the complete application palette between dark and light.")
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
                                    model: [qsTr("Dark"), qsTr("Light")]
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
                            heading: qsTr("Planning defaults")
                            description: qsTr("These values drive workload guidance and prefill new tasks.")
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("Daily task capacity")
                                description: qsTr("How much focused task time can realistically fit in one day.")
                            }

                            SpinBox {
                                id: capacityControl
                                Layout.preferredWidth: 160
                                from: 60
                                to: 960
                                stepSize: 30
                                editable: true
                                value: root.appSettings.dailyCapacityMinutes
                                textFromValue: (value, locale) => qsTr("%1 min").arg(value)
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
                                heading: qsTr("Default task estimate")
                                description: qsTr("The initial duration shown when capturing a task.")
                            }

                            SpinBox {
                                Layout.preferredWidth: 160
                                from: 5
                                to: 480
                                stepSize: 5
                                editable: true
                                value: root.appSettings.defaultEstimatedMinutes
                                textFromValue: (value, locale) => qsTr("%1 min").arg(value)
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
                                heading: qsTr("Default importance")
                                description: qsTr("The starting importance for newly captured tasks.")
                            }

                            ComboBox {
                                Layout.preferredWidth: 160
                                model: [qsTr("1 · Low"), "2", qsTr("3 · Normal"),
                                    "4", qsTr("5 · Critical")]
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
                            heading: qsTr("Behavior")
                            description: qsTr("Control how information and irreversible-looking actions are presented.")
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("Show timeline")
                                description: qsTr("Show time-based planning panels in Today. Leave this off for a task-list-first view.")
                            }

                            SettingsSwitch {
                                checked: root.appSettings.showTimeline
                                onToggled: root.appSettings.showTimeline = checked
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("24-hour clock")
                                description: qsTr("When timeline panels are shown, display hours as 09:00 and 14:00.")
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
                                heading: qsTr("Show priority explanations")
                                description: qsTr("Keep the reason behind Daymark’s ranking visible in detailed queues.")
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
                                heading: qsTr("Confirm task completion")
                                description: qsTr("Ask before a task is marked complete and removed from the active queue.")
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
                    implicitHeight: updateContent.implicitHeight + 32
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        id: updateContent
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 13

                        SectionHeading {
                            Layout.fillWidth: true
                            heading: qsTr("Software updates")
                            description: qsTr("Daymark checks its official GitHub Releases page when it starts. Downloads and installation always need your approval.")
                        }

                        SettingDivider { Layout.fillWidth: true }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: root.updateService.latestVersion.length > 0
                                    ? qsTr("Installed %1 · Latest %2")
                                        .arg(root.updateService.currentVersion)
                                        .arg(root.updateService.latestVersion)
                                    : qsTr("Installed %1")
                                        .arg(root.updateService.currentVersion)
                                description: root.updateService.statusMessage.length > 0
                                    ? root.updateService.statusMessage
                                    : qsTr("No package is downloaded until you choose to update.")
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Item { Layout.fillWidth: true }

                                AppButton {
                                    theme: root.theme
                                    text: root.updateService.checking
                                        ? qsTr("Checking…") : qsTr("Check now")
                                    enabled: !root.updateService.busy
                                    onClicked: root.updateService.checkForUpdates()
                                }

                                AppButton {
                                    visible: root.updateService.canDownload
                                    theme: root.theme
                                    primary: true
                                    text: qsTr("Download")
                                    onClicked: root.updateService.downloadUpdate()
                                }

                                AppButton {
                                    visible: root.updateService.canInstall
                                    theme: root.theme
                                    primary: true
                                    text: root.updateService.installActionLabel
                                    onClicked: root.updateService.installUpdate()
                                }
                            }
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            visible: root.updateService.downloading
                            from: 0
                            to: 100
                            value: root.updateService.downloadProgress
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Update checks contact GitHub only for release metadata. Downloaded packages must match GitHub’s SHA-256 digest before Daymark opens an installer.")
                            color: root.theme.textMuted
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
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
                            heading: qsTr("Local data & privacy")
                            description: qsTr("Your tasks, categories, subcategories, goals, and preferences stay on this machine. Export a portable copy whenever you want.")
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("Portable Daymark data")
                                description: qsTr("Export tasks, categories, subcategories and notes, completed history, long-term goals, milestones, and preferences. Imports merge with data already on this computer.")
                            }

                            AppButton {
                                theme: root.theme
                                text: qsTr("Import")
                                onClicked: {
                                    root.dataTransfer.clearStatus()
                                    importFileDialog.open()
                                }
                            }

                            AppButton {
                                theme: root.theme
                                primary: true
                                text: qsTr("Export")
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
                                    text: qsTr("Data folder")
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
                                    text: qsTr("The folder could not be opened by the desktop.")
                                    color: root.theme.danger
                                    font.pixelSize: 10
                                }
                            }

                            AppButton {
                                theme: root.theme
                                text: qsTr("Open data folder")
                                onClicked: root.folderOpenFailed = !root.appSettings.openDataDirectory()
                            }
                        }

                        SettingDivider { Layout.fillWidth: true }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18

                            SettingCopy {
                                Layout.fillWidth: true
                                heading: qsTr("Reset preferences")
                                description: qsTr("Restore settings only. Your tasks, categories, subcategories, goals, milestones, and local database will not be changed.")
                            }

                            AppButton {
                                theme: root.theme
                                text: qsTr("Reset to defaults")
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

        title: qsTr("Export Daymark data")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("Daymark data (*.daymark.json)")]
        defaultSuffix: "daymark.json"
        onAccepted: root.dataTransfer.exportData(selectedFile)
    }

    FileDialog {
        id: importFileDialog

        title: qsTr("Import Daymark data")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Daymark data (*.daymark.json)"), qsTr("JSON files (*.json)")]
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
                text: qsTr("Import this Daymark data?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Tasks, categories, subcategories, goals, milestones, and preferences from the selected file will be merged into this computer. Existing records are kept.")
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
                    onClicked: importConfirmationDialog.close()
                }

                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Import and merge")
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
                text: qsTr("Reset all preferences?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Color mode, interface, accent, planning defaults, and behavior settings will be restored. Tasks and goals stay untouched.")
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
                    text: qsTr("Cancel")
                    onClicked: resetDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Reset preferences")
                    onClicked: {
                        root.appSettings.resetDefaults()
                        resetDialog.close()
                    }
                }
            }
        }
    }
}
