import QtQuick 1.1

Item {
	id: item1
//	property string text: ""
//	property alias pointsize: text1.font.pointSize
	property alias pressed: mousearea1.pressed
	width: oneChar.height * 1.8
	height: width
	signal clicked

	Rectangle {
		anchors.fill: parent
        color: fgcolor //mousearea1.pressed ? Qt.darker(fgcolor) : fgcolor
        opacity: mousearea1.pressed ? 0.7 : 0.4
		radius: oneChar.height * 0.3
	}

	MouseArea {
		id: mousearea1
		anchors.fill: parent
		onClicked: parent.clicked()
	}
	Text {
		id: oneChar
		text: " "
		visible: false
	}
}
