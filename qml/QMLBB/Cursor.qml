import QtQuick 1.1


Rectangle {
	Text {
		id: invertedCharacter
        color: bgcolor
		font.pointSize: fontsize
		text: cursor_rect.parent.cursorPosition >= cursor_rect.parent.text.length ?
			" ": cursor_rect.parent.text[cursor_rect.parent.cursorPosition]
		anchors.centerIn: parent
		font.bold: cursor_rect.parent.font.bold
	}
//	Component.onCompleted: {
//		/* set width to single character width */
//		width = parent.width / parent.displayText.length
//	}
	id: cursor_rect
	width: invertedCharacter.width
	height: parent.height
    color: fgcolor

	SequentialAnimation on opacity  {
		id: cursorAnimation
		loops: Animation.Infinite
		PropertyAnimation {
			from: 1.0
			duration: 400
		}
		PropertyAnimation {
			easing.type: Easing.OutSine
			to: 0.0
			duration: 200
		}
		PropertyAnimation {duration: 400}
		PropertyAnimation {
			easing.type: Easing.InSine
			from: 0
			to: 1.0
			duration: 200
		}
	}
	onXChanged: {
		opacity = 1
		cursorAnimation.restart()
	}
}
