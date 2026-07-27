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
        const colors = [
            root.theme.accent,
            root.theme.secondaryAccent,
            root.theme.success,
            root.theme.danger,
            root.theme.textSecondary
        ]
        return colors[index % colors.length]
    }

    function wash(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Flickable {
        anchors.fill: parent
        contentWidth: Math.max(width, cloudStage.width)
        contentHeight: Math.max(height, cloudStage.height)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: cloudStage

            width: Math.max(980, root.width)
            height: cloudFlow.y + cloudFlow.height + 36
            spacing: 20

            Item {
                width: parent.width
                height: 116

                Rectangle {
                    anchors.centerIn: parent
                    width: 270
                    height: 78
                    radius: 39
                    color: root.theme.accentSoft
                    border.color: root.theme.accent
                    border.width: 2

                    Rectangle {
                        x: -24
                        y: 13
                        width: 68
                        height: 54
                        radius: 28
                        color: root.wash(root.theme.accent, 0.22)
                    }
                    Rectangle {
                        anchors.right: parent.right
                        anchors.rightMargin: -30
                        y: 8
                        width: 78
                        height: 62
                        radius: 32
                        color: root.wash(root.theme.secondaryAccent, 0.20)
                    }
                    Column {
                        anchors.centerIn: parent
                        spacing: 3
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "MY WORKING WORLD"
                            color: root.theme.accent
                            font.pixelSize: 10
                            font.weight: Font.Bold
                            font.letterSpacing: 1.4
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Everything in motion"
                            color: root.theme.textPrimary
                            font.pixelSize: 19
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            Flow {
                id: cloudFlow

                x: 24
                width: parent.width - 48
                height: implicitHeight
                spacing: 18

                Repeater {
                    model: root.hierarchy

                    delegate: Rectangle {
                        id: branchCloud

                        required property var modelData
                        property color branchColor: root.nodeColor(modelData.colorIndex)

                        width: modelData.childCount > 4 ? 380 : 315
                        height: branchContents.implicitHeight + 34
                        radius: 34
                        color: root.wash(branchColor, root.theme.dark ? 0.12 : 0.09)
                        border.color: root.wash(branchColor, 0.72)
                        border.width: 2

                        Rectangle {
                            x: -10
                            y: 22
                            width: 54
                            height: 54
                            radius: 27
                            color: root.wash(branchCloud.branchColor, 0.14)
                        }
                        Rectangle {
                            anchors.right: parent.right
                            anchors.rightMargin: -13
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 18
                            width: 72
                            height: 58
                            radius: 30
                            color: root.wash(branchCloud.branchColor, 0.12)
                        }

                        Column {
                            id: branchContents

                            x: 17
                            y: 15
                            width: parent.width - 34
                            spacing: 10

                            Button {
                                id: branchButton

                                width: parent.width
                                height: 48
                                text: branchCloud.modelData.title
                                leftPadding: 14
                                rightPadding: 14
                                onClicked: root.nodeSelected(branchCloud.modelData)

                                contentItem: RowLayout {
                                    spacing: 9
                                    Rectangle {
                                        implicitWidth: 10
                                        implicitHeight: 10
                                        radius: 5
                                        color: branchCloud.branchColor
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: branchButton.text
                                        color: root.theme.textPrimary
                                        font.pixelSize: 15
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        text: branchCloud.modelData.kind === "goal" ? "GOAL" : "AREA"
                                        color: branchCloud.branchColor
                                        font.pixelSize: 8
                                        font.weight: Font.Bold
                                        font.letterSpacing: 1
                                    }
                                }
                                background: Rectangle {
                                    radius: 24
                                    color: branchButton.hovered
                                        ? root.theme.surfaceHover : root.theme.surfaceRaised
                                    border.color: branchButton.activeFocus
                                        ? root.theme.accent : root.theme.borderStrong
                                    border.width: branchButton.activeFocus ? 2 : 1
                                }
                            }

                            Flow {
                                width: parent.width
                                height: implicitHeight
                                spacing: 7

                                Repeater {
                                    model: branchCloud.modelData.children

                                    delegate: Rectangle {
                                        id: childCloud

                                        required property var modelData
                                        property bool nested: modelData.childCount > 0

                                        width: nested ? branchContents.width : Math.min(
                                            branchContents.width,
                                            Math.max(132, childButton.implicitContentWidth + 30))
                                        height: nested ? nestedColumn.implicitHeight + 16 : 38
                                        radius: nested ? 20 : 19
                                        color: root.wash(branchCloud.branchColor, nested ? 0.10 : 0.16)
                                        border.color: root.wash(branchCloud.branchColor, 0.48)

                                        Column {
                                            id: nestedColumn

                                            x: childCloud.nested ? 8 : 0
                                            y: childCloud.nested ? 8 : 0
                                            width: childCloud.nested ? parent.width - 16 : parent.width
                                            spacing: 7

                                            Button {
                                                id: childButton

                                                width: parent.width
                                                height: 38
                                                text: childCloud.modelData.title
                                                onClicked: root.nodeSelected(childCloud.modelData)
                                                contentItem: Text {
                                                    text: childButton.text
                                                    color: root.theme.textPrimary
                                                    font.pixelSize: 11
                                                    font.weight: childCloud.nested ? Font.DemiBold : Font.Medium
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                    elide: Text.ElideRight
                                                }
                                                background: Rectangle {
                                                    radius: 19
                                                    color: childButton.hovered
                                                        ? root.theme.surfaceHover : root.theme.surface
                                                    border.color: childButton.activeFocus
                                                        ? branchCloud.branchColor : "transparent"
                                                    border.width: childButton.activeFocus ? 2 : 0
                                                }
                                            }

                                            Flow {
                                                visible: childCloud.nested
                                                width: parent.width
                                                height: visible ? implicitHeight : 0
                                                spacing: 5

                                                Repeater {
                                                    model: childCloud.modelData.children

                                                    delegate: Button {
                                                        id: leafButton

                                                        required property var modelData
                                                        width: Math.min(
                                                            nestedColumn.width,
                                                            Math.max(118, implicitContentWidth + 24))
                                                        height: 32
                                                        text: modelData.title
                                                        onClicked: root.nodeSelected(modelData)
                                                        contentItem: Text {
                                                            text: leafButton.text
                                                            color: root.theme.textSecondary
                                                            font.pixelSize: 10
                                                            horizontalAlignment: Text.AlignHCenter
                                                            verticalAlignment: Text.AlignVCenter
                                                            elide: Text.ElideRight
                                                        }
                                                        background: Rectangle {
                                                            radius: 16
                                                            color: leafButton.hovered
                                                                ? root.theme.surfaceHover : root.theme.surfaceRaised
                                                            border.color: leafButton.activeFocus
                                                                ? branchCloud.branchColor : root.theme.border
                                                            border.width: leafButton.activeFocus ? 2 : 1
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
