#include <QApplication>
#include "mainwindow.h" // Include the new MainWindow header

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow; // Create an instance of our MainWindow
    mainWindow.show();

    return app.exec();
}
