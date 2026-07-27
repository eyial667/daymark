// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var flatNodes
    required property int mapRevision
    required property var theme
    signal nodeSelected(var node)

    function nodeColor(index) {
        return [root.theme.accent, root.theme.secondaryAccent, root.theme.success,
            root.theme.danger, root.theme.textSecondary][index % 5]
    }

    function wash(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    function nodeX(node) {
        return width / 2 + Math.cos(node.angle) * width * 0.88 * node.radius
    }

    function nodeY(node) {
        return height / 2 + Math.sin(node.angle) * height * 0.86 * node.radius
    }

    function nodeById(nodeId) {
        for (let index = 0; index < flatNodes.length; ++index) {
            if (flatNodes[index].nodeId === nodeId)
                return flatNodes[index]
        }
        return null
    }

    onMapRevisionChanged: constellationCanvas.requestPaint()
    onWidthChanged: constellationCanvas.requestPaint()
    onHeightChanged: constellationCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radius + 8
        color: root.theme.dark ? root.theme.sidebar : root.theme.surfaceRaised
        border.color: root.theme.borderStrong
    }

    Canvas {
        id: constellationCanvas

        anchors.fill: parent
        renderTarget: Canvas.FramebufferObject

        onPaint: {
            const context = getContext("2d")
            context.reset()
            context.clearRect(0, 0, width, height)

            context.fillStyle = root.wash(root.theme.textMuted, root.theme.dark ? 0.28 : 0.18)
            for (let index = 0; index < 70; ++index) {
                const x = (index * 83 + 29) % Math.max(1, width)
                const y = (index * 47 + 61) % Math.max(1, height)
                const radius = index % 9 === 0 ? 1.6 : 0.8
                context.beginPath()
                context.arc(x, y, radius, 0, Math.PI * 2)
                context.fill()
            }

            for (let index = 0; index < root.flatNodes.length; ++index) {
                const node = root.flatNodes[index]
                const parent = node.parentId.length > 0
                    ? root.nodeById(node.parentId) : null
                const fromX = parent ? root.nodeX(parent) : width / 2
                const fromY = parent ? root.nodeY(parent) : height / 2
                const toX = root.nodeX(node)
                const toY = root.nodeY(node)
                const accent = root.nodeColor(node.colorIndex)

                context.beginPath()
                context.moveTo(fromX, fromY)
                const bend = (toX - fromX) * 0.18
                context.bezierCurveTo(
                    fromX + bend,
                    fromY,
                    toX - bend,
                    toY,
                    toX,
                    toY)
                context.strokeStyle = root.wash(accent, node.depth === 1 ? 0.66 : 0.34)
                context.lineWidth = node.depth === 1 ? 1.8 : 1.0
                context.stroke()
            }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: 162
        height: 74
        radius: 37
        color: root.theme.accentSoft
        border.color: root.theme.accent
        border.width: 2

        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 24
            height: parent.height + 24
            radius: height / 2
            color: "transparent"
            border.color: root.wash(root.theme.accent, 0.25)
        }

        Column {
            anchors.centerIn: parent
            spacing: 2
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "DAYMARK"
                color: root.theme.accent
                font.pixelSize: 9
                font.weight: Font.Bold
                font.letterSpacing: 1.7
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "My working world"
                color: root.theme.textPrimary
                font.pixelSize: 14
                font.weight: Font.DemiBold
            }
        }
    }

    Repeater {
        model: root.flatNodes

        delegate: Item {
            id: starNode

            required property var modelData
            property color starColor: root.nodeColor(modelData.colorIndex)
            property real nodeWidth: modelData.depth === 1 ? 142
                : modelData.depth === 2 ? 118 : 100
            property real nodeHeight: modelData.depth === 1 ? 54
                : modelData.depth === 2 ? 42 : 34

            x: root.nodeX(modelData) - nodeWidth / 2
            y: root.nodeY(modelData) - nodeHeight / 2
            width: nodeWidth
            height: nodeHeight

            Rectangle {
                anchors.centerIn: parent
                width: parent.width + (starNode.modelData.depth === 1 ? 18 : 10)
                height: parent.height + (starNode.modelData.depth === 1 ? 18 : 10)
                radius: height / 2
                color: root.wash(starNode.starColor, starNode.modelData.depth === 1 ? 0.16 : 0.08)
            }

            Button {
                id: nodeButton

                anchors.fill: parent
                text: starNode.modelData.title
                onClicked: root.nodeSelected(starNode.modelData)
                contentItem: Column {
                    anchors.centerIn: parent
                    width: parent.width - 14
                    spacing: 1
                    Text {
                        width: parent.width
                        text: nodeButton.text
                        color: root.theme.textPrimary
                        font.pixelSize: starNode.modelData.depth === 1 ? 11 : 9
                        font.weight: starNode.modelData.depth === 1
                            ? Font.DemiBold : Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                    Text {
                        visible: starNode.modelData.depth < 3
                        width: parent.width
                        text: starNode.modelData.kind.toUpperCase()
                        color: starNode.starColor
                        font.pixelSize: 7
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
                background: Rectangle {
                    radius: height / 2
                    color: nodeButton.hovered
                        ? root.theme.surfaceHover : root.theme.surfaceRaised
                    border.color: nodeButton.activeFocus
                        ? root.theme.textPrimary : starNode.starColor
                    border.width: nodeButton.activeFocus || starNode.modelData.depth === 1 ? 2 : 1
                }
            }
        }
    }

    Text {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 12
        text: "CONSTELLATION · select any orbit to inspect it"
        color: root.theme.textMuted
        font.pixelSize: 8
        font.weight: Font.Bold
        font.letterSpacing: 1
    }
}
