#include "supervisorio.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
      //  QApplication::setGraphicsSystem("raster");
    //#endif
    //QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    supervisorio w;
    //Controle c;
    w.show();


    return a.exec();
}


