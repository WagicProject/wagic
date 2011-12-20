import QtQuick 1.0

Rectangle {
    id: main
    width: 360
    height: 360
    scale: 1

    Column{
        id: column1
        x: -48
        y: 0
        width: 457
        height: 374
        anchors.horizontalCenterOffset: 1
        scale: 1
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 35
        Image {
            id: logo

            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
            source: "http://wololo.net/forum/styles/prosilver/imageset/site_logo.gif"
        }

        Text {
            text: qsTr("Downloading resources")
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
        }

        ProgressBar {
            id: progressbar
            value: fileDownloader.received
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

}



