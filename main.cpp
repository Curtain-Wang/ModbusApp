#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QApplication::setStyle(QStyleFactory::create("Fusion")); // 或 "Windows"，避免原生 macOS 样式
    w.show();
    return a.exec();
}
