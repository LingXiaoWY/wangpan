#include "mainwindow.h"
#include <QApplication>
#include "ckernel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CKernel::getInstance();
    return a.exec();
}
