// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var mapModel
    required property var theme
    property bool showSuggestionAction: false
    property int mapMode: 2
    property var selectedNode: null
    signal addRequested()
    signal addInCategoryRequested(string categoryId, string subcategoryId)
    signal manageCategoriesRequested()
    signal manageGoalsRequested()
    signal suggestionRequested()

    function selectNode(node) {
        selectedNode = node
    }

    function kindLabel(kind) {
        return kind === "category" ? "WORK AREA"
            : kind === "subcategory" ? "SUBCATEGORY"
            : kind === "goal" ? "GOAL"
            : kind === "milestone" ? "MILESTONE"
            : "TASK"
    }

    Component {
        id: cloudMode
        MentalMapClouds {
            hierarchy: root.mapModel.hierarchy
            theme: root.theme
            onNodeSelected: node => root.selectNode(node)
        }
    }

    Component {
        id: flowMode
        MentalMapFlow {
            hierarchy: root.mapModel.hierarchy
            theme: root.theme
            onNodeSelected: node => root.selectNode(node)
        }
    }

    Component {
        id: constellationMode
        MentalMapConstellation {
            flatNodes: root.mapModel.flatNodes
            mapRevision: root.mapModel.revision
            theme: root.theme
            onNodeSelected: node => root.selectNode(node)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.width < 720 ? 12 : 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Mental map"
                    color: root.theme.textPrimary
                    font.pixelSize: 25
                    font.weight: Font.DemiBold
                }
                Text {
                    visible: root.width >= 720
                    text: root.mapModel.itemCount + " active nodes · work areas, tasks, goals, and milestones"
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }
            }
            AppButton {
                visible: root.showSuggestionAction && root.width >= 800
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
            spacing: 7

            Text {
                visible: root.width >= 700
                text: "VIEW"
                color: root.theme.textMuted
                font.pixelSize: 9
                font.weight: Font.Bold
                font.letterSpacing: 1.2
            }
            AppButton {
                theme: root.theme
                primary: root.mapMode === 0
                quiet: root.mapMode !== 0
                text: root.width < 720 ? "☁  Clouds" : "☁  Recursive clouds"
                onClicked: root.mapMode = 0
            }
            AppButton {
                theme: root.theme
                primary: root.mapMode === 1
                quiet: root.mapMode !== 1
                text: root.width < 720 ? "➜  Arrows" : "➜  Arrow flow"
                onClicked: root.mapMode = 1
            }
            AppButton {
                theme: root.theme
                primary: root.mapMode === 2
                quiet: root.mapMode !== 2
                text: root.width < 720 ? "✦  Stars" : "✦  Constellation"
                onClicked: root.mapMode = 2
            }
            Item { Layout.fillWidth: true }
            AppButton {
                visible: root.width >= 860
                theme: root.theme
                quiet: true
                text: "Work areas"
                onClicked: root.manageCategoriesRequested()
            }
            AppButton {
                visible: root.width >= 860
                theme: root.theme
                quiet: true
                text: "Goals"
                onClicked: root.manageGoalsRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius + 6
            color: root.theme.background
            border.color: root.theme.borderStrong
            clip: true

            Loader {
                anchors.fill: parent
                anchors.margins: 2
                sourceComponent: root.mapMode === 0 ? cloudMode
                    : root.mapMode === 1 ? flowMode : constellationMode
            }

            EmptyState {
                anchors.centerIn: parent
                visible: root.mapModel.itemCount === 0
                theme: root.theme
            }

            Rectangle {
                visible: root.selectedNode !== null
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 14
                width: Math.min(350, parent.width - 28)
                implicitHeight: inspectorColumn.implicitHeight + 28
                radius: root.theme.radius + 6
                color: root.theme.surfaceRaised
                border.color: root.theme.accent
                border.width: 1

                ColumnLayout {
                    id: inspectorColumn

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 14
                    spacing: 7

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: root.selectedNode
                                ? root.kindLabel(root.selectedNode.kind) : ""
                            color: root.theme.accent
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            font.letterSpacing: 1.2
                        }
                        AppButton {
                            theme: root.theme
                            quiet: true
                            text: "×"
                            onClicked: root.selectedNode = null
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.selectedNode ? root.selectedNode.title : ""
                        color: root.theme.textPrimary
                        font.pixelSize: 18
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.selectedNode ? root.selectedNode.detail : ""
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.selectedNode !== null
                            && (root.selectedNode.categoryId.length > 0
                                || root.selectedNode.kind === "goal"
                                || root.selectedNode.kind === "milestone")
                        AppButton {
                            visible: root.selectedNode !== null
                                && root.selectedNode.categoryId.length > 0
                            theme: root.theme
                            primary: true
                            text: "+ Add in this area"
                            onClicked: root.addInCategoryRequested(
                                root.selectedNode.categoryId,
                                root.selectedNode.subcategoryId)
                        }
                        AppButton {
                            visible: root.selectedNode !== null
                                && (root.selectedNode.kind === "goal"
                                    || root.selectedNode.kind === "milestone")
                            theme: root.theme
                            text: "Open goals"
                            onClicked: root.manageGoalsRequested()
                        }
                    }
                }
            }
        }
    }
}
