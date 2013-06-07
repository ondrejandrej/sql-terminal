
#include <QtCore>
#include "utils.h"

void TextBuffer::operator<<(const QString &str)
{
	buffer.append(str.split("\n"));
	while(buffer.length() >= bufferSize)
		buffer.removeFirst();
	context.setContextProperty("_consoleText", buffer.join("\n"));
}
