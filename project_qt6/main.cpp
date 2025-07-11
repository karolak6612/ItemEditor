#include <QApplication>
#include "mainwindow.h" // Include the new MainWindow header
#include "otb/item.h"   // For OTB::Sprite::createBlankSprite()

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    OTB::Sprite::createBlankSprite(); // Initialize blank sprite data

    MainWindow mainWindow; // Create an instance of our MainWindow
    mainWindow.show();

    return app.exec();
}
