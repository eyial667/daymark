// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var dailyNoteModel
    required property var theme
    signal closeRequested()

    Rectangle {
        anchors.fill: parent
        color: root.theme.shadow
        opacity: root.theme.dark ? 0.46 : 0.24

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.dailyNoteModel.saveNow()
                root.closeRequested()
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 14
        width: Math.min(430, parent.width - 28)
        radius: root.theme.radius + 6
        color: root.theme.surfaceRaised
        border.color: root.theme.accent
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "Today’s note"
                        color: root.theme.textPrimary
                        font.pixelSize: 23
                        font.weight: Font.DemiBold
                    }
                    Text {
                        text: Qt.formatDate(new Date(), "dddd, MMMM d")
                        color: root.theme.textMuted
                        font.pixelSize: 11
                    }
                }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: "Close"
                    onClicked: {
                        root.dailyNoteModel.saveNow()
                        root.closeRequested()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 54
                radius: root.theme.radius
                color: root.theme.accentSoft
                border.color: root.theme.borderStrong

                Text {
                    anchors.fill: parent
                    anchors.margins: 11
                    text: "Capture decisions, loose thoughts, and context worth carrying into tomorrow’s review."
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }
            }

            TextArea {
                id: noteEditor

                Layout.fillWidth: true
                Layout.fillHeight: true
                text: root.dailyNoteModel.todayText
                placeholderText: "What happened today? What should tomorrow-you remember?"
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                color: root.theme.textPrimary
                placeholderTextColor: root.theme.textMuted
                font.pixelSize: 14
                leftPadding: 15
                rightPadding: 15
                topPadding: 14
                bottomPadding: 14
                onTextChanged: {
                    if (root.dailyNoteModel.todayText !== text)
                        root.dailyNoteModel.todayText = text
                }
                onActiveFocusChanged: {
                    if (!activeFocus)
                        root.dailyNoteModel.saveNow()
                }

                background: Rectangle {
                    radius: root.theme.radius + 2
                    color: root.theme.surface
                    border.color: noteEditor.activeFocus
                        ? root.theme.accent : root.theme.borderStrong
                    border.width: noteEditor.activeFocus ? 2 : 1
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: root.dailyNoteModel.dirty
                        ? "Saving…" : "Saved locally · today + yesterday only"
                    color: root.dailyNoteModel.dirty
                        ? root.theme.accent : root.theme.textMuted
                    font.pixelSize: 10
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: "Save note"
                    onClicked: root.dailyNoteModel.saveNow()
                }
            }
        }
    }

    Connections {
        target: root.dailyNoteModel
        function onTodayTextChanged() {
            if (!noteEditor.activeFocus
                    && noteEditor.text !== root.dailyNoteModel.todayText) {
                noteEditor.text = root.dailyNoteModel.todayText
            }
        }
    }
}
