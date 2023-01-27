#include "mainwindow.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QObject>
#include <QNetworkReply>
#include <QtCore>


int main(int argc, char *argv[])
{
    
    auto* a = new QApplication(argc, argv);
    QCoreApplication::addLibraryPath(QString("./opensslruntime"));
    MainWindow w;
    w.show();
    qDebug() << "SslSupport: " << QSslSocket::supportsSsl();
    qDebug() << "SslLibraryBuildVersion: " << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "SslLibraryRuntimeVersion: " << QSslSocket::sslLibraryVersionString();
	return (*a).exec();

}
