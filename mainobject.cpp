#include <QtCore>
#include <QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QtSql>
//#include "qmlapplicationviewer.h"
#include "mainobject.h"

MainObject::MainObject(QObject *parent) :
	QObject(parent), context(*viewer.engine()->rootContext()), console(*viewer.engine()->rootContext())
{
	baseFont.setFamily("Courier New");
	baseFont.setPointSize(10);
	QApplication::setFont(baseFont);

	loadSettings();
	context.setContextProperty("mainObject", this);
	context.setContextProperty("cmdHistory", &cmdHistory);
	context.setContextProperty("_consoleText", QString(""));
	context.setContextProperty("_promptText", QString("SQLTerm> "));
	viewer.addImportPath(QLatin1String("modules"));
	viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
	viewer.setMainQmlFile(QLatin1String("qml/QMLBB/main.qml"));
	viewer.showExpanded();

	console.setSize(80);
	console << "--== SQL Terminal 0.1.0 ==--\nEnter .help for more information.";
	createDBConnection();
}

MainObject::~MainObject() {
	QSettings settings("Cute Projects", "SQL Terminal");
	settings.setValue("Command history", cmdHistory.getData());
}

void MainObject::processCommand(QString prompt, QString command)
{
	console << prompt + command;
	cmdHistory.append(command);
	command = command.trimmed();
	if(command.length() == 0) return;
	if(incompleteCmd.length() == 0 && command.at(0) == '.') {
		command = command.right(command.length() - 1);	// omit the leading period
		QStringList args = command.split(QRegExp("\\s+"));
		if(command == "help") {
			printHelp();
			return;
		}
		if(args.length() < 2) {
			console << "Error: missing parameter value";
			return;
		}
		if(!setParameter(args[0], args[1]))
			console << "Error: unknown parameter";
	}
	else {
		incompleteCmd.append(command + "\n");
		if(incompleteCmd.at(incompleteCmd.length() - 2) == ';') {
			runQuery(incompleteCmd);
			incompleteCmd.clear();
			context.setContextProperty("_promptText", "SQLTerm> ");
		} else {
			context.setContextProperty("_promptText", "...> ");
		}
	}
}

void MainObject::runQuery(QString query) {
	QSqlQuery _query;
	if(_query.exec(query))
		while(_query.next()) {
			QVariant value;
			QString output_row;
			int i = 0;
			while(true) {
				value = _query.value(i++);
				if(!value.isValid())
					break;
				output_row += value.toString() + "\t";
			}
			console << output_row;
		}
	else {
		QSqlError err = _query.lastError();
		if(err.isValid()) {
			console << err.databaseText();
			console << err.driverText();
		}
	}
}

bool MainObject::setParameter(const QString param, const QString val)
{
	if(!parameters.contains(param))
		return false;
	QSettings settings("Cute Projects", "SQL Terminal");
	settings.setValue(param, val);
	context.setContextProperty(param, val);
	return true;
}

void MainObject::createDBConnection()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//	console << QDir::homePath();
//	qDebug() << db.isValid();
#ifdef Q_OS_SYMBIAN
    db.setDatabaseName("E:\\Others\\sql_terminal.db");
#else
	db.setDatabaseName("./shared/documents/sql_terminal.db");
#endif

	if(!db.open())
		console << db.lastError().text();
}

void MainObject::printHelp() {
	QString helptext;
	helptext = "Usage: enter your SQL query or a special terminal command "
            "(beginning with a dot). Resulting database is stored in "
            "\"/documents/sql_terminal.db\"\n\n"
			"Available \"dot commands\":\nname:  \t\tdefault value:";
	QMapIterator<QString, QString> i(parameters);
	while(i.hasNext()) {
		i.next();
		helptext += "\n." + i.key() + "\t\t" + i.value();
	}
	console << helptext;
}


