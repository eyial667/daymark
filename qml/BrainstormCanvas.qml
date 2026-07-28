// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var mapModel
    required property var theme
    required property string viewKind
    property string selectedType: ""
    property string selectedId: ""
    property real panX: width / 2 - 420
    property real panY: height / 2 - 280
    property real zoom: 1.0
    signal objectSelected(string objectType, var objectData)
    signal selectionCleared()
    signal addNoteRequested(string groupId, real x, real y)

    function colorFor(name) {
        return name === "secondary" ? root.theme.secondaryAccent
            : name === "success" ? root.theme.success
            : name === "danger" ? root.theme.danger
            : name === "neutral" ? root.theme.textSecondary
            : root.theme.accent
    }

    function wash(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    function groupById(groupId) {
        for (let index = 0; index < root.mapModel.groups.length; ++index) {
            const group = root.mapModel.groups[index]
            if (group.groupId === groupId)
                return group
        }
        return null
    }

    function noteById(noteId) {
        for (let index = 0; index < root.mapModel.notes.length; ++index) {
            const note = root.mapModel.notes[index]
            if (note.noteId === noteId)
                return note
        }
        return null
    }

    function noteWorldX(note) {
        const group = groupById(note.groupId)
        return group ? group.x + note.x + 72 : 0
    }

    function noteWorldY(note) {
        const group = groupById(note.groupId)
        return group ? group.y + note.y + 28 : 0
    }

    function viewportCenter() {
        return Qt.point(
            (root.width / 2 - root.panX) / root.zoom,
            (root.height / 2 - root.panY) / root.zoom)
    }

    function suggestedNotePosition(groupId) {
        const group = groupById(groupId)
        if (!group)
            return Qt.point(24, 72)
        const count = Math.max(0, group.noteCount - (root.viewKind === "constellation" ? 1 : 0))
        if (root.viewKind === "cloud") {
            return Qt.point(20 + (count % 2) * 168, 72 + Math.floor(count / 2) * 76)
        }
        const angle = count * 2.399963
        const radius = 120 + (count % 3) * 34
        return Qt.point(
            group.width / 2 - 72 + Math.cos(angle) * radius,
            group.height / 2 - 28 + Math.sin(angle) * radius)
    }

    function zoomBy(factor, anchorX, anchorY) {
        const oldZoom = root.zoom
        const nextZoom = Math.max(0.28, Math.min(2.6, oldZoom * factor))
        const worldX = (anchorX - root.panX) / oldZoom
        const worldY = (anchorY - root.panY) / oldZoom
        root.zoom = nextZoom
        root.panX = anchorX - worldX * nextZoom
        root.panY = anchorY - worldY * nextZoom
    }

    function resetView() {
        root.zoom = 1.0
        root.panX = root.width / 2 - 420
        root.panY = root.height / 2 - 280
    }

    onPanXChanged: edgeCanvas.requestPaint()
    onPanYChanged: edgeCanvas.requestPaint()
    onZoomChanged: edgeCanvas.requestPaint()
    onWidthChanged: edgeCanvas.requestPaint()
    onHeightChanged: edgeCanvas.requestPaint()

    Connections {
        target: root.mapModel
        function onMapChanged() { edgeCanvas.requestPaint() }
    }

    Rectangle {
        anchors.fill: parent
        color: root.viewKind === "constellation"
            ? (root.theme.dark ? root.theme.sidebar : root.theme.surfaceRaised)
            : root.theme.background
    }

    Canvas {
        id: gridCanvas
        anchors.fill: parent
        onPaint: {
            const context = getContext("2d")
            context.reset()
            context.clearRect(0, 0, width, height)
            const spacing = 42 * root.zoom
            if (spacing < 12)
                return
            const offsetX = ((root.panX % spacing) + spacing) % spacing
            const offsetY = ((root.panY % spacing) + spacing) % spacing
            context.fillStyle = root.wash(root.theme.textMuted,
                root.viewKind === "constellation" ? 0.22 : 0.13)
            for (let x = offsetX; x < width; x += spacing) {
                for (let y = offsetY; y < height; y += spacing) {
                    context.beginPath()
                    context.arc(x, y, root.viewKind === "constellation" ? 1.1 : 0.8,
                        0, Math.PI * 2)
                    context.fill()
                }
            }
        }
        Connections {
            target: root
            function onPanXChanged() { gridCanvas.requestPaint() }
            function onPanYChanged() { gridCanvas.requestPaint() }
            function onZoomChanged() { gridCanvas.requestPaint() }
        }
    }

    MouseArea {
        id: panArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        property real previousX: 0
        property real previousY: 0
        onPressed: mouse => {
            previousX = mouse.x
            previousY = mouse.y
            root.selectionCleared()
        }
        onPositionChanged: mouse => {
            if (!pressed)
                return
            root.panX += mouse.x - previousX
            root.panY += mouse.y - previousY
            previousX = mouse.x
            previousY = mouse.y
        }
        onWheel: wheel => {
            root.zoomBy(wheel.angleDelta.y > 0 ? 1.12 : 0.89, wheel.x, wheel.y)
            wheel.accepted = true
        }
    }

    Canvas {
        id: edgeCanvas
        anchors.fill: parent

        onPaint: {
            const context = getContext("2d")
            context.reset()
            context.clearRect(0, 0, width, height)
            for (let index = 0; index < root.mapModel.connections.length; ++index) {
                const edge = root.mapModel.connections[index]
                if (edge.viewKind !== root.viewKind)
                    continue
                const source = root.noteById(edge.sourceNoteId)
                const target = root.noteById(edge.targetNoteId)
                if (!source || !target)
                    continue
                const fromX = root.panX + root.noteWorldX(source) * root.zoom
                const fromY = root.panY + root.noteWorldY(source) * root.zoom
                const toX = root.panX + root.noteWorldX(target) * root.zoom
                const toY = root.panY + root.noteWorldY(target) * root.zoom
                const dx = toX - fromX
                const dy = toY - fromY
                const length = Math.max(1, Math.sqrt(dx * dx + dy * dy))
                const unitX = dx / length
                const unitY = dy / length
                const endX = toX - unitX * 52 * root.zoom
                const endY = toY - unitY * 25 * root.zoom
                const color = root.colorFor(source.color)
                context.strokeStyle = root.wash(color,
                    root.selectedType === "connection"
                        && root.selectedId === edge.connectionId ? 0.98 : 0.62)
                context.fillStyle = context.strokeStyle
                context.lineWidth = (root.selectedId === edge.connectionId ? 2.5 : 1.5)
                    * Math.max(0.7, root.zoom)
                context.beginPath()
                context.moveTo(fromX, fromY)
                context.lineTo(endX, endY)
                context.stroke()
                const arrowSize = 9 * Math.max(0.75, root.zoom)
                context.beginPath()
                context.moveTo(endX, endY)
                context.lineTo(
                    endX - unitX * arrowSize - unitY * arrowSize * 0.6,
                    endY - unitY * arrowSize + unitX * arrowSize * 0.6)
                context.lineTo(
                    endX - unitX * arrowSize + unitY * arrowSize * 0.6,
                    endY - unitY * arrowSize - unitX * arrowSize * 0.6)
                context.closePath()
                context.fill()
            }
        }
    }

    Item {
        id: world
        x: root.panX
        y: root.panY
        scale: root.zoom
        transformOrigin: Item.TopLeft

        Repeater {
            model: root.mapModel.groups

            delegate: Item {
                id: groupItem
                required property var modelData
                property color groupColor: root.colorFor(modelData.color)
                visible: modelData.kind === root.viewKind
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height

                Rectangle {
                    anchors.fill: parent
                    visible: root.viewKind === "cloud"
                    radius: 34
                    color: root.wash(groupItem.groupColor, root.theme.dark ? 0.14 : 0.09)
                    border.color: root.selectedType === "group"
                        && root.selectedId === groupItem.modelData.groupId
                        ? root.theme.textPrimary : root.wash(groupItem.groupColor, 0.78)
                    border.width: root.selectedType === "group"
                        && root.selectedId === groupItem.modelData.groupId ? 3 : 2
                }

                Rectangle {
                    id: groupHeader
                    x: root.viewKind === "cloud" ? 14 : (parent.width - width) / 2
                    y: root.viewKind === "cloud" ? 12 : -38
                    width: root.viewKind === "cloud" ? parent.width - 28 : 220
                    height: 44
                    radius: 22
                    color: root.theme.surfaceRaised
                    border.color: root.selectedType === "group"
                        && root.selectedId === groupItem.modelData.groupId
                        ? root.theme.textPrimary : groupItem.groupColor
                    border.width: root.selectedType === "group"
                        && root.selectedId === groupItem.modelData.groupId ? 2 : 1

                    Text {
                        anchors.left: parent.left
                        anchors.right: countLabel.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 14
                        anchors.rightMargin: 8
                        text: groupItem.modelData.title
                        color: root.theme.textPrimary
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        id: countLabel
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 13
                        text: groupItem.modelData.noteCount
                        color: groupItem.groupColor
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeAllCursor
                        drag.target: groupItem
                        onPressed: root.objectSelected("group", groupItem.modelData)
                        onReleased: root.mapModel.moveGroup(
                            groupItem.modelData.groupId, groupItem.x, groupItem.y)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    enabled: root.viewKind === "cloud"
                    acceptedButtons: Qt.LeftButton
                    onClicked: root.objectSelected("group", groupItem.modelData)
                    onDoubleClicked: mouse => root.addNoteRequested(
                        groupItem.modelData.groupId,
                        Math.max(16, mouse.x - 72),
                        Math.max(66, mouse.y - 28))
                }

                Repeater {
                    model: root.mapModel.notes

                    delegate: Item {
                        id: noteItem
                        required property var modelData
                        property color noteColor: root.colorFor(modelData.color)
                        visible: modelData.groupId === groupItem.modelData.groupId
                        x: modelData.x
                        y: modelData.y
                        width: 144
                        height: 56

                        Rectangle {
                            anchors.fill: parent
                            radius: noteItem.modelData.center ? 28 : 12
                            color: noteItem.modelData.center
                                ? root.wash(noteItem.noteColor, root.theme.dark ? 0.30 : 0.18)
                                : root.theme.surfaceRaised
                            border.color: root.selectedType === "note"
                                && root.selectedId === noteItem.modelData.noteId
                                ? root.theme.textPrimary : noteItem.noteColor
                            border.width: root.selectedType === "note"
                                && root.selectedId === noteItem.modelData.noteId ? 3 : 1
                        }
                        Rectangle {
                            visible: noteItem.modelData.priority > 0
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.margins: 7
                            width: 7
                            height: 7
                            radius: 4
                            color: noteItem.modelData.priority >= 4
                                ? root.theme.danger : noteItem.noteColor
                        }
                        Text {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 12
                            text: noteItem.modelData.title
                            color: root.theme.textPrimary
                            font.pixelSize: noteItem.modelData.center ? 11 : 10
                            font.weight: noteItem.modelData.center
                                ? Font.DemiBold : Font.Medium
                            horizontalAlignment: Text.AlignHCenter
                            maximumLineCount: 2
                            elide: Text.ElideRight
                            wrapMode: Text.Wrap
                        }
                        Text {
                            visible: noteItem.modelData.linkedTaskId.length > 0
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.margins: 6
                            text: noteItem.modelData.linkedTaskCompleted ? "✓" : "□"
                            color: noteItem.modelData.linkedTaskCompleted
                                ? root.theme.success : noteItem.noteColor
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.SizeAllCursor
                            drag.target: noteItem
                            drag.minimumX: root.viewKind === "cloud" ? 10 : -10000
                            drag.maximumX: root.viewKind === "cloud"
                                ? groupItem.width - noteItem.width - 10 : 10000
                            drag.minimumY: root.viewKind === "cloud" ? 62 : -10000
                            drag.maximumY: root.viewKind === "cloud"
                                ? groupItem.height - noteItem.height - 10 : 10000
                            onPressed: root.objectSelected("note", noteItem.modelData)
                            onReleased: root.mapModel.moveNote(
                                noteItem.modelData.noteId, noteItem.x, noteItem.y)
                        }
                    }
                }
            }
        }
    }

    Repeater {
        model: root.mapModel.connections

        delegate: Button {
            id: edgeButton
            required property var modelData
            property var source: root.noteById(modelData.sourceNoteId)
            property var target: root.noteById(modelData.targetNoteId)
            visible: modelData.viewKind === root.viewKind && source && target
            x: visible ? root.panX
                + (root.noteWorldX(source) + root.noteWorldX(target)) * root.zoom / 2
                - width / 2 : 0
            y: visible ? root.panY
                + (root.noteWorldY(source) + root.noteWorldY(target)) * root.zoom / 2
                - height / 2 : 0
            width: Math.max(28, edgeLabel.implicitWidth + 14)
            height: 24
            onClicked: root.objectSelected("connection", modelData)
            contentItem: Text {
                id: edgeLabel
                text: edgeButton.modelData.label.length > 0
                    ? edgeButton.modelData.label : "→"
                color: root.theme.textPrimary
                font.pixelSize: 9
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                radius: 12
                color: root.theme.surfaceRaised
                border.color: root.selectedType === "connection"
                    && root.selectedId === edgeButton.modelData.connectionId
                    ? root.theme.textPrimary : root.theme.borderStrong
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        spacing: 6
        AppButton {
            theme: root.theme
            quiet: true
            text: "+"
            onClicked: root.zoomBy(1.18, root.width / 2, root.height / 2)
        }
        AppButton {
            theme: root.theme
            quiet: true
            text: "−"
            onClicked: root.zoomBy(0.84, root.width / 2, root.height / 2)
        }
        AppButton {
            theme: root.theme
            quiet: true
            text: qsTr("Center")
            onClicked: root.resetView()
        }
    }

    Text {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 12
        text: qsTr("Drag empty space to pan · wheel to zoom · double-click a group to add an idea")
        color: root.theme.textMuted
        font.pixelSize: 9
    }
}
