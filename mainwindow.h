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

/// <summary>
/// Класс, экземпляр которого содержит в себе необходимую информацию о каком-либо товаре
/// </summary>
class Item {
public:
    /// <summary>
    /// Конструктор класса Item, создает объект Item и заполняет поля ссылки, интервала опроса и метки (из аргументов),  
    /// в самом конструкторе заполняется поле, содержащее информацию о магазине. Используется при ручном добавлении товара пользователем
    /// </summary>
    /// <param name="link">Ссылка на товар</param>
    /// <param name="timespan">Интервал опроса на наличие скидки(в минутах)</param>
    /// <param name="nickname">Метка товара</param>
    Item(const QUrl& link, const int& timespan, const QString& nickname);
    /// <summary>
    /// Перегрузка конструктора, имеющая дополнительный аргумент shop, используется при автоматическом
    /// заполнении списка товаров из файла конфигурации
    /// </summary>
    /// <param name="link">Ссылка на товар</param>
    /// <param name="timespan">Интервал опроса на наличие скидки(в минутах)</param>
    /// <param name="nickname">Метка товара</param>
    /// <param name="shop">Название магазина, в котором представлен товар</param>
    Item(const QUrl& link, const int& timespan, const QString& nickname, const QString& shop);
    /// <summary>
    /// Умолчательный конструктор 
    /// </summary>
    Item() = default;
    /// <summary>
    /// Умолчательный деструктор
    /// </summary>
    ~Item() = default;
    /// <summary>
    /// Метод, использующийся для получения метки товара
    /// </summary>
    /// <returns>Метку товара</returns>
    QString getName() const;
    /// <summary>
    /// Метод, использующийся для получения интервала опроса товара
    /// </summary>
    /// <returns>Интервал опроса товара(в минутах)</returns>
    int getTimespan() const;
    /// <summary>
    /// Метод, использующийся для получения ссылки на товар
    /// </summary>
    /// <returns>Ссылку на товар</returns>
    QUrl getLink() const;
    /// <summary>
    /// Метод, использующийся для получения названия магазина
    /// </summary>
    /// <returns>Название магазина, в котором представлен товар</returns>
    QString getShop() const;
    /// <summary>
    /// Метод, использующийся для получения момента последнего опроса товара
    /// </summary>
    /// <returns>Временную метку последней проверки товара</returns>
    std::chrono::steady_clock::time_point getLastCheckupTime() const;
    /// <summary>
    /// Метод, использующийся для определения статуса товара (есть на него скидка или нет)
    /// </summary>
    /// <returns>True в случае, если скидка на товар была найдена, иначе false</returns>
    bool getDiscountStatus() const;
    /// <summary>
    /// Метод, использующийся для получения названия магазина, в котором представлен товар, из ссылки на этот товар
    /// </summary>
    /// <param name="temp">Ссылка на товар</param>
    /// <returns>Имя магазина, в случае если поддерживается его парсинг, иначе - пустую строку</returns>
    static QString getShopNameFromLink(const std::string& temp);

    /// <summary>
    /// Метод, позволяющий установить время последней проверки товара на наличие скидки.
    /// </summary>
    /// <param name="timepoint">Время последней проверки</param>
    void setLastCheckupTime(std::chrono::steady_clock::time_point timepoint);
    /// <summary>
    /// Метод, позволяющий установить статус наличия скидки у товара
    /// </summary>
    /// <param name="status">Статус наличия скидки</param>
    void setDiscountStatus(bool status);
    
    
    #ifdef _WIN32
        /// <summary>
        /// Метод, устанавливающий значение дескриптора библиотеки, в которой содержится код для проверки наличия скидки на данный товар. 
        /// Объявляется и определяется только при сборке под Windows
        /// </summary>
        /// <param name="load">Дескриптор библиотеки</param>
        void setHandle(HINSTANCE load);
    #else
        /// <summary>
        /// Метод, устанавливающий значение дескриптора библиотеки, в которой содержится код для проверки наличия скидки на данный товар.
        /// Объявляется и определяется только при сборке не под Windows
        /// </summary>
        /// <param name="handle">Дескриптор библиотеки</param>
        void setHandle(void* handle);
    #endif

    #ifdef _WIN32
        /// <summary>
        /// Метод, позволяющий получить значение дескриптора библиотеки, в которой содержится код для проверки наличия скидки на данный товар. 
        /// Объявляется и определяется только при сборке под Windows
        /// </summary>
        /// <returns>Дескриптор библиотеки</returns>
        HINSTANCE getHandle() const;
    #else
        /// <summary>
        /// Метод, позволяющий получить значение дескриптора библиотеки, в которой содержится код для проверки наличия скидки на данный товар. 
        /// Объявляется и определяется только при сборке не под Windows
        /// </summary>
        /// <returns>Дескриптор библиотеки</returns>
        void* getHandle() const;
    #endif
    
    

private:
    /// <summary>
    /// Поле, содержащее в себе обязательно валидную ссылку на товар.
    /// </summary>
    QUrl link_;
    /// <summary>
    /// Поле, содержащее в себе обязательно валидный интервал опроса на наличие скидки (в минутах). Если таковой не указан пользователем,
    /// устанавливается умолчательное значение в 15 минут.
    /// </summary>
    int timespan_;
    /// <summary>
    /// Поле, содержащее в себе метку товара. Если таковая не указана пользователем, устанавливается умолчательная метка "Безымянный товар X",
    /// где X - порядковый номер.
    /// </summary>
    QString nickname_;
    /// <summary>
    /// Поле, содержащее в себе название магазина, в котором представлен товар
    /// </summary>
    QString shop_;
    /// <summary>
    /// Поле, содержащее в себе временную метку, указывающую на момент, когда в последний раз проверялось наличие скидки на товар
    /// </summary>
    std::chrono::steady_clock::time_point lastCheckupTime_;
    /// <summary>
    /// Поле, содержащее в себе информацию, есть ли скидка на данный товар
    /// </summary>
    bool isDiscounted_ = false;

    #ifdef _WIN32
        /// <summary>
        /// Поле, содержащее в себе дескриптор библиотеки, в которой находится код для проверки наличия скидки на данный товар.
        /// Объявляется и определяется только при сборке под Windows
        /// </summary>
        HINSTANCE load_;
    #else
        /// <summary>
        /// Поле, содержащее в себе дескриптор библиотеки, в которой находится код для проверки наличия скидки на данный товар.
        /// Объявляется и определяется только при сборке не под Windows
        /// </summary>
        void* handle_;
    #endif
};



/// <summary>
/// Класс, который предоставляет функционал для периодической проверки товаров на наличие скидок
/// </summary>
class Worker : public QObject {
    Q_OBJECT

public:
    friend class MainWindow;
    /// <summary>
    /// Конструктор класса Worker. В аргументы конструктора можно передать указатель на родительский класс,
    /// что передаст управление памятью экземпляру родительского класса
    /// </summary>
    /// <param name="parent">Указатель на родительский класс</param>
    Worker(QObject *parent = nullptr);

    /// <summary>
    /// Деструктор класса Worker. В нем происходит освобождение памяти динамически выделенных полей netManager и timer,
    /// также очищается копия массива товаров
    /// </summary>
    ~Worker();
    /// <summary>
    /// Метод, раз в 15 минут проходящий по массиву товаров и проверяющий, пришло ли время проверять наличие скидки
    /// </summary>
    void checkDiscount();
    /// <summary>
    /// Метод, копирующий массив товаров, хранящийся в экземпляре класса MainWindow, в приватный массив класса Worker
    /// </summary>
    /// <param name="items">Массив товаров</param>
    void copyItems(std::vector<Item> items);


public slots:
    /// <summary>
    /// Слот, вызывающийся во время начала работы потока-обработчика. Создает таймер и запускает его, создает менеджер
    /// подключения к сети
    /// </summary>
    void process();

signals:
    /// <summary>
    /// Сигнал, испускаeмый методом process при обнаружении скидки. Передает имя товара в слот-обработчик discountSlot,
    /// находящийся в классе MainWindow;
    /// </summary>
    void discount(QString);
    /// <summary>
    /// Сигнал, испускаемый методом process в конце опроса всех товаров на наличие скидки. Обрабатывается слотом updateList,
    /// находящимся в классе MainWindow
    /// </summary>
    void updateListSignal();
    /// <summary>
    /// Сигнал, испускаемый методом process в конце опроса всех товаров на наличие скидки. Обрабатывается слотом viewTimeSlot,
    /// находящимся в классе MainWindow
    /// </summary>
    void viewTime();
    

private:
    /// <summary>
    /// Поле, хранящее в себе указатель на менеджер доступа к сети
    /// </summary>
    QNetworkAccessManager* netManager;
    /// <summary>
    /// Поле, хранящее в себе сетевой запрос, передаваемый менеджеру доступа к сети
    /// </summary>
    QNetworkRequest netRequest;
    /// <summary>
    /// Поле, хранящее в себе указатель на таймер
    /// </summary>
    QTimer* timer;
    /// <summary>
    /// Копия массива товаров, содержащегося в классе MainWindow
    /// </summary>
    std::vector<Item> itemsListCopy;
    
};



/// <summary>
/// Главный класс, отображающий графический интерфейс и содержащий в себе обработчики пользовательского ввода,
/// функционал работы с динамическими библиотеками и создания/подгрузки файлов конфигурации
/// </summary>
class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    /// <summary>
    /// Конструктор класса MainWindow. В аргументы конструктора можно передать указатель на родительский класс,
    /// что передаст управление памятью экземпляру родительского класса. В конструкторе происходит поиск и открытие
    /// доступных динамических библиотек, создаются экземпляры менеджера доступа к сети и экземпляр потока-обработчика
    /// </summary>
    /// <param name="parent">Указатель на родительский класс</param>
    MainWindow(QWidget *parent = nullptr);
    /// <summary>
    /// Деструктор класса MainWindow. Удаляет динамически выделенную память, высвобождает динамические библиотеки,
    /// останавливает и удаляет поток-обработчик, создает файл конфигурации и помещает его рядом с DiscountTracer.exe 
    /// </summary>
    ~MainWindow();
    /// <summary>
    /// Метод, определяющий, поддерживается ли в данный момент парсинг конкретного магазина
    /// </summary>
    /// <param name="shopname">Имя магазина</param>
    /// <returns>True, если парсинг поддерживается, иначе - false</returns>
    static bool shopParsingIsSupported(const QString& shopname);
    /// <summary>
    /// Метод, выполняющий подгрузку динамических библиотек
    /// </summary>
    /// <param name="path">Путь к динамической библиотеке</param>
    void openDll(fs::path path);
    /// <summary>
    /// Метод, выгружающий все динамические библиотеки
    /// </summary>
    void closeDll();
    /// <summary>
    /// Метод, удаляющий товар из массива товаров
    /// </summary>
    /// <param name="nickname">Метка товара</param>
    static void deleteItem(const QString& nickname);
    /// <summary>
    /// Метод, обрабатывающий введенные пользователем атрибуты товара и создающие на основе них товар.
    /// Также выполняет проверку введенных данных на валидность.
    /// </summary>
    void processInput();
    /// <summary>
    /// Метод, сохраняющий внесенные товары в файл конфигурации config.json. Файл создается в той же директории,
    /// где лежит главный исполняемый файл DiscountTracer.exe
    /// </summary>
    void exportListToJson();
    /// <summary>
    /// Метод, импортирующий товары из файла конфигурации в массив товаров.
    /// </summary>
    void importListFromJson();

    #ifdef _WIN32
        /// <summary>
        /// Метод, создающий пару из значений дескриптора динамической библиотеки и пути к ней. После создания пары, она помещается
        /// в массив dynLibsList. Объявляется и определяется только при сборке под Windows
        /// </summary>
        /// <param name="mem">Дескриптор библиотеки</param>
        /// <param name="filepath">Путь к библиотеке</param>
        static void insertDllIntoList(const HINSTANCE& mem, const fs::path& filepath);

    #else
        /// <summary>
        /// Метод, создающий пару из значений дескриптора динамической библиотеки и пути к ней. После создания пары, она помещается
        /// в массив dynLibsList. Объявляется и определяется только при сборке не под Windows
        /// </summary>
        /// <param name="mem">Дескриптор библиотеки</param>
        /// <param name="filepath">Путь к библиотеке</param>
        static void insertDllIntoList(void* mem, fs::path& filepath);
    #endif

    /// <summary>
    /// Поле, отвечающее за нумерацию безымянных товаров
    /// </summary>
    inline static int defaultNameNum_ = 1;


private:
    /// <summary>
    /// Поле, хранящее указатель на экземпляр класса, работающего с проверкой товаров на наличие скидки
    /// </summary>
    Worker *worker;
    /// <summary>
    /// Поле, хранящее указатель на экземпляр, отвечающий за графический интерфейс и его элементы
    /// </summary>
    Ui::MainWindow *ui;
    /// <summary>
    /// Поле, хранящее указатель на экземпляр менеджера доступа к сети
    /// </summary>
    QNetworkAccessManager* manager;
    /// <summary>
    /// Поле, хранящее в себе сетевой запрос, передаваемый менеджеру доступа к сети
    /// </summary>   
    QNetworkRequest request;
    /// <summary>
    /// Поле, хранящее указатель на экземпляр потока-обработчика
    /// </summary>
    QThread* thread;

    /// <summary>
    /// Поле, хранящее информацию о валидности введенной пользователем ссылки на товар
    /// </summary>
    inline static bool linkIsValid = false;
    /// <summary>
    /// Массив товаров, внесенных в программу пользователем или подгруженных из файла конфигурации
    /// </summary>
    inline static std::vector<Item> itemsList;

    #ifdef _WIN32
       /// <summary>
       /// Массив, содержащий в себе информацию об открытых динамических библиотеках. Хранит дескрипторы и пути
       /// к библиотекам. Объявляется и определяется только при сборке под Windows
       /// </summary>
       inline static std::vector<std::pair<HINSTANCE, fs::path>> dynLibsList;
    #else
       /// <summary>
       /// Массив, содержащий в себе информацию об открытых динамических библиотеках. Хранит дескрипторы и пути
       /// к библиотекам. Объявляется и определяется только при сборке не под Windows
       /// </summary>
       inline static std::vector<std::pair<void*, fs::path>> dynLibsList; 
    #endif
    

signals:
    /// <summary>
    /// Сигнал, после испускания которого поток-обработчик останавливается.
    /// </summary>
    void stopButtonActivated();
    /// <summary>
    /// Сигнал, который испускается при успешном добавлении пользователем нового товара. Обрабатывается слотом
    /// updateList
    /// </summary>
    void newItemAdded();
    /// <summary>
    /// Сигнал, после испускания которого запускается цикл опроса товаров на наличие скидки
    /// </summary>
    void startProcessing();
    

private slots:
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Добавить товар". Проверяет ссылку на товар на валидность,
    /// после чего передает управление методу processInput
    /// </summary>
    void on_addItemButton_clicked();
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Начать отслеживание". Создает экземпляр класса-обработчика, выполняет
    /// копирование необходимой информации в его поля, передает его в поток-обработчик и запускает последний.
    /// </summary>
    void on_startButton_clicked();
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Прекратить отслеживание". Завершает работу потока-обработчика, очищает
    /// информацию о скидках на товары
    /// </summary>
    void on_stopButton_clicked();
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Удалить товар". Удаляет товар, выбранный пользователем в таблице товаров
    /// </summary>
    void on_deleteItemButton_clicked();
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Экспортировать товары". Внутри себя вызывает метод exportListToJson
    /// </summary>
    void on_exportConfigButton_clicked();
    /// <summary>
    /// Слот-обработчик нажатия на кнопку "Импортировать товары". Внутри себя вызывает метод importListFromJson
    /// </summary>
    void on_importConfigButton_clicked();
    /// <summary>
    /// Слот-обработчик ответа менеджера доступа к сети. На основе полученного ответа делает вывод о валидности
    /// введенной пользователем ссылки
    /// </summary>
    /// <param name="reply">Результат работы менеджера доступа к сети</param>
    void linkCheckFinished(QNetworkReply* reply);
    /// <summary>
    /// Слот, очищающий поле, отображающее статус выполненной операции. Вызывается через две секунды после
    /// показа статуса операции пользователю.
    /// </summary>
    void clearStatusLabel();
    /// <summary>
    /// Слот, обновляющий список товаров. Если на товар появилась скидка, окрашивает его метку в зеленый
    /// </summary>
    void updateList();
    /// <summary>
    /// Слот-обработчик сигнала о появившейся скидке. Выставляет полученному товару наличие
    /// скидки в поле isDiscounted_ класса Item
    /// </summary>
    /// <param name="name">Метка товара, на который появилась скидка</param>
    void discountSlot(QString name);
    /// <summary>
    /// Слот-обработчик сигнала о завершении итерации цикла опроса товаров. Отображает пользователю время последнего опроса
    /// (день недели, число, месяц, время в формате ЧЧ:ММ::СС)
    /// </summary>
    void viewTimeSlot();
};

#endif // MAINWINDOW_H
