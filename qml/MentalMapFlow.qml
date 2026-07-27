// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var hierarchy
    required property var theme
    signal nodeSelected(var node)

    function nodeColor(index) {
        return [root.theme.accent, root.theme.secondaryAccent, root.theme.success,
            root.theme.danger, root.theme.textSecondary][index % 5]
    }

    function wash(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Flickable {
        anchors.fill: parent
        contentWidth: Math.max(width, flowStage.width)
        contentHeight: Math.max(height, flowStage.height)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: flowStage

            width: Math.max(1080, root.width)
            height: rootCard.height + branchColumn.height + 86
            spacing: 22

            Button {
                id: rootCard

                anchors.horizontalCenter: parent.horizontalCenter
                width: 280
                height: 58
                text: "Everything I’m working on"
                contentItem: Text {
                    text: rootCard.text
                    color: root.theme.textPrimary
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    radius: root.theme.radius + 8
                    color: root.theme.accentSoft
                    border.color: rootCard.activeFocus
                        ? root.theme.textPrimary : root.theme.accent
                    border.width: 2
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "↓"
                color: root.theme.accent
                font.pixelSize: 26
            }

            Column {
                id: branchColumn

                x: 28
                width: parent.width - 56
                spacing: 14

                Repeater {
                    model: root.hierarchy

                    delegate: Rectangle {
                        id: flowLane

                        required property var modelData
                        property color laneColor: root.nodeColor(modelData.colorIndex)

                        width: branchColumn.width
                        height: Math.max(88, childrenColumn.implicitHeight + 24)
                        radius: root.theme.radius + 5
                        color: root.wash(laneColor, root.theme.dark ? 0.07 : 0.05)
                        border.color: root.wash(laneColor, 0.32)

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Button {
                                id: branchNode

                                Layout.preferredWidth: 190
                                Layout.preferredHeight: 60
                                text: flowLane.modelData.title
                                onClicked: root.nodeSelected(flowLane.modelData)
                                contentItem: Column {
                                    spacing: 3
                                    Text {
                                        width: parent.width
                                        text: branchNode.text
                                        color: root.theme.textPrimary
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    Text {
                                        width: parent.width
                                        text: flowLane.modelData.kind.toUpperCase()
                                        color: flowLane.laneColor
                                        font.pixelSize: 8
                                        font.weight: Font.Bold
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                                background: Rectangle {
                                    radius: root.theme.radius + 4
                                    color: branchNode.hovered
                                        ? root.theme.surfaceHover : root.theme.surface
                                    border.color: branchNode.activeFocus
                                        ? root.theme.textPrimary : flowLane.laneColor
                                    border.width: branchNode.activeFocus ? 2 : 1
                                }
                            }

                            Text {
                                text: "➜"
                                color: flowLane.laneColor
                                font.pixelSize: 25
                            }

                            ColumnLayout {
                                id: childrenColumn

                                Layout.fillWidth: true
                                spacing: 8

                                Repeater {
                                    model: flowLane.modelData.children

                                    delegate: RowLayout {
                                        id: childFlow

                                        required property var modelData
                                        Layout.fillWidth: true
                                        spacing: 9

                                        Button {
                                            id: childNode

                                            Layout.preferredWidth: 190
                                            Layout.preferredHeight: 42
                                            text: childFlow.modelData.title
                                            onClicked: root.nodeSelected(childFlow.modelData)
                                            contentItem: Text {
                                                text: childNode.text
                                                color: root.theme.textPrimary
                                                font.pixelSize: 11
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                            }
                                            background: Rectangle {
                                                radius: root.theme.radius + 3
                                                color: childNode.hovered
                                                    ? root.theme.surfaceHover : root.theme.surfaceRaised
                                                border.color: childNode.activeFocus
                                                    ? flowLane.laneColor : root.theme.borderStrong
                                                border.width: childNode.activeFocus ? 2 : 1
                                            }
                                        }

                                        Text {
                                            visible: childFlow.modelData.childCount > 0
                                            text: "→"
                                            color: root.wash(flowLane.laneColor, 0.9)
                                            font.pixelSize: 21
                                        }

                                        Flow {
                                            Layout.fillWidth: true
                                            spacing: 6

                                            Repeater {
                                                model: childFlow.modelData.children

                                                delegate: Button {
                                                    id: leafNode

                                                    required property var modelData
                                                    width: Math.max(112, Math.min(190, implicitContentWidth + 24))
                                                    height: 34
                                                    text: modelData.title
                                                    onClicked: root.nodeSelected(modelData)
                                                    contentItem: Text {
                                                        text: leafNode.text
                                                        color: root.theme.textSecondary
                                                        font.pixelSize: 10
                                                        horizontalAlignment: Text.AlignHCenter
                                                        verticalAlignment: Text.AlignVCenter
                                                        elide: Text.ElideRight
                                                    }
                                                    background: Rectangle {
                                                        radius: 17
                                                        color: leafNode.hovered
                                                            ? root.theme.surfaceHover : root.theme.surface
                                                        border.color: leafNode.activeFocus
                                                            ? flowLane.laneColor : root.theme.border
                                                        border.width: leafNode.activeFocus ? 2 : 1
                                                    }
                                                }
                                            }
                                        }

                                        Text {
                                            visible: childFlow.modelData.childCount === 0
                                            Layout.fillWidth: true
                                            text: childFlow.modelData.detail
                                            color: root.theme.textMuted
                                            font.pixelSize: 10
                                            elide: Text.ElideRight
                                        }
                                    }
                                }

                                Text {
                                    visible: flowLane.modelData.childCount === 0
                                    text: "No active child nodes"
                                    color: root.theme.textMuted
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
