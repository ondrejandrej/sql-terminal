#ifndef UTILS_H
#define UTILS_H

#include <QDeclarativeContext>
#include <QStringList>

class TextBuffer
{
public:
	TextBuffer(QDeclarativeContext &_context) : context(_context), bufferSize(0) {}
	void setSize(int size) { bufferSize = size; }
	void operator<<(const QString &str);

private:
	QDeclarativeContext &context;
	QStringList buffer;
	int bufferSize;
};

class CommandHistory: public QObject
{
	Q_OBJECT
public:
	CommandHistory(): currentIndex(0) {}
	void append(const QString &text) {
//		tmp.clear();
		if(text.trimmed().length() == 0 ||
				(data.length() > 0 &&
				data.at(data.length() - 1) == text))
		{
			currentIndex = data.length();
			return;
		}
		data.append(text);
		if(data.length() > maxLength)
			data.removeFirst();
		currentIndex = data.length();
	}
	Q_INVOKABLE QString up() {
		if(data.length() == 0) return tmp;
		if(currentIndex > 0) currentIndex--;
		QString retVal = data.at(currentIndex);
		return retVal;
	}
	Q_INVOKABLE QString down() {
		if(currentIndex < data.length())
			currentIndex++;
		if(currentIndex < data.length()) {
			return data.at(currentIndex);
		}
		else
			return tmp;
	}
	Q_INVOKABLE void setTMP(QString value) {
		if(/*tmp.length() == 0 &&*/ currentIndex == data.length())
			tmp = value;
	}
	QStringList getData() const {
		return data;
	}
	void setData(const QStringList &_data) {
		data = _data;
		while(data.length() > maxLength)
			data.removeFirst();
		currentIndex = data.length();
	}

private:
	static const int maxLength = 20;
	int currentIndex;
	QStringList data;
	QString tmp;
};

#endif // UTILS_H
