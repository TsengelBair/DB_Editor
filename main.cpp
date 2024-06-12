#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(900, 650);
    w.setWindowTitle("DB Manager");
    w.show();
    return a.exec();
}
