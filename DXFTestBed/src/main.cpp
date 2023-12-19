#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
