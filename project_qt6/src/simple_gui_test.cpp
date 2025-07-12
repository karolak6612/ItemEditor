#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

class SimpleTestWindow : public QMainWindow
{
public:
    SimpleTestWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("Simple GUI Test - ItemEditor Qt6");
        setMinimumSize(400, 300);
        
        // Create central widget
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // Create layout
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
        
        // Add a label
        QLabel* label = new QLabel("GUI Test Window", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size: 18px; font-weight: bold; margin: 20px;");
        layout->addWidget(label);
        
        // Add a button
        QPushButton* button = new QPushButton("Click Me!", this);
        layout->addWidget(button);
        
        // Connect button
        connect(button, &QPushButton::clicked, [this]() {
            QMessageBox::information(this, "Test", "GUI is working correctly!");
        });
        
        layout->addStretch();
        
        qDebug() << "Simple test window created successfully";
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Starting simple GUI test...";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Application arguments:" << app.arguments();
    
    SimpleTestWindow window;
    window.show();
    window.raise();
    window.activateWindow();
    
    qDebug() << "Window shown, entering event loop...";
    
    return app.exec();
}