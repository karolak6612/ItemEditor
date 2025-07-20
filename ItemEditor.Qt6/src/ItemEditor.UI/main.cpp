#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QDebug>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("ItemEditor");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("ItemEditor Qt6");
    app.setOrganizationName("ItemEditor");
    app.setOrganizationDomain("itemeditor.com");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/application.ico"));
    
    // Configure for Windows x86 optimization
#ifdef Q_OS_WIN
    // Enable high-DPI support
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Set Windows-specific styling
    app.setStyle(QStyleFactory::create("WindowsVista"));
#endif
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}