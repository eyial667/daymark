// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var taskModel
    required property var appSettings
    required property var theme
    property bool showSuggestionAction: false
    signal addRequested()
    signal suggestionRequested()
    signal completionRequested(int taskIndex, string taskTitle)
    signal categoryRequested(
        string taskId,
        string taskTitle,
        string categoryId,
        string subcategoryId)

    component MapNode: Rectangle {
        id: node

        required property string label
        property color nodeColor: root.theme.textSecondary
        property bool selected: false

        width: Math.max(116, nodeLabel.implicitWidth + 30)
        height: 40
        radius: 11
        color: selected ? root.theme.accentSoft : root.theme.surfaceRaised
        border.color: selected ? nodeColor : root.theme.borderStrong
        border.width: selected ? 2 : 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Rectangle {
                implicitWidth: 8
                implicitHeight: 8
                radius: 4
                color: node.nodeColor
            }
            Text {
                id: nodeLabel
                Layout.fillWidth: true
                text: node.label
                color: root.theme.textPrimary
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 2
                Text {
                    text: "Map"
                    color: root.theme.textPrimary
                    font.pixelSize: 23
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "Goals, categories, subcategories, and today’s work"
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }

            Item { Layout.fillWidth: true }

            AppButton {
                visible: root.showSuggestionAction
                theme: root.theme
                primary: true
                text: "Give me something to do"
                onClicked: root.suggestionRequested()
            }

            AppButton {
                theme: root.theme
                text: "+ Add task"
                onClicked: root.addRequested()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Rectangle {
                id: mapFrame

                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: root.theme.radius
                color: root.theme.background
                border.color: root.theme.border
                clip: true

                Canvas {
                    id: connections
                    anchors.fill: parent
                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.lineWidth = 1.4
                        ctx.strokeStyle = root.theme.borderStrong

                        function curve(x1, y1, x2, y2, color) {
                            ctx.beginPath()
                            ctx.strokeStyle = color || root.theme.borderStrong
                            ctx.moveTo(x1, y1)
                            ctx.bezierCurveTo((x1 + x2) / 2, y1, (x1 + x2) / 2, y2, x2, y2)
                            ctx.stroke()
                        }

                        const w = width
                        const h = height
                        curve(124, h * 0.50, w * 0.31, h * 0.36)
                        curve(124, h * 0.50, w * 0.31, h * 0.64)
                        curve(w * 0.31 + 125, h * 0.36, w * 0.55, h * 0.36, root.theme.accent)
                        curve(w * 0.31 + 125, h * 0.64, w * 0.55, h * 0.64)
                        curve(w * 0.55 + 145, h * 0.36, w - 168, h * 0.20, root.theme.accent)
                        curve(w * 0.55 + 145, h * 0.36, w - 168, h * 0.36)
                        curve(w * 0.55 + 145, h * 0.36, w - 168, h * 0.52)
                        curve(w * 0.55 + 145, h * 0.36, w - 168, h * 0.68)
                    }
                }

                MapNode {
                    x: 28
                    y: mapFrame.height * 0.50 - height / 2
                    label: "Life"
                    nodeColor: root.theme.secondaryAccent
                    selected: true
                }
                MapNode {
                    x: mapFrame.width * 0.31
                    y: mapFrame.height * 0.36 - height / 2
                    label: "Work"
                    nodeColor: root.theme.accent
                }
                MapNode {
                    x: mapFrame.width * 0.31
                    y: mapFrame.height * 0.64 - height / 2
                    label: "Growth"
                    nodeColor: root.theme.success
                }
                MapNode {
                    x: mapFrame.width * 0.55
                    y: mapFrame.height * 0.36 - height / 2
                    label: "Product launch"
                    nodeColor: root.theme.accent
                    selected: true
                }
                MapNode {
                    x: mapFrame.width * 0.55
                    y: mapFrame.height * 0.64 - height / 2
                    label: "Learn continuously"
                    nodeColor: root.theme.success
                }

                Repeater {
                    model: root.taskModel

                    delegate: MapNode {
                        required property int index
                        required property string title

                        visible: index < 4
                        x: mapFrame.width - width - 28
                        y: mapFrame.height * (0.20 + index * 0.16) - height / 2
                        label: title
                        nodeColor: index === 0 ? root.theme.danger : root.theme.accent
                        selected: index === 0
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 16
                    width: 230
                    height: 38
                    radius: 10
                    color: root.theme.surfaceRaised
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 18
                        Text { text: "↖"; color: root.theme.textSecondary; font.pixelSize: 13 }
                        Text { text: "−"; color: root.theme.textSecondary; font.pixelSize: 15 }
                        Text { text: "100%"; color: root.theme.textPrimary; font.pixelSize: 10 }
                        Text { text: "+"; color: root.theme.textSecondary; font.pixelSize: 15 }
                        Text { text: "⌗"; color: root.theme.textSecondary; font.pixelSize: 13 }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 310
                Layout.fillHeight: true
                radius: root.theme.radius
                color: root.theme.surface
                border.color: root.theme.border

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Today’s queue"
                            color: root.theme.textPrimary
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: root.taskModel.activeCount
                            color: root.theme.accent
                            font.pixelSize: 11
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(contentHeight, 310)
                        spacing: 5
                        clip: true
                        model: root.taskModel

                        delegate: TaskRow {
                            id: taskRow
                            width: ListView.view.width
                            theme: root.theme
                            compact: true
                            showReason: root.appSettings.showPriorityReasons
                            onCompletionRequested: rowIndex =>
                                root.completionRequested(rowIndex, taskRow.title)
                            onCategoryRequested: (taskId, categoryId, subcategoryId) =>
                                root.categoryRequested(
                                    taskId, taskRow.title, categoryId, subcategoryId)
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 1
                        color: root.theme.border
                        visible: root.appSettings.showPriorityReasons
                    }

                    Text {
                        text: "WHY THIS IS URGENT"
                        color: root.theme.textMuted
                        font.pixelSize: 9
                        font.weight: Font.Bold
                        font.letterSpacing: 1
                        visible: root.appSettings.showPriorityReasons
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.taskModel.activeCount > 0 ? root.taskModel.topTaskTitle : "No selected task"
                        color: root.theme.textPrimary
                        wrapMode: Text.WordWrap
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        visible: root.appSettings.showPriorityReasons
                    }

                    ColumnLayout {
                        visible: root.appSettings.showPriorityReasons
                            && root.taskModel.activeCount > 0
                        spacing: 8

                        Text {
                            text: "◉  Highest priority score"
                            color: root.theme.danger
                            font.pixelSize: 11
                        }
                        Text {
                            text: "↗  Connected to Product launch"
                            color: root.theme.accent
                            font.pixelSize: 11
                        }
                        Text {
                            text: "✓  Completing it keeps momentum"
                            color: root.theme.success
                            font.pixelSize: 11
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
