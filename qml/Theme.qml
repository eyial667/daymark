// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

QtObject {
    id: theme

    required property int style
    required property int colorMode
    required property int accentPreset

    readonly property bool dark: colorMode === 0

    readonly property color background: dark
        ? (style === 0 ? "#0B0D10"
            : style === 1 ? "#090C0F"
            : style === 2 ? "#0C0C0E"
            : "#0A0C0F")
        : (style === 0 ? "#F4F5F8"
            : style === 1 ? "#F1F6F6"
            : style === 2 ? "#F7F4EE"
            : "#F3F4F7")
    readonly property color sidebar: dark
        ? (style === 0 ? "#090B0E"
            : style === 1 ? "#0A0E11"
            : style === 2 ? "#0B0B0D"
            : "#090B0E")
        : (style === 0 ? "#EAECF2"
            : style === 1 ? "#E7F0F0"
            : style === 2 ? "#ECE8DF"
            : "#E9EBF1")
    readonly property color surface: dark
        ? (style === 0 ? "#11151A"
            : style === 1 ? "#10161A"
            : style === 2 ? "#141418"
            : "#111419")
        : (style === 0 ? "#FFFFFF"
            : style === 1 ? "#FCFFFF"
            : style === 2 ? "#FFFCF7"
            : "#FFFFFF")
    readonly property color surfaceRaised: dark
        ? (style === 0 ? "#161A20"
            : style === 1 ? "#151D21"
            : style === 2 ? "#19191E"
            : "#171A20")
        : (style === 0 ? "#F8F9FB"
            : style === 1 ? "#F5FAFA"
            : style === 2 ? "#FAF6EF"
            : "#F7F8FB")
    readonly property color surfaceHover: dark
        ? (style === 0 ? "#1B2027"
            : style === 1 ? "#182328"
            : style === 2 ? "#202027"
            : "#1C2128")
        : (style === 0 ? "#EEF0F5"
            : style === 1 ? "#EAF4F3"
            : style === 2 ? "#F1ECE3"
            : "#EDEFF5")
    readonly property color border: dark
        ? (style === 0 ? "#272C35"
            : style === 1 ? "#263137"
            : style === 2 ? "#2D2D34"
            : "#293039")
        : (style === 0 ? "#D8DBE3"
            : style === 1 ? "#D1DFDF"
            : style === 2 ? "#DED8CE"
            : "#D7DAE2")
    readonly property color borderStrong: dark
        ? (style === 0 ? "#373E49"
            : style === 1 ? "#33464B"
            : style === 2 ? "#3A383F"
            : "#38414C")
        : (style === 0 ? "#BCC1CD"
            : style === 1 ? "#B3C8C7"
            : style === 2 ? "#C5BCB0"
            : "#BAC0CC")

    readonly property color interfaceAccent: dark
        ? (style === 0 ? "#7C6CFF"
            : style === 1 ? "#55C9C3"
            : style === 2 ? "#D6A85F"
            : "#8A78F5")
        : (style === 0 ? "#6654D9"
            : style === 1 ? "#247F7B"
            : style === 2 ? "#956418"
            : "#6956D8")
    readonly property color interfaceAccentSoft: dark
        ? (style === 0 ? "#282343"
            : style === 1 ? "#173336"
            : style === 2 ? "#392F21"
            : "#292444")
        : (style === 0 ? "#E8E4FF"
            : style === 1 ? "#DCEFED"
            : style === 2 ? "#F2E5D0"
            : "#E9E5FF")
    readonly property color accent: accentPreset === 0 ? interfaceAccent
        : accentPreset === 1 ? (dark ? "#8A78F5" : "#6956D8")
        : accentPreset === 2 ? (dark ? "#55C9C3" : "#247F7B")
        : accentPreset === 3 ? (dark ? "#D6A85F" : "#956418")
        : (dark ? "#67C587" : "#347A4E")
    readonly property color accentSoft: accentPreset === 0 ? interfaceAccentSoft
        : accentPreset === 1 ? (dark ? "#292444" : "#E9E5FF")
        : accentPreset === 2 ? (dark ? "#173336" : "#DCEFED")
        : accentPreset === 3 ? (dark ? "#392F21" : "#F2E5D0")
        : (dark ? "#183326" : "#DDEEE3")
    readonly property color secondaryAccent: dark
        ? (style === 0 ? "#59A8FF"
            : style === 1 ? "#78B982"
            : style === 2 ? "#9B8CD9"
            : "#55C9C3")
        : (style === 0 ? "#3373B5"
            : style === 1 ? "#4B8254"
            : style === 2 ? "#6854A6"
            : "#247F7B")

    readonly property color danger: dark ? "#EF6A5B" : "#C84437"
    readonly property color dangerSoft: dark ? "#3A1E1D" : "#F9E2DF"
    readonly property color success: dark ? "#67C587" : "#347A4E"
    readonly property color successSoft: dark ? "#183326" : "#DDEEE3"
    readonly property color textPrimary: dark
        ? (style === 2 ? "#F3F0EA" : "#F2F4F7")
        : (style === 2 ? "#29251F" : "#20242A")
    readonly property color textSecondary: dark
        ? (style === 2 ? "#B6B1AB" : "#A8AFB9")
        : (style === 2 ? "#625C54" : "#555D68")
    readonly property color textMuted: dark
        ? (style === 2 ? "#7E7A77" : "#737B86")
        : (style === 2 ? "#827A70" : "#7A838F")
    readonly property color shadow: dark ? "#000000" : "#6F7785"

    readonly property int radius: style === 0 ? 5
        : style === 1 ? 10
        : style === 2 ? 3
        : 8
    readonly property string modeName: style === 0 ? qsTr("Midnight Command")
        : style === 1 ? qsTr("Spatial Map")
        : style === 2 ? qsTr("Quiet Focus")
        : qsTr("Daymark Hybrid")
    readonly property string modeCode: style === 0 ? "A"
        : style === 1 ? "B"
        : style === 2 ? "C"
        : "D"
}
