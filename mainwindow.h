#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

////QT INCLUDES////
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

////STD INCLUDES////
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>


#ifdef _WIN32
#include <windows.h>
#else                   
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Item {
public:
    Item(const QUrl& link, const int& timespan, const QString& nickname); 
    Item(const QUrl& link, const int& timespan, const QString& nickname, const QString& shop);
    Item() = default;                                                                         
    ~Item() = default;

    QString getName() const;
    int getTimespan() const;
    QUrl getLink() const;
    QString getShop() const;
    std::chrono::steady_clock::time_point getLastCheckupTime() const;
    bool getDiscountStatus() const;


    void setLastCheckupTime(std::chrono::steady_clock::time_point timepoint);
    void setDiscountStatus(bool status);
    

    
    #ifdef _WIN32
    void setHandle(HINSTANCE load);
    HINSTANCE getHandle() const;
    #else
    void setHandle(void* handle);
    void* getHandle() const;
    #endif
    

private:
    QUrl link_;
    int timespan_;
    QString nickname_;
    QString shop_;
    std::chrono::steady_clock::time_point lastCheckupTime_;
    bool isDiscounted_ = false;

    #ifdef _WIN32
        HINSTANCE load_;
    #else
        void* handle_;
    #endif
};




class Worker : public QObject {
    Q_OBJECT

public:
    friend class MainWindow;
    Worker(QObject *parent = nullptr);
    ~Worker();

    void checkDiscount();

    void copyItems(std::vector<Item> items);


public slots:
    void process();
signals:
    void finished();
    void discount(QString);
    void updateListSignal();
    void viewTime();
    

private:
    QNetworkAccessManager* netManager;
    QNetworkRequest netRequest;
    QTimer* timer;
    std::vector<Item> itemsListCopy;
    

    std::vector<Item>::iterator iter_;
};




class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    inline static int defaultNameNum_ = 1;
    #ifdef _WIN32
        static void insertDllIntoList(const HINSTANCE& mem, const fs::path& filepath);

    #else
        static void insertDllIntoList(void* mem, fs::path& filepath); 
    #endif

    static bool shopParsingIsSupported(const QString& shopname);

    void openDll(fs::path path);
    void closeDll();

    static QString getItemName(const int& index);

    static int itemsListSize();

    static void deleteItem(const QString& nickname);

    static void sortItemsList();

    void processInput();

    void exportListToJson();

    void importListFromJson();


private:
    Worker *worker;
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    QNetworkRequest request;
    QThread* thread;
    QTimer* timer;

    inline static bool linkIsValid = false;
    inline static std::vector<Item> itemsList;

    #ifdef _WIN32
       inline static std::vector<std::pair<HINSTANCE, fs::path>> dynLibsList;
    #else
       inline static std::vector<std::pair<void*, fs::path>> dynLibsList; 
    #endif
    

signals:
    void stopButtonActivated();
    void newItemAdded();
    void stopTracingDiscount();
    void startProcessing();
    

private slots:
    void on_addItemButton_clicked();
    void updateList();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_deleteItemButton_clicked();

    void linkCheckFinished(QNetworkReply* reply);
    void clearStatusLabel();

    void discountSlot(QString name);

    void viewTimeSlot();

    void on_exportConfigButton_clicked();
    void on_importConfigButton_clicked();

};

#endif // MAINWINDOW_H
