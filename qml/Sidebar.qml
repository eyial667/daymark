// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar

    required property var theme
    required property var appSettings
    property int currentIndex: 0
    signal pageRequested(int index)
    signal addRequested()

    color: theme.sidebar
    implicitWidth: 210
    border.color: theme.border
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 5

        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 22
            spacing: 10

            Rectangle {
                implicitWidth: 34
                implicitHeight: 34
                radius: sidebar.theme.radius
                color: sidebar.theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "D"
                    color: "white"
                    font.pixelSize: 17
                    font.weight: Font.Bold
                }
            }

            Text {
                text: "Daymark"
                color: sidebar.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
        }

        Repeater {
            model: [
                { name: "Today", glyph: "◫" },
                { name: "Inbox", glyph: "▱" },
                { name: "Map", glyph: "⌘" },
                { name: "Projects", glyph: "□" },
                { name: "Focus", glyph: "◎" },
                { name: "Review", glyph: "✓" },
                { name: "Settings", glyph: "⚙" }
            ]

            delegate: Button {
                id: navigationButton

                required property int index
                required property var modelData

                Layout.fillWidth: true
                implicitHeight: 42
                text: modelData.name
                leftPadding: 14

                contentItem: RowLayout {
                    spacing: 11

                    Text {
                        text: navigationButton.modelData.glyph
                        color: sidebar.currentIndex === navigationButton.index
                            ? sidebar.theme.accent
                            : sidebar.theme.textMuted
                        font.pixelSize: 15
                        Layout.preferredWidth: 17
                    }

                    Text {
                        Layout.fillWidth: true
                        text: navigationButton.text
                        color: sidebar.currentIndex === navigationButton.index
                            ? sidebar.theme.textPrimary
                            : sidebar.theme.textSecondary
                        font.pixelSize: 13
                        font.weight: sidebar.currentIndex === navigationButton.index
                            ? Font.DemiBold
                            : Font.Normal
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                background: Rectangle {
                    radius: sidebar.theme.radius
                    color: sidebar.currentIndex === navigationButton.index
                        ? sidebar.theme.accentSoft
                        : "transparent"
                    border.color: sidebar.currentIndex === navigationButton.index
                        ? sidebar.theme.borderStrong
                        : "transparent"
                }

                onClicked: sidebar.pageRequested(navigationButton.index)
            }
        }

        Item { Layout.fillHeight: true }

        AppButton {
            Layout.fillWidth: true
            theme: sidebar.theme
            primary: true
            text: "+  Add task"
            onClicked: sidebar.addRequested()
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            Layout.topMargin: 10
            Layout.bottomMargin: 8
            color: sidebar.theme.border
        }

        Text {
            text: "INTERFACE"
            color: sidebar.theme.textMuted
            font.pixelSize: 9
            font.weight: Font.Bold
            font.letterSpacing: 1.2
        }

        ComboBox {
            id: styleSelector

            Layout.fillWidth: true
            implicitHeight: 38
            model: [
                "A · Midnight Command",
                "B · Spatial Map",
                "C · Quiet Focus",
                "D · Daymark Hybrid"
            ]
            currentIndex: Number(sidebar.appSettings.interfaceStyle)
            onActivated: sidebar.appSettings.interfaceStyle = currentIndex

            contentItem: Text {
                leftPadding: 10
                rightPadding: 24
                text: styleSelector.displayText
                color: sidebar.theme.textPrimary
                font.pixelSize: 11
                font.weight: Font.Medium
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                radius: sidebar.theme.radius
                color: styleSelector.hovered
                    ? sidebar.theme.surfaceHover
                    : sidebar.theme.surface
                border.color: sidebar.theme.border
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: sidebar.theme.modeName + " · Local-first"
                color: sidebar.theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }

            Button {
                id: colorModeButton

                implicitWidth: 30
                implicitHeight: 30
                padding: 0
                ToolTip.visible: hovered
                ToolTip.text: sidebar.appSettings.colorMode === 0
                    ? "Switch to light mode"
                    : "Switch to dark mode"

                contentItem: Text {
                    text: sidebar.appSettings.colorMode === 0 ? "☀" : "☾"
                    color: sidebar.theme.textSecondary
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: sidebar.theme.radius
                    color: colorModeButton.hovered
                        ? sidebar.theme.surfaceHover
                        : "transparent"
                    border.color: colorModeButton.hovered
                        ? sidebar.theme.border
                        : "transparent"
                }

                onClicked: sidebar.appSettings.colorMode =
                    sidebar.appSettings.colorMode === 0 ? 1 : 0
            }
        }
    }
}
