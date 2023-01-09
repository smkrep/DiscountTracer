#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtCore>
#include <QtGui>

void wait() {};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////MAINWINDOW DEFINITIONS////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openDll(fs::path path) {
#ifdef _WIN32
    HINSTANCE load;
    load = LoadLibrary(path.string().c_str());
    MainWindow::insertDllIntoList(load, path);

#else
    void* load;
    load = dlopen(path.c_str(), RTLD_LAZY);
    insertDllIntoList(load, path);
#endif
    
}

void MainWindow::closeDll(){
    
#ifdef _WIN32
    for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {
        FreeLibrary((*iter).first);
    }

#else
    for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {
        dlclose((*iter).first);
    }
#endif
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stopButton->setDisabled(true);  

    #ifdef _WIN32
        for (const auto& file : fs::directory_iterator("./dlls")) {
            if (!fs::is_directory(file)) {

                if (fs::path(file).extension() == ".dll")
                {
                    fs::path p = fs::path(file);
                    openDll(p);
                }
            }
        }

        for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {

            std::string filename = (*iter).second.filename().string(), substring = ".dll";
            std::size_t ptr = filename.find(substring);

            if (ptr != std::string::npos) {
                filename.erase(ptr, substring.length());
            }

            ui->ShopsList->addItem(QString::fromStdString(filename));
        }
    #else

        for (const auto& file : fs::directory_iterator("./dlls")) {
            if (!fs::is_directory(file)) {

                if (fs::path(file).extension() == ".so")
                {
                    fs::path p = fs::path(file);
                    openDll(p);
                }
            }
        }

        for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {

            std::string filename = (*iter).second.filename().string(), substring = ".so", substring1 = "lib";

            std::size_t ptr = filename.find(substring1);
            if (ptr != std::string::npos) {
                filename.erase(ptr, substring1.length());
            }
        
            ptr = filename.find(substring);
            if (ptr != std::string::npos) {
                filename.erase(ptr, substring.length());
            }

            ui->ShopsList->addItem(QString::fromStdString(filename));
        }

    #endif  

    manager = new QNetworkAccessManager();

    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this,
        SLOT(linkCheckFinished(QNetworkReply*)));
   
    connect(this, &MainWindow::newItemAdded, this, &MainWindow::updateList);

    thread = new QThread(this);
}

MainWindow::~MainWindow()
{  
    if (!itemsList.empty()) {
        exportListToJson();  
    }
    if (thread != nullptr && thread->isRunning()) {
        QEventLoop loop;
        connect(thread, SIGNAL(finished()), &loop, SLOT(quit()));
        thread->quit();
        loop.exec();
        delete thread;
        
    }
    else {
        delete thread;
    }
    delete ui;
    delete manager;
    closeDll();

}

void MainWindow::on_addItemButton_clicked()
{
    if (ui->linkLineEdit->text() == "") {
        ui->statusLabel->setStyleSheet("QLabel {color:red;}");
        ui->statusLabel->setText(QString::fromLocal8Bit("Введите ссылку!"));

        QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        return;
    }
    else {

        request.setUrl(QUrl(ui->linkLineEdit->text()));
        manager->get(request);
        QTimer::singleShot(600, this, &MainWindow::processInput);

    }
}

void MainWindow::clearStatusLabel() {
    ui->statusLabel->clear();
}

void MainWindow::on_deleteItemButton_clicked() {
    int r = ui->ItemsList->currentRow();


    if (r != -1) {

        deleteItem(ui->ItemsList->item(r)->text());

        QListWidgetItem* listitem = ui->ItemsList->takeItem(r);

        delete listitem;
        ui->exportConfigButton->setEnabled(true);
    }
    else {
        ui->statusLabel->setStyleSheet("QLabel {color:red;}");
        ui->statusLabel->setText(QString::fromLocal8Bit("Выберите товар для удаления!"));

        QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
    }
}

void MainWindow::on_startButton_clicked() {
    updateList();
    ui->startButton->setDisabled(true);
    ui->stopButton->setEnabled(true);
    ui->addItemButton->setDisabled(true);

    
    worker = new Worker();
    connect(this, SIGNAL(startProcessing()), worker, SLOT(process()));

    connect(worker, SIGNAL(discount(QString)), this, SLOT(discountSlot(QString)));
    connect(worker, SIGNAL(updateListSignal()), this, SLOT(updateList()));
    connect(worker, SIGNAL(viewTime()), this, SLOT(viewTimeSlot()));


    
    connect(this, SIGNAL(stopButtonActivated()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));

    #ifdef _WIN32

        for (auto i = itemsList.begin(); i != itemsList.end(); i++) {

            for (auto iterator = dynLibsList.begin(); iterator != dynLibsList.end(); iterator++) {
                if ((*iterator).second.filename().string() == (*i).getShop().toStdString() + ".dll") {
                    (*i).setHandle((*iterator).first);
                    break;
                }
            }
        }
        
    #else

        for (auto i = itemsList.begin(); i != itemsList.end(); i++) {

            for (auto iterator = dynLibsList.begin(); iterator != dynLibsList.end(); iterator++) {
                if ((*iterator).second.filename().string() == "lib" + (*i).getShop().toStdString() + ".so") {
                    (*i).setHandle((*iterator).first);
                    break;
                }
            }
        }

    #endif
    worker->copyItems(itemsList);

    worker->moveToThread(thread);
    (thread)->start();
    emit startProcessing();
    
}

void MainWindow::updateList() {
    ui->ItemsList->clear();

    for (auto iter = itemsList.begin(); iter != itemsList.end(); iter++) {
        if (iter->getDiscountStatus()) {
            QListWidgetItem* itm = new QListWidgetItem();
            itm->setText(iter->getName());
            itm->setTextColor(Qt::green);
            ui->ItemsList->addItem(itm);
            
        }
        else {
            ui->ItemsList->addItem(iter->getName());
        }

    }
}

void MainWindow::discountSlot(QString name) {
    for (auto iter = itemsList.begin(); iter != itemsList.end();iter++) {
        if (iter->getName() == name) {
            iter->setDiscountStatus(true);
            break;
        }
    }
    
}

void MainWindow::on_stopButton_clicked() {
    emit stopButtonActivated();
    thread->quit();
    ui->startButton->setEnabled(true);
    ui->stopButton->setDisabled(true);
    ui->addItemButton->setDisabled(false);

    for (auto it = itemsList.begin(); it != itemsList.end(); it++) {
        it->setDiscountStatus(false);
    }
    updateList();
}

void MainWindow::viewTimeSlot() {
    QDateTime lastTimeChecked = QDateTime::currentDateTime();
    QString time = lastTimeChecked.toString();
    std::string timestr = time.toStdString();
    size_t found = timestr.find("2023");
    timestr.erase(found, 4);
    ui->threadStatusLabel->setText(QString::fromLocal8Bit("Последняя проверка осуществлена:\n\n ") + QString::fromStdString(timestr));
}

bool MainWindow::shopParsingIsSupported(const QString& shopname) {
#ifdef _WIN32
    for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {
        if ((*iter).second.filename() == shopname.toStdString() + ".dll") {
            return true;
        }
}
    return false;
#else
    for (auto iter = dynLibsList.begin(); iter != dynLibsList.end(); iter++) {
        if ((*iter).second.filename() == "lib" + shopname.toStdString() + ".so") { 
            return true;
        }
    }
    return false;
#endif
}

void MainWindow::exportListToJson() {
    QJsonDocument document;
    QJsonArray content;
    if (itemsList.empty()) {
        ui->statusLabel->setStyleSheet("QLabel {color:red;}");
        ui->statusLabel->setText(QString::fromLocal8Bit("Добавьте хотя бы один товар!"));

        QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        return;
    
    }
    else {
        for (auto iter = itemsList.begin(); iter != itemsList.end(); iter++) {
            
            QJsonObject item;
            item.insert("nickname", iter->getName());
            item.insert("link", iter->getLink().toString());
            item.insert("time_interval(minutes)", iter->getTimespan());
            item.insert("shop", iter->getShop());

            content.append(item);
        }
        QJsonObject final_obj;
        final_obj.insert("items", content);

        document.setObject(final_obj);
        QByteArray bytes = document.toJson(QJsonDocument::Indented);

        QFile file("./config.json");

        if (!file.exists()) {
            std::ofstream{ "config.json" };
        }
        else {
            file.remove();
            std::ofstream{ "config.json" };
        }

        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream iStream(&file);
            iStream.setCodec("utf-8");
            iStream << bytes;
            file.close();

            ui->statusLabel->setStyleSheet("QLabel {color:green;}");
            ui->statusLabel->setText(QString::fromLocal8Bit("Товары сохранены!"));

            QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        }
        else {
            ui->statusLabel->setStyleSheet("QLabel {color:red;}");
            ui->statusLabel->setText(QString::fromLocal8Bit("Не удалось открыть файл!"));

            QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        }

        ui->exportConfigButton->setDisabled(true);
    
    }
}

void MainWindow::importListFromJson() {
    QJsonDocument document;
    QJsonArray content;
    QJsonObject rootObj;
    QFile file("./config.json");

    if (!file.exists()) {
        QMessageBox::information(this, QString::fromLocal8Bit("Ошибка загрузки"), QString::fromLocal8Bit("Файл config.json не найден!"));
        return;
    }

    QByteArray bytes;

    if (file.open(QIODevice::ReadOnly)) {        
        bytes = file.readAll();
        file.close();
    
    }
    else {

        QMessageBox::information(this, QString::fromLocal8Bit("Ошибка открытия"), QString::fromLocal8Bit("Не удалось открыть config.json!"));
        return;  
    }

    QJsonParseError jsonError;
    document = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("Ошибка парсинга"), QString::fromLocal8Bit("Не удалось распарсить config.json!"));
        return;
    }

    rootObj = document.object();
    content = rootObj.value("items").toArray();

    itemsList.clear();

    foreach(const QJsonValue & val, content) {
        QUrl link = QUrl(val.toObject().value("link").toString());
        QString nickname = val.toObject().value("nickname").toString();
        QString shop = val.toObject().value("shop").toString();
        int timespan = val.toObject().value("time_interval(minutes)").toInt();
        Item item(link, timespan, nickname, shop);
        itemsList.push_back(item);
    }

    updateList();

    ui->statusLabel->setStyleSheet("QLabel {color:green;}");
    ui->statusLabel->setText(QString::fromLocal8Bit("Товары из конфига подгружены!"));

    QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);

}

void MainWindow::on_exportConfigButton_clicked() {
    exportListToJson();
}

void MainWindow::on_importConfigButton_clicked() {
    importListFromJson();
}

#ifdef _WIN32
void MainWindow::insertDllIntoList(const HINSTANCE& mem, const fs::path& filepath) {
    std::pair elem = std::make_pair(mem, filepath);
    dynLibsList.push_back(elem);
}

#else
void MainWindow::insertDllIntoList(void* mem, fs::path& filepath) { 
    std::pair elem = std::make_pair(mem, filepath);
    dynLibsList.push_back(elem);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ITEM DEFINITIONS//////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString getShopName(const std::string& temp) {

    QString name;
    std::size_t found = temp.find("www.");
    if (found != std::string::npos) {
        found += 4;
        while (temp[found] != '.') {
            name += temp[found];
            found++;
        }
    }
    else {
        std::size_t found = temp.find("://");  //алгоритм для определения имени магазина, на который ведет ссылка
        if (found != std::string::npos) {
            found += 3;
            while (temp[found] != '.') {
                name += temp[found];
                found++;
            }
        }
        else {
            found = 0;
            while (temp[found] != '.' && found != temp.size()) {
                name += temp[found];
                found++;
            }
        }
    }

    if (MainWindow::shopParsingIsSupported(name)) {
        return name;
    }
    else {
        return "";
    }

}

Item::Item(const QUrl& link, const int& timespan, const QString& nickname) {

    link_ = link;
    if (nickname == "") {
        nickname_ = QString::fromLocal8Bit("Безымянный товар ") + QString::number(MainWindow::defaultNameNum_);
    }
    else {
        nickname_ = nickname;
    }
    if (timespan == 0) {
        timespan_ = 5;
    
    }
    else {
        timespan_ = timespan;
    }
    
    QString convert = link.toString(), name;
    std::string temp = convert.toStdString();

    shop_ = getShopName(temp);
    
}

Item::Item(const QUrl& link, const int& timespan, const QString& nickname, const QString& shop) {
    link_ = link;
    timespan_ = timespan;
    nickname_ = nickname;
    shop_ = shop;
}

QString Item::getName() const {
    return this->nickname_;
}

QString Item::getShop() const {
    return this->shop_;
}

int Item::getTimespan() const {
    return this->timespan_;
}

QUrl Item::getLink() const {
    return this->link_;
}

std::chrono::steady_clock::time_point Item::getLastCheckupTime() const {
    return this->lastCheckupTime_;
}

void Item::setLastCheckupTime(std::chrono::steady_clock::time_point timepoint) {
    lastCheckupTime_ = timepoint;
}

void Item::setDiscountStatus(bool status) {
    isDiscounted_ = status;
}

bool Item::getDiscountStatus() const{
    return this->isDiscounted_;
}

#ifdef _WIN32
void Item::setHandle(HINSTANCE load) {
    load_ = load;
}

HINSTANCE Item::getHandle() const {
    return this->load_;
}
#else
void Item::setHandle(void* handle) {
    handle_ = handle;
}

void* Item::getHandle() const {
    return this->handle_;
}
#endif

bool comparator(Item a, Item b) {
    return a.getTimespan() < b.getTimespan();
}

void MainWindow::sortItemsList() {
    std::sort(itemsList.begin(), itemsList.end(), comparator);
}

void MainWindow::deleteItem(const QString& nickname) {
    defaultNameNum_--;
    for (std::vector<Item>::iterator iter = itemsList.begin(); iter != itemsList.end(); iter++) {
        if ((*iter).getName() == nickname) {
            itemsList.erase(iter);
            break;
        }
    }
}

int MainWindow::itemsListSize() {
    return itemsList.size();
}

QString MainWindow::getItemName(const int& index) {
    return itemsList[index].getName();
}

void MainWindow::processInput() {

    if (!linkIsValid) {
        ui->statusLabel->setStyleSheet("QLabel {color:red;}");
        ui->statusLabel->setText(QString::fromLocal8Bit("Ссылка невалидна \n либо отсутствует подключение к Интернету!"));

        QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        return;
    }
    else if (linkIsValid && getShopName(ui->linkLineEdit->text().toStdString()) == "") {
        ui->statusLabel->setStyleSheet("QLabel {color:red;}");
        ui->statusLabel->setText(QString::fromLocal8Bit("В настоящее время сайт не поддерживается!"));

        QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
        return;
    }
    else {
        defaultNameNum_++;
        bool ok;
        ui->timespanLineEdit->text().toInt(&ok);
        if (!ok && ui->timespanLineEdit->text() != "") {
            ui->statusLabel->setStyleSheet("QLabel {color:red;}");
            ui->statusLabel->setText(QString::fromLocal8Bit("Укажите валидный интервал опроса!"));

            QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
            return;
        }
        else {
            Item item(ui->linkLineEdit->text(), ui->timespanLineEdit->text().toInt(), ui->nicknameLineEdit->text());
            itemsList.push_back(item);
            ui->statusLabel->setStyleSheet("QLabel {color:green;}");
            ui->statusLabel->setText(QString::fromLocal8Bit("Товар успешно добавлен!"));
            
            QTimer::singleShot(2000, this, &MainWindow::clearStatusLabel);
            ui->linkLineEdit->clear();
            ui->timespanLineEdit->clear();
            ui->nicknameLineEdit->clear();
            emit newItemAdded();
            ui->exportConfigButton->setEnabled(true);
        }
    }
}

void MainWindow::linkCheckFinished(QNetworkReply* reply) {
    if (reply->error()) {
        linkIsValid = false;
    }
    else {
        linkIsValid = true;
    }


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////WORKER DEFINITIONS////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


Worker::Worker(QObject* parent) : QObject(parent) {
}

Worker::~Worker() {
    timer->stop();
    delete timer;
    delete netManager;
    itemsListCopy.clear();
}

void Worker::process() {
    netManager = new QNetworkAccessManager();

    for (auto item : itemsListCopy) {
        item.setLastCheckupTime(std::chrono::steady_clock::now());
    }
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Worker::getHTML);
    timer->start(3000); 
}

void Worker::getHTML() {
    std::chrono::steady_clock::time_point currCheckUpTime = std::chrono::steady_clock::now();

    for (auto iter = itemsListCopy.begin(); iter != itemsListCopy.end(); iter++) {
        if (std::chrono::duration_cast<std::chrono::seconds>(currCheckUpTime - (*iter).getLastCheckupTime()) > std::chrono::seconds((*iter).getTimespan() - 1)) {
            
            netRequest.setUrl((*iter).getLink());
            QNetworkReply* reply = netManager->get(netRequest);

            QEventLoop loop;
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();

            QString answer = reply->readAll();

            #ifdef _WIN32
                typedef bool (*parser) (std::string);
                parser parse;
                parse = (parser)*GetProcAddress((*iter).getHandle(), "parse");
            #else
                bool (*parse) (std::string);
                parse = (bool (*)(std::string))dlsym((*iter).getHandle(), "parse");
            #endif

            //migalka++;
            if (parse(answer.toStdString())) {
                emit discount((*iter).getName());
            }

            (*iter).setLastCheckupTime(std::chrono::steady_clock::now());

            netManager->clearAccessCache();
            delete reply;
        }
    }
    emit updateListSignal();
    emit viewTime();

}

void Worker::findDiscount(QNetworkReply* reply) {
    QString answer = reply->readAll();

//#ifdef _WIN32
//    typedef bool (*parser) (std::string);
//    parser parse;
//    parse = (parser)*GetProcAddress(load_, "parse");
//#else
//    bool (*parse) (std::string);
//    parse = (bool (*)(std::string))dlsym(handle_, "parse");
//#endif
//    migalka++;
//
//    if (parse(answer.toStdString())) {
//        emit discount((*iter_).getName());
//    }
//    else {
//        //emit noDiscount();
//    }
}

void Worker::copyItems(std::vector<Item> items) {
    itemsListCopy = items;
}


