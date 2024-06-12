#include "mainwindow.h"

#include <QPushButton>
#include <QLabel>
#include <QtSql>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Получаем параметры подключения из хэдера, вынесенного в gitIgnore
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setUserName(DBSettings::userName());
    db.setHostName(DBSettings::hostName());
    db.setPassword(DBSettings::password());

    // Создаем основной виджет и основной слой
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    // Создаем stackedWidget и добавляем его на основной слой
    stackedWidget = new QStackedWidget;
    mainLayout->addWidget(stackedWidget);

    // Создаем виджет первого окна и слой
    QWidget* firstWindow = new QWidget;
    QHBoxLayout* firstLayout = new QHBoxLayout(firstWindow);
    QPushButton* viewDbsBtn = new QPushButton("Перейти в редактор", this);
    firstLayout->addWidget(viewDbsBtn);
    connect(viewDbsBtn, &QPushButton::clicked, this, &MainWindow::onViewDbsBtnClicked);

    // 2-ое окно
    QWidget* secondWindow = new QWidget;
    secondLayout = new QVBoxLayout(secondWindow);
    QLabel* secondLayoutLabel = new QLabel("Доступные БД");
    secondLayout->addWidget(secondLayoutLabel);

    // 3-е окно с таблицами при выборе бд
    QWidget* tablesWindow = new QWidget;
    tablesLayout = new QVBoxLayout(tablesWindow);
    QLabel* tablesLayoutLabel = new QLabel("Таблицы");
    tablesLayout->addWidget(tablesLayoutLabel);

    // 4-ое окно с отображением редактируемой таблицы
    QWidget* tableDisplayWindow = new QWidget;
    tableDisplayLayout = new QVBoxLayout(tableDisplayWindow);
    QPushButton* saveChanges = new QPushButton("Сохранить изменения");
    QPushButton* addRow = new QPushButton("Добавить запись");
    QPushButton* deleteRow = new QPushButton("Удалить запись");
    tableView = new QTableView;
    tableDisplayLayout->addWidget(tableView);
    tableDisplayLayout->addWidget(saveChanges);
    tableDisplayLayout->addWidget(addRow);
    tableDisplayLayout->addWidget(deleteRow);
    connect(saveChanges, &QPushButton::clicked, this, &MainWindow::onSaveChangesBtnClicked);
    connect(addRow, &QPushButton::clicked, this, &MainWindow::onAddRowBtnClicked);
    connect(deleteRow, &QPushButton::clicked, this, &MainWindow::onDeleteRowBtnClicked);

    // Добавляем в stackedWidget все виджеты (окна)
    stackedWidget->addWidget(firstWindow);
    stackedWidget->addWidget(secondWindow);
    stackedWidget->addWidget(tablesWindow);
    stackedWidget->addWidget(tableDisplayWindow);
    // Устанавливаем mainWidget как центральный виджет
    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow() {}

void MainWindow::onViewDbsBtnClicked()
{
    // Очищаем вектор имен
    dbsNames.clear();
    // Очищаем все виджеты кроме текст лейбла
    while (secondLayout->count() > 1) {
        QWidget* widget = secondLayout->itemAt(1)->widget(); // получаем виджет
        secondLayout->removeWidget(widget); // удаляем полученный виджет из слоя
        delete widget; // удаляем сам виджет
    }

    db.open(); // Открываем соединение с бд

    // Подготавливаем запрос
    QSqlQuery query(db);
    query.prepare("SELECT datname FROM pg_database WHERE datistemplate = false;");

    if (!query.exec()) {
        qDebug() << "Ошибка:" << query.lastError().text();
        return;
    }

    // Вытаскиваем в строку имя и пушим в вектор QString
    while(query.next()){
        QString dbName = query.value(0).toString();
        if (dbName != "postgres"){
            dbsNames.push_back(dbName);
        };
    }

    // Динамически создаем кнопки с именами бд и добавляем на слой
    for (const QString& dbName : dbsNames) {
        QPushButton* dbNameBtn = new QPushButton(dbName, this);
        secondLayout->addWidget(dbNameBtn);
        connect(dbNameBtn, &QPushButton::clicked, this, &MainWindow::onDbBtnClicked);
    }

    stackedWidget->setCurrentIndex(1); // Переходим на второе окно
}

void MainWindow::onDbBtnClicked()
{
    QPushButton* pushedBtn = qobject_cast<QPushButton*>(sender());
    if (pushedBtn) {
        QString parsedDbName = pushedBtn->text();
        // Устанавливаем имя базы данных
        db.setDatabaseName(parsedDbName);

        if (db.open()) {
            qDebug() << "Successfully connected to database:" << parsedDbName;
            // Запрос на получение всех таблиц в выбранной бд
            QSqlQuery query(db);
            query.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public'");

            while (query.next()){
                // Вытаскиваем в строку и пушим в вектор (член класса)
                QString tableName = query.value(0).toString();
                tableNames.push_back(tableName);
            }

            for (const QString& tableName : tableNames) {
                QPushButton* tableNameBtn = new QPushButton(tableName, this);
                tablesLayout->addWidget(tableNameBtn);
                connect(tableNameBtn, &QPushButton::clicked, this, &MainWindow::onTableBtnClicked);
            }

            for (const QString& tableName : tableNames) {
                qDebug() << tableName << " ";
            }

            stackedWidget->setCurrentIndex(2); // Переходим на второе окно

        } else {
            qDebug() << "Failed to connect to database:" << db.lastError().text();
        }
    } else {
        qDebug() << "Failed";
    }
}


void MainWindow::onTableBtnClicked()
{
    // Метод sender вернет слот отправивший сигнал, но в виде указателя типа QObject*, поэтому нужно привести тип к QPushButton
    QPushButton* pushedBtn = qobject_cast<QPushButton*>(sender());
    if (pushedBtn) {
        QString tableName = pushedBtn->text();

        // Удаляем предыдущую модель, если она существует
        delete model;

        model = new QSqlTableModel(this, db.database());
        model->setTable(tableName);
        model->select(); // Загружаем данные из таблицы

        // Устанавливаем сортировку по полю id
        model->setSort(0, Qt::AscendingOrder); // по возрастанию (от 0 до ..)
        model->select();

        tableView->setModel(model);
        tableView->setEditTriggers(QAbstractItemView::DoubleClicked); // редактирование по двойному клику

        stackedWidget->setCurrentIndex(3); // Переходим на страницу с QTableView
    }
}

void MainWindow::onSaveChangesBtnClicked()
{
    if (model) {
        if (model->submitAll()) {
            qDebug() << "Changes saved successfully.";
        } else {
            qDebug() << "Failed to save changes:" << model->lastError().text();
        }
    }
}

void MainWindow::onAddRowBtnClicked()
{
    if (model) {
        int row = model->rowCount();
        model->insertRow(row);
        tableView->selectRow(row);
        qDebug() << "Added new row at index:" << row;
    }
}

void MainWindow::onDeleteRowBtnClicked()
{
    if (model) {
        QModelIndexList selected = tableView->selectionModel()->selectedRows();
        for (int i = 0; i < selected.count(); ++i) {
            model->removeRow(selected.at(i).row());
        }
        if (model->submitAll()) {
            qDebug() << "Deleted selected rows.";
        } else {
            qDebug() << "Failed to delete rows:" << model->lastError().text();
        }
    }
}
