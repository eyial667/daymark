// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    required property var updateService
    required property var theme

    function sizeLabel(bytes) {
        if (bytes < 1024 * 1024)
            return Math.max(1, Math.round(bytes / 1024)) + " KB"
        return (bytes / (1024 * 1024)).toFixed(1) + " MB"
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(540, parent ? parent.width - 36 : 540)
    modal: true
    focus: true
    padding: 24
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        radius: root.theme.radius + 3
        color: root.theme.surfaceRaised
        border.color: root.theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                implicitWidth: 42
                implicitHeight: 42
                radius: 21
                color: root.theme.accentSoft

                Text {
                    anchors.centerIn: parent
                    text: "↑"
                    color: root.theme.accent
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: "Daymark " + root.updateService.latestVersion + " is available"
                    color: root.theme.textPrimary
                    font.pixelSize: 19
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
                Text {
                    text: "Installed " + root.updateService.currentVersion
                        + "  ·  " + root.sizeLabel(root.updateService.downloadSize)
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: notesText.implicitHeight + 22
            radius: root.theme.radius
            color: root.theme.surface
            border.color: root.theme.border
            visible: root.updateService.releaseNotes.length > 0

            Text {
                id: notesText
                anchors.fill: parent
                anchors.margins: 11
                text: root.updateService.releaseNotes
                textFormat: Text.PlainText
                color: root.theme.textSecondary
                font.pixelSize: 11
                lineHeight: 1.25
                wrapMode: Text.WordWrap
                maximumLineCount: 10
                elide: Text.ElideRight
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            visible: root.updateService.downloading

            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: root.updateService.downloadProgress
            }
            Text {
                text: root.updateService.downloadProgress + "%"
                color: root.theme.textMuted
                font.pixelSize: 10
            }
        }

        Text {
            Layout.fillWidth: true
            text: root.updateService.statusMessage
            color: root.updateService.hasError
                ? root.theme.danger : root.theme.textSecondary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }

        Text {
            Layout.fillWidth: true
            visible: root.updateService.canInstall
            text: root.updateService.installHint
            color: root.theme.textMuted
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 3

            AppButton {
                theme: root.theme
                quiet: true
                text: root.updateService.downloading ? "Cancel download" : "Later"
                onClicked: {
                    if (root.updateService.downloading)
                        root.updateService.cancelDownload()
                    else
                        root.close()
                }
            }

            Item { Layout.fillWidth: true }

            AppButton {
                visible: root.updateService.hasError
                theme: root.theme
                text: "Check again"
                onClicked: root.updateService.checkForUpdates()
            }

            AppButton {
                visible: root.updateService.canDownload
                theme: root.theme
                primary: true
                text: "Download update"
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
}
