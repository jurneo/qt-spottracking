#include <QApplication>
#include "mainwindow.h"
#include <boost/gil/gil_all.hpp>
//#include <boost/gil/extension/io/png_dynamic_io.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
