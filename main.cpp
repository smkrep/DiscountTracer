#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QObject>
#include <QNetworkReply>
#include <QtCore>


int main(int argc, char *argv[])
{
    auto* a = new QApplication(argc, argv);
    MainWindow w;
    w.show();
	return (*a).exec();

}
