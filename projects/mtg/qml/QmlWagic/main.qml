import CustomComponents 1.0
import QtQuick 1.1

Rectangle {
    id: main
    width: 480
    height: 272
    state: "DOWNLOADING"
    color: "black"

    states: [
        State {
            name: "DOWNLOADING"
            PropertyChanges {
                target: column1; visible: true
            }
            PropertyChanges {
                target: wagic; visible: false
            }
            PropertyChanges {
                target:fileDownloader; url: "http://wagic.googlecode.com/files/core_017.zip"
            }
        },
        State {
            name: "NORMAL"
            when: (fileDownloader.done == true)
            PropertyChanges {
                target: column1; visible: false
            }
            PropertyChanges {
                target: wagic; visible: true; focus: true
            }
        }
    ]

    Column{
        id: column1
        x: -48
        y: 0
        width: 480
        height: 272
        anchors.horizontalCenterOffset: 1
        scale: 1
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 15
        Image {
            id: logo

            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
            source: "http://wagic.googlecode.com/svn/trunk/projects/mtg/bin/Res/graphics/menutitle.png"
        }

        Text {
            text: qsTr("Downloading resources")
            font.bold: true
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
        }

        ProgressBar {
            id: progressbar
            height: 40
            value: fileDownloader.received
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }


    WagicCore {
        id: wagic
        anchors.fill: parent
        visible: false
        active: Qt.WindowActive
    }
/*
    Rectangle {
        id: wagic
        anchors.fill: parent
        color: "red"
        visible: false
    }
*/
    MouseArea {
        id: mousearea
        hoverEnabled: true
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        onPositionChanged: {
            wagic.pixelInput(
                        (mouse.x*wagic.nominalWidth)/width,
                        (mouse.y*wagic.nominalHeight)/height)
        }

        onPressed: {
            if(mouse.button == Qt.LeftButton)
                wagic.doOK()
            else if(mouse.button == Qt.MiddleButton)
                wagic.doCancel()
            else if(mouse.button == Qt.RightButton)
                wagic.doNext()
        }

        onReleased: {
            if(mouse.button == Qt.LeftButton)
                wagic.done()
            else if(mouse.button == Qt.MiddleButton)
                wagic.done()
            else if(mouse.button == Qt.RightButton)
                wagic.done()
        }

        onPressAndHold: {
            wagic.doMenu()
        }
    }
    function resize(){
        if(width/height <= wagic.nominalRatio)
        {
            mousearea.x = 0
            mousearea.y = -((width/wagic.nominalRatio)-height)/2
            mousearea.width = width
            mousearea.height = width / wagic.nominalRatio
        }
        else
        {
            mousearea.x = -(height*wagic.nominalRatio-width)/2
            mousearea.y = 0
            mousearea.width = height * wagic.nominalRatio
            mousearea.height = height
        }
    }

    onWidthChanged: {
        mousearea.anchors.fill = undefined
        resize()
    }
    onHeightChanged: {
        mousearea.anchors.fill = undefined
        resize()
    }
}



