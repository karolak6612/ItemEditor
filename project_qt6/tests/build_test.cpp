#include <QCoreApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "ItemEditor Qt6 Build Test - SUCCESS!";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Application Name:" << app.applicationName();
    
    std::cout << "Console output: Build test completed successfully!" << std::endl;
    
    return 0;
}