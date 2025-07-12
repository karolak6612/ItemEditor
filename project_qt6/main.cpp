#include <QApplication>
#include <QFile>
#include <QDebug>
#include "mainwindow.h"
#include "otb/item.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Load stylesheet
    QFile styleFile(":/dark.qss"); // Assuming it will be added to resources
    if (!styleFile.exists()) {
        // Fallback for when not in resources (e.g., during development)
        styleFile.setFileName("dark.qss");
    }

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        qDebug() << "Applied dark stylesheet.";
    } else {
        qWarning() << "Could not find stylesheet 'dark.qss'. Using default style.";
    }


    OTB::Sprite::createBlankSprite();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
