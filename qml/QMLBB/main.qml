import QtQuick 1.1

Rectangle {
	id: rootElement
    color: bgcolor
	width: 360
	height: 360
	z: 0
	onHeightChanged: {
		consoleFlickable.height = height - inputLine.height
		consoleFlickable.scrollToBottom()
        console.log("height changed: " + height)
	}
	Component.onCompleted: {
	//	inputLine.focus = true
	}

	Flickable {
		id: consoleFlickable
		anchors.top:  parent.top
		anchors.left:  parent.left
		anchors.right:  parent.right
		//height: parent.height - inputLine.height - dummyRect.height
		anchors.bottom: inputLine.top
		contentHeight: consoleText.height
		clip: true
		onHeightChanged: {
				scrollToBottom()
				console.log("console height changed: " + height)
		}
		function scrollToBottom() {
//			console.log("contentHeight: " + contentHeight + " height: " + height)
			if(contentHeight > height)
				contentY = contentHeight - height
		}
		Text {
			width: rootElement.width
			id: consoleText
			text: _consoleText
            font.pointSize: fontsize
            color: fgcolor
		//	height: parent.height
			anchors.bottom: parent.bottom
			wrapMode: Text.WrapAnywhere
			onTextChanged: {
				//if(height < parent.height)
				//	height = parent.height
				consoleFlickable.scrollToBottom()
			}
		}
		MouseArea {
			anchors.fill: parent
			onPressed:
				focus = true
		}
	}
	Grid {
		id: buttonsGrid
		visible: inputLine.focus
		columns: 2
		spacing: inputLine.height * 0.1
		anchors.bottom: inputLine.bottom
		anchors.right: parent.right
		ButtonEx {
			id: semicolonButton
			Text {
				text: ";"
				color: bgcolor
				scale: parent.pressed ? 0.95 : 1
				anchors.centerIn: parent
				font.weight: Font.Bold
			}
			onClicked: {
				var pos = inputLine.cursorPosition
				var first = inputLine.text.substring(0, pos)
				var second = inputLine.text.substring(pos)
				inputLine.text = first + ";" + second
				inputLine.cursorPosition = pos + 1
			}
		}
		ButtonEx {
			id: upButton
			Text {
				text: "<"
				rotation: 90
				color: bgcolor
				scale: parent.pressed ? 0.95 : 1
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
				font.weight: Font.Bold
			}
			onClicked: {
				cmdHistory.setTMP(inputLine.commandText)
				inputLine.commandText = cmdHistory.up()
			}
		}
		ButtonEx {
			id: homeEndButton
			Text {
				text: "Home\nEnd"
				color: bgcolor
				scale: parent.pressed ? 0.95 : 1
				anchors.centerIn: parent
				font.weight: Font.Bold
				font.pointSize: 6
			}
			onClicked: {
				if(inputLine.cursorPosition === inputLine.text.length)
					inputLine.cursorPosition = inputLine.promptText.length
				else
					inputLine.cursorPosition = inputLine.text.length
			}
		}
		ButtonEx {
			id: downButton
			Text {
				text: ">"
				rotation: 90
				color: bgcolor
				scale: parent.pressed ? 0.95 : 1
				anchors.centerIn: parent
				font.weight: Font.Bold
			}
			onClicked: {
				cmdHistory.setTMP(inputLine.commandText)
				inputLine.commandText = cmdHistory.down()
			}
		}
	}
	TextInput {
		id: inputLine
		property string promptText: _promptText
		property string commandText
        //anchors.top: consoleFlickable.bottom
        anchors.bottom: dummyRect.top
		width: rootElement.width - buttonsGrid.width - oneCharacter.width
		font.bold: true
        font.pointSize: fontsize
        color: fgcolor
		focus: false
		cursorDelegate: Cursor {}
		Keys.onPressed: {
			if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
				mainObject.processCommand(promptText, commandText)
				commandText = ""
				if(event.key === Qt.Key_Enter &&
					promptText != "...> ")	// applies to Symbian
					focus = false
			}
			else if(event.key === Qt.Key_Left || event.key === Qt.Key_Backspace) {
				if(cursorPosition <= promptText.length)
					event.accepted = true
			}
			else if(event.key === Qt.Key_Up)
				upButton.clicked()
			else if(event.key === Qt.Key_Down)
				downButton.clicked()
		}
		onCommandTextChanged: {
			text = promptText + commandText
		}
		onPromptTextChanged: {
			text = promptText + commandText
		}
		onTextChanged: {
			commandText = text.substring(promptText.length)
		}
		onFocusChanged: {
			if(focus === true) {
				openSoftwareInputPanel()
				consoleFlickable.scrollToBottom()
			}
		}
		MouseArea {		// do not allow the user to place cursor on promptText
			anchors.left: parent.left
			anchors.top: parent.top
			height: parent.height
			width: oneCharacter.width * parent.promptText.length
			onClicked:
				parent.focus = true
		}
		Text {
			id: oneCharacter
			text: " "
			font.pointSize: fontsize
			visible: false
		}
	}
    Rectangle {		// Quick and dirty fix to lift inputLine above virtual keyboard on BlackBerry 10.1 onwards
        id: dummyRect
        anchors.bottom: parent.bottom
//		height: inputLine.focus ? (parent.height === 1280 ? 525 : 385) : 0
		height: inputLine.focus ? transformHeight(parent.height) : 0
        width: parent.width
		function transformHeight(_height) {
			if(_height === 1280)
				return 525	// height of virtual keyboard on Z10 in portrait
			if(_height === 768)
				return 385	// height of virtual keyboard on Z10 in landscape
			return 0	// for phones other than Z10, don't lift inputLine
		}
    }
}


