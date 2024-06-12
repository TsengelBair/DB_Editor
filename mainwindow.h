#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dbsettings.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTableView>
#include <QSqlTableModel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QSqlDatabase db; // Делаем бд членом класса для вз-я
    QStackedWidget* stackedWidget; // Для динамического переключения в методах
    // Второй слой с отображением бд
    QVBoxLayout* secondLayout;
    QVector<QString> dbsNames;
    // Третий слой с отображением таблиц в конкретной бд
    QVBoxLayout* tablesLayout;
    QVector<QString> tableNames;
    // Для таблицы в ui (4-ый слой)
    QVBoxLayout* tableDisplayLayout;
    QTableView* tableView;
    QSqlTableModel* model;

private slots:
    void onViewDbsBtnClicked();
    void onDbBtnClicked();
    void onTableBtnClicked();
    void onSaveChangesBtnClicked();
    void onAddRowBtnClicked();
    void onDeleteRowBtnClicked();
};
#endif // MAINWINDOW_H
