#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowFlags(w.windowFlags() &~ Qt::WindowMaximizeButtonHint);
    w.show();

    printf("*****Compile time:%s, %s*****\n", __DATE__, __TIME__);
    fflush(stdout);

    return a.exec();
}
