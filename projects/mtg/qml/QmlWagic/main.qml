import CustomComponents 1.0
import QtQuick 1.1

Rectangle {
    id: main
    width: 480
    height: 272
    state: "DOWNLOADING"
    color: "black"
    property url resource: "http://wagic.googlecode.com/files/core_017.zip"
    property string hash: "cc6f9415f747acea500cdce190f0df6ee41db7cb"

    states: [
        State {
            name: "DOWNLOADING"
            when: (fileDownloader.hash != hash)
            PropertyChanges {
                target: column1; visible: true
            }
            PropertyChanges {
                target: wagic; visible: false
            }
            PropertyChanges {
                target:fileDownloader; url: resource
            }
        },
        State {
            name: "NORMAL"
            when: (fileDownloader.hash == hash)
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
        active: Qt.application.active
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
        property int lastTick: 0
        property int lastX: 0
        property int lastY: 0

        onPositionChanged: {
            wagic.pixelInput(
                        (mouse.x*wagic.nominalWidth)/width,
                        (mouse.y*wagic.nominalHeight)/height)
        }

        onPressed: {
            wagic.pixelInput(
                        (mouse.x*wagic.nominalWidth)/width,
                        (mouse.y*wagic.nominalHeight)/height)

            if(mouse.button == Qt.LeftButton) {
                lastTick = wagic.getTick()
                lastX = mouse.x
                lastY = mouse.y
            } else if(mouse.button == Qt.MiddleButton)
                wagic.doCancel()
            else if(mouse.button == Qt.RightButton)
                wagic.doNext()
        }

        onReleased: {
            if(mouse.button == Qt.LeftButton) {
                var currentTick = wagic.getTick()
                if(currentTick - lastTick <= 250 )
                {
                    if(Math.abs(lastX-mouse.x) < 50 && Math.abs(lastY-mouse.y) < 50 )
                    {
                        wagic.doOK()
//                        wagic.done()
                    }
                }
                else if(currentTick - lastTick >= 250)
                {
                    wagic.doScroll(mouse.x - lastX, mouse.y - lastY)
                }
            } else if(mouse.button == Qt.MiddleButton)
                wagic.done()
            else if(mouse.button == Qt.RightButton)
                wagic.done()
        }

        onPressAndHold: {
            if(Math.abs(lastX-mouse.x)<50 && Math.abs(lastY-mouse.y)<50)
            {
                wagic.doMenu()
            }
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



