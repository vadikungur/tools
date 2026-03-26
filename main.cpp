#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QCoreApplication::setOrganizationName("COMLogger");
    QCoreApplication::setApplicationName("COMPortLogger");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
