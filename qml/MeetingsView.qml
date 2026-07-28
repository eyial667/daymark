// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var meetingModel
    required property var theme

    property int pendingRemovalRow: -1
    property string pendingRemovalTitle: ""

    function prepareMeetingDialog() {
        const suggested = new Date(Date.now() + 60 * 60 * 1000)
        suggested.setMinutes(Math.ceil(suggested.getMinutes() / 15) * 15, 0, 0)
        titleField.clear()
        notesField.clear()
        dateField.text = Qt.formatDate(suggested, "yyyy-MM-dd")
        timeField.text = Qt.formatTime(suggested, "HH:mm")
        meetingDialog.open()
    }

    ScrollView {
        id: meetingScroll

        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Item {
            width: meetingScroll.availableWidth
            implicitHeight: pageColumn.implicitHeight + 64

            ColumnLayout {
                id: pageColumn

                width: Math.min(920, parent.width - 56)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 28
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 18

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTr("Meetings")
                            color: root.theme.textPrimary
                            font.pixelSize: 28
                            font.weight: Font.DemiBold
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Schedule commitments and receive a local notification 10 minutes before they begin.")
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }

                    AppButton {
                        theme: root.theme
                        primary: true
                        text: qsTr("+  Add meeting")
                        onClicked: root.prepareMeetingDialog()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    MetricCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        theme: root.theme
                        label: qsTr("UPCOMING")
                        value: root.meetingModel.meetingCount.toString()
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 2
                        implicitHeight: 84
                        radius: root.theme.radius + 1
                        color: root.theme.surface
                        border.color: root.theme.border

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 4

                            Text {
                                text: qsTr("NEXT MEETING")
                                color: root.theme.textMuted
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 1.1
                            }
                            Text {
                                Layout.fillWidth: true
                                text: root.meetingModel.nextMeetingTitle
                                color: root.theme.textPrimary
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: root.meetingModel.nextMeetingWhen
                                color: root.theme.textSecondary
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: statusText.implicitHeight + 22
                    visible: root.meetingModel.statusMessage.length > 0
                    radius: root.theme.radius
                    color: root.theme.accentSoft
                    border.color: root.theme.borderStrong

                    Text {
                        id: statusText
                        anchors.fill: parent
                        anchors.margins: 11
                        text: root.meetingModel.statusMessage
                        color: root.theme.textPrimary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.meetingModel.clearStatus()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 220
                    visible: root.meetingModel.meetingCount === 0
                    radius: root.theme.radius + 2
                    color: root.theme.surface
                    border.color: root.theme.border

                    ColumnLayout {
                        anchors.centerIn: parent
                        width: Math.min(500, parent.width - 48)
                        spacing: 9

                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            implicitWidth: 50
                            implicitHeight: 50
                            radius: 25
                            color: root.theme.accentSoft
                            border.color: root.theme.borderStrong

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("◷")
                                color: root.theme.accent
                                font.pixelSize: 23
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("No upcoming meetings")
                            color: root.theme.textPrimary
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Add a dated meeting and Daymark will remind you locally 10 minutes before it starts while the app is running.")
                            color: root.theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }

                Repeater {
                    model: root.meetingModel

                    delegate: Rectangle {
                        id: meetingCard

                        required property int index
                        required property string title
                        required property string notes
                        required property string startsAtText
                        required property string reminderText

                        Layout.fillWidth: true
                        implicitHeight: cardContent.implicitHeight + 30
                        radius: root.theme.radius + 2
                        color: root.theme.surface
                        border.color: root.theme.border

                        RowLayout {
                            id: cardContent

                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 14

                            Rectangle {
                                Layout.alignment: Qt.AlignTop
                                implicitWidth: 42
                                implicitHeight: 42
                                radius: 21
                                color: root.theme.accentSoft
                                border.color: root.theme.borderStrong

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("◷")
                                    color: root.theme.accent
                                    font.pixelSize: 18
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Text {
                                    Layout.fillWidth: true
                                    text: meetingCard.title
                                    color: root.theme.textPrimary
                                    font.pixelSize: 16
                                    font.weight: Font.DemiBold
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: meetingCard.startsAtText
                                    color: root.theme.textSecondary
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    visible: meetingCard.notes.length > 0
                                    text: meetingCard.notes
                                    color: root.theme.textMuted
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: meetingCard.reminderText
                                    color: root.theme.accent
                                    font.pixelSize: 10
                                    font.weight: Font.Medium
                                }
                            }

                            AppButton {
                                theme: root.theme
                                quiet: true
                                text: qsTr("Remove")
                                onClicked: {
                                    root.pendingRemovalRow = meetingCard.index
                                    root.pendingRemovalTitle = meetingCard.title
                                    removeDialog.open()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: meetingDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(520, root.width - 36)
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
            spacing: 11

            Text {
                text: qsTr("Add a meeting")
                color: root.theme.textPrimary
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            Text {
                text: qsTr("TITLE")
                color: root.theme.textMuted
                font.pixelSize: 9
                font.weight: Font.Bold
            }
            TextField {
                id: titleField
                Layout.fillWidth: true
                placeholderText: qsTr("Meeting title")
                selectByMouse: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Text {
                        text: qsTr("DATE")
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }
                    DateField {
                        id: dateField
                        Layout.fillWidth: true
                        theme: root.theme
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 130
                    spacing: 5

                    Text {
                        text: qsTr("TIME")
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }
                    TextField {
                        id: timeField
                        Layout.fillWidth: true
                        placeholderText: qsTr("HH:MM")
                        maximumLength: 5
                        inputMethodHints: Qt.ImhTime
                        selectByMouse: true
                    }
                }
            }

            Text {
                text: qsTr("NOTES")
                color: root.theme.textMuted
                font.pixelSize: 9
                font.weight: Font.Bold
            }
            TextArea {
                id: notesField
                Layout.fillWidth: true
                Layout.preferredHeight: 86
                placeholderText: qsTr("Optional location, call link, or context")
                wrapMode: TextArea.Wrap
                selectByMouse: true
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Daymark sends one local reminder 10 minutes before the start time. Keep the app running to receive it.")
                color: root.theme.textMuted
                font.pixelSize: 10
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
                    onClicked: meetingDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Add meeting")
                    onClicked: {
                        if (root.meetingModel.addMeeting(
                                titleField.text,
                                dateField.text,
                                timeField.text,
                                notesField.text)) {
                            meetingDialog.close()
                        }
                    }
                }
            }
        }

        onOpened: titleField.forceActiveFocus()
    }

    Dialog {
        id: removeDialog

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(420, root.width - 36)
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
                text: qsTr("Remove this meeting?")
                color: root.theme.textPrimary
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: root.pendingRemovalTitle
                color: root.theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    theme: root.theme
                    quiet: true
                    text: qsTr("Cancel")
                    onClicked: removeDialog.close()
                }
                AppButton {
                    theme: root.theme
                    primary: true
                    text: qsTr("Remove")
                    onClicked: {
                        if (root.meetingModel.removeMeeting(root.pendingRemovalRow))
                            removeDialog.close()
                    }
                }
            }
        }

        onClosed: {
            root.pendingRemovalRow = -1
            root.pendingRemovalTitle = ""
        }
    }
}
