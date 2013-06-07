#ifndef MAINOBJECT_H
#define MAINOBJECT_H

#include <QObject>
#include <QFont>
#include <QSettings>
#include <QMap>
#include <QDeclarativeContext>
#include "qmlapplicationviewer.h"
#include "utils.h"

class MainObject : public QObject
{
    Q_OBJECT
public:
    explicit MainObject(QObject *parent = 0);
    ~MainObject();
    
signals:
    
public slots:
	void processCommand(QString prompt, QString command);
    
private:
    QFont baseFont;
    QmlApplicationViewer viewer;
	QDeclarativeContext &context;
	TextBuffer console;
	QString incompleteCmd;
	CommandHistory cmdHistory;
	QMap<QString, QString> parameters;
	bool setParameter(const QString param, const QString val);
	void createDBConnection();
	void printHelp();
	void runQuery(QString query);
	void loadSettings()
	{
		parameters["fontsize"] = "9";
		parameters["bgcolor"] = "lightblue";
		parameters["fgcolor"] = "black";
		QSettings settings("Cute Projects", "SQL Terminal");
		QMapIterator<QString, QString> i(parameters);
		while(i.hasNext()) {
			i.next();
			context.setContextProperty(	i.key(),
										settings.value(i.key(), i.value()));
		}
		cmdHistory.setData(settings.value("Command history").toStringList());
	}
};

#endif // MAINOBJECT_H
