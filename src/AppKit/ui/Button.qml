import QtQuick 1.1

Rectangle {
    id: button
    width: 100
    height: 32
    radius: 10
    state: "Disabled"
    border.width: 1
    border.color: "#c9c9c9"
    gradient: Gradient {
        GradientStop { id: gradientTop; position: 0.0; color: "#fdfdfd"; }
        GradientStop { id: gradientBottom; position: 1.0; color: "#eaeaea"; }
    }

    property alias text: textElem.text

    Text {
        id: text1
        x: 0
        y: 0
        text: qsTr("Text")
        font.pixelSize: 12
        opacity: 0
    }

    Text {
        id: textElem
        x: 0
        y: 0
        width: 100
        height: 32
        text: "Text"
        smooth: true
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 12
        anchors.fill: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges { target: gradientTop; color: "#fdfdfd"; }
            PropertyChanges { target: gradientBottom; color: "#eaeaea"; }
        },
        State {
            name: "Pressed"; when: mouseArea.pressed
            PropertyChanges { target: gradientTop; color: "#757575"; }
            PropertyChanges { target: gradientBottom; color: "#a8a8a8"; }
        },
        State {
            name: "Disabled"
            PropertyChanges { target: textElem; color: "#888888" }
            PropertyChanges { target: mouseArea; enabled: false }
        }

    ]

    transitions: [
        Transition {
            ColorAnimation { duration: 75 }
        }

    ]
}
