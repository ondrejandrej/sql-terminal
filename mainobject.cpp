#include <QtCore>
#include <QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QtSql>
//#include "qmlapplicationviewer.h"
#include "mainobject.h"
#include <QtDebug>
#include "base64.h"
#include "deaccent.cpp"

qint64 base64toInt(const QString &str);
qint64 last_insert_rowid(QSqlQuery &q);

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
	createDatabase();
	timer = new QTimer(this);
	//connect(timer, SIGNAL(timeout()), this, SLOT(loadData()));
//	timer->singleShot(5000, this, SLOT(loadData()));
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
	if(incompleteCmd.length() == 0 && command.at(0) == '.') {	// if there was no unfinished SQL before
		command = command.right(command.length() - 1);	// omit the leading period
		QStringList args = command.split(QRegExp("\\s+"));
		if(command == "help") {
			printHelp();
			return;
		}
		if(args[0] == "load") {		// REMOVE THIS
			//timer->singleShot(1000, this, SLOT(loadData()));
			loadData(args[1]);
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

	if(!db.open()) {
		console << db.lastError().text();
		return;
	}
	QSqlQuery _query;
	_query.exec("PRAGMA foreign_keys = ON;");
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

void MainObject::loadData(QString fileName)
{
#ifdef Q_OS_SYMBIAN
	QFile idxFile(QString("E:\\Others\\Slovniky\\%1.index").arg(fileName));
#else
	QFile idxFile(QString(".\\%1.index").arg(fileName));
#endif
	idxFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream idxStream(&idxFile);
	idxStream.setCodec("UTF-8");
	QSqlQuery _query;
	QString SQLString, old_keyword;
	QStringList entry;
	QEventLoop eLoop;
	QElapsedTimer timer;
	timer.start();
	qint64 old_time = timer.elapsed();
	qint64 new_time = 0;

	qDebug() << _query.exec("INSERT INTO databases(name, long_name, datafile_path) VALUES('" + fileName +
							"', 'Dlhý text " + fileName + "', 'C:\\Documents\\" + fileName + ".dict.dz');");

	qint64 current_database_id = last_insert_rowid(_query);
	_query.exec("BEGIN TRANSACTION");

	unsigned int i = 0;
	qint64 current_keyword_id;
	while(!idxStream.atEnd()) {
		entry = idxStream.readLine().split("\t");
		if(entry.length() != 3)
			break;
		if(old_keyword != entry[0]) {
			SQLString = QString("INSERT INTO keywords(title, database_id, title_deaccent) VALUES('%1', %2, '%3');")
							.arg(entry[0]).arg(current_database_id).arg(deaccent(entry[0]));
//			qDebug() << SQLString;
			if(!_query.exec(SQLString))
				break;
			current_keyword_id = last_insert_rowid(_query);
//			_query.exec(QString("SELECT keyword_id FROM keywords WHERE title = '%1' AND database_id = %2;")
//							.arg(entry[0]).arg(current_database_id));
//			_query.next();
//			current_keyword_id = _query.value(0).toLongLong();
		}
		SQLString = QString("INSERT INTO entries(position, data_length, keyword_id) VALUES(%1, %2, %3);")
						.arg(base64toInt(entry[1])).arg(base64toInt(entry[2])).arg(current_keyword_id);
//		qDebug() << SQLString;
		if(!_query.exec(SQLString))
			break;


		if(i % 1000 == 0) {
			//console << QString("%1").arg(i);
			//eLoop.processEvents();
			if(i % 3000 == 0) {
				eLoop.processEvents();
				_query.exec("END TRANSACTION");
				//qDebug() << SQLString;
				new_time = timer.elapsed();
				console << QString("The transaction took %1 ms.").arg(new_time - old_time);
				old_time = new_time;
				_query.exec("BEGIN TRANSACTION");
			}
		}

		old_keyword = entry[0];
		i ++;
	}
	QSqlError err = _query.lastError();
	if(err.isValid()) {
		console << err.databaseText();
		console << err.driverText();
	}
	_query.exec("END TRANSACTION");
	console << QString("Operation took %1 ms.").arg(new_time);
}

void MainObject::createDatabase()
{
	QSqlQuery q;
	if(q.exec("SELECT * FROM databases WHERE database_id = 1"))
		return;

	q.exec("CREATE TABLE databases ( "
			   "database_id	INTEGER	NOT NULL	PRIMARY KEY, "
				"name	TEXT	NOT NULL	UNIQUE, "
				"long_name	TEXT, "
				"datafile_path	TEXT	NOT NULL, "
				"sametypesequence	TEXT)");

	q.exec("CREATE TABLE keywords ( "
			"	title	TEXT	NOT NULL, "
			"	database_id	INTEGER	NOT NULL	REFERENCES databases(database_id)	ON DELETE CASCADE, "
			"	title_deaccent	TEXT	NOT NULL, "
			"	keyword_id	INTEGER	NOT NULL	PRIMARY KEY, "
			"	UNIQUE(title, database_id)) ");

	//-- CREATE UNIQUE INDEX idx1_keywords ON keywords(title, database_id);
	//CREATE INDEX idx2_keywords ON keywords(title_deaccent, database_id, title);

	q.exec("CREATE TABLE entries ( "
			"	position	INTEGER	NOT NULL, "
			"	data_length	INTEGER	NOT NULL, "
			"	keyword_id	INTEGER	NOT NULL	REFERENCES keywords(keyword_id)	ON DELETE CASCADE); ");
}

qint64 base64toInt(const QString &str)
{
	int c;
	qint64 val = 0;
	for(unsigned int i = 0; (i < str.length()) && (i < sizeof(qint64)); i++) {
		c = static_cast<int>(str.at(str.length() - i - 1).toAscii());
		if((c & 0x80) || (base64_val[c] == -1))
			throw QString("Non-Base64 character was encountered.");
		val |= base64_val[c] << (i * 6);
	}
	return val;
}

qint64 last_insert_rowid(QSqlQuery &q) {
	q.exec("SELECT last_insert_rowid();");
	q.next();
	return q.value(0).toLongLong();
}
