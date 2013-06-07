#include <QApplication>
#include "qmlapplicationviewer.h"
#include "mainobject.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
	QScopedPointer<QApplication> app(createApplication(argc, argv));

	MainObject mainObject;

	return app->exec();
}
