#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QDebug>
#include "ui/mainwindow.h"
#include "core/application.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("ItemEditor Qt6");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("ItemEditor");
    app.setOrganizationDomain("itemeditor.org");
    
    qDebug() << "Starting ItemEditor Qt6 Application";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Application directory:" << QDir::currentPath();
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/application_form.png"));
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    qDebug() << "MainWindow created and shown";
    
    return app.exec();
}