import QtQuick 1.0

 Rectangle {
     id: progressbar

     property int minimum: 0
     property int maximum: 100
     property int value: 0

     width: 150
     height: 80
     radius: 10
     gradient: Gradient {
         GradientStop { id: gradient1; position: 0.0; color: "red" }
         GradientStop { id: gradient2; position: 0.7; color: "blue" }
     }
     border.width: 2
     border.color: "blue"
     opacity: 0.7
     smooth: true
     clip: true


     Rectangle {
         id: highlight

         property int widthDest: ((progressbar.width * (value - minimum)) / (maximum - minimum))

         anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
         width: highlight.widthDest
         radius: 10
         gradient: Gradient {
             GradientStop { id: gradient3; position: 0.0; color: "blue" }
             GradientStop { id: gradient4; position: 0.7; color:  "red" }
         }
         smooth: true

         Behavior on width { SmoothedAnimation { velocity: 1200 } }
     }

     Text {
         anchors { centerIn: progressbar }
         color: "black"
         font.pixelSize: 12
         font.bold: true
         text: Math.floor((value - minimum) / (maximum - minimum) * 100) + '%'
     }
 }
