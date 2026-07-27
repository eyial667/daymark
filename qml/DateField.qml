// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: control

    required property var theme
    property alias text: dateInput.text
    property string placeholderText: qsTr("YYYY-MM-DD")
    property int displayedMonth: new Date().getMonth()
    property int displayedYear: new Date().getFullYear()

    signal accepted()

    implicitWidth: 230
    implicitHeight: 40

    function clear() {
        dateInput.clear()
    }

    function parsedDate() {
        const match = /^(\d{4})-(\d{2})-(\d{2})$/.exec(dateInput.text.trim())
        if (!match)
            return null

        const year = Number(match[1])
        const month = Number(match[2]) - 1
        const day = Number(match[3])
        const candidate = new Date(year, month, day, 12, 0, 0)
        if (candidate.getFullYear() !== year
                || candidate.getMonth() !== month
                || candidate.getDate() !== day) {
            return null
        }
        return candidate
    }

    function openCalendar() {
        const current = parsedDate()
        const initial = current || new Date()
        displayedMonth = initial.getMonth()
        displayedYear = initial.getFullYear()
        calendarPopup.open()
    }

    function shiftMonth(offset) {
        const shifted = new Date(displayedYear, displayedMonth + offset, 1, 12, 0, 0)
        displayedMonth = shifted.getMonth()
        displayedYear = shifted.getFullYear()
    }

    function dateForCell(index) {
        const first = new Date(displayedYear, displayedMonth, 1, 12, 0, 0)
        const mondayBasedOffset = (first.getDay() + 6) % 7
        return new Date(
            displayedYear,
            displayedMonth,
            1 - mondayBasedOffset + index,
            12,
            0,
            0)
    }

    function selectDate(date) {
        dateInput.text = Qt.formatDate(date, "yyyy-MM-dd")
        displayedMonth = date.getMonth()
        displayedYear = date.getFullYear()
        calendarPopup.close()
        dateInput.forceActiveFocus()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 6

        TextField {
            id: dateInput

            Layout.fillWidth: true
            Layout.fillHeight: true
            placeholderText: control.placeholderText
            maximumLength: 10
            selectByMouse: true
            font.pixelSize: 13
            onAccepted: control.accepted()
        }

        Button {
            id: calendarButton

            Layout.preferredWidth: 40
            Layout.fillHeight: true
            padding: 0
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Choose from calendar")

            contentItem: Text {
                text: qsTr("▦")
                color: control.theme.accent
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: control.theme.radius
                color: calendarButton.pressed
                    ? control.theme.accentSoft
                    : calendarButton.hovered
                    ? control.theme.surfaceHover
                    : control.theme.surface
                border.color: calendarPopup.opened
                    ? control.theme.accent
                    : control.theme.border
            }

            onClicked: control.openCalendar()
        }
    }

    Popup {
        id: calendarPopup

        x: Math.max(0, control.width - width)
        y: control.height + 6
        width: 330
        padding: 12
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: control.theme.radius + 3
            color: control.theme.surfaceRaised
            border.color: control.theme.borderStrong
        }

        contentItem: ColumnLayout {
            spacing: 9

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Button {
                    implicitWidth: 34
                    implicitHeight: 32
                    text: qsTr("‹")
                    onClicked: control.shiftMonth(-1)
                }

                Text {
                    Layout.fillWidth: true
                    text: Qt.formatDate(
                        new Date(
                            control.displayedYear,
                            control.displayedMonth,
                            1,
                            12,
                            0,
                            0),
                        qsTr("MMMM yyyy"))
                    color: control.theme.textPrimary
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    implicitWidth: 34
                    implicitHeight: 32
                    text: qsTr("›")
                    onClicked: control.shiftMonth(1)
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 7
                columnSpacing: 3
                rowSpacing: 3

                Repeater {
                    model: [qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"),
                        qsTr("Fri"), qsTr("Sat"), qsTr("Sun")]

                    delegate: Text {
                        required property var modelData

                        Layout.fillWidth: true
                        Layout.preferredHeight: 22
                        text: modelData
                        color: control.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Repeater {
                    model: 42

                    delegate: Button {
                        id: dayButton

                        required property int index
                        property var cellDate: control.dateForCell(index)
                        property bool inDisplayedMonth:
                            cellDate.getMonth() === control.displayedMonth
                        property bool selected:
                            control.text === Qt.formatDate(cellDate, "yyyy-MM-dd")
                        property bool today:
                            Qt.formatDate(cellDate, "yyyy-MM-dd")
                                === Qt.formatDate(new Date(), "yyyy-MM-dd")

                        Layout.fillWidth: true
                        Layout.preferredHeight: 34
                        padding: 0

                        contentItem: Text {
                            text: dayButton.cellDate.getDate()
                            color: dayButton.selected
                                ? "#FFFFFF"
                                : dayButton.inDisplayedMonth
                                ? control.theme.textPrimary
                                : control.theme.textMuted
                            opacity: dayButton.inDisplayedMonth ? 1.0 : 0.55
                            font.pixelSize: 11
                            font.weight: dayButton.selected || dayButton.today
                                ? Font.Bold
                                : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: control.theme.radius
                            color: dayButton.selected
                                ? control.theme.accent
                                : dayButton.hovered
                                ? control.theme.surfaceHover
                                : dayButton.today
                                ? control.theme.accentSoft
                                : "transparent"
                            border.color: dayButton.today && !dayButton.selected
                                ? control.theme.borderStrong
                                : "transparent"
                        }

                        onClicked: control.selectDate(cellDate)
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: control.theme.border
            }

            RowLayout {
                Layout.fillWidth: true

                AppButton {
                    theme: control.theme
                    quiet: true
                    text: qsTr("Clear")
                    onClicked: {
                        control.clear()
                        calendarPopup.close()
                    }
                }

                Item { Layout.fillWidth: true }

                AppButton {
                    theme: control.theme
                    text: qsTr("Today")
                    onClicked: control.selectDate(new Date())
                }
            }
        }
    }
}
