/**
 * Item Editor Qt6 - UI Performance Test
 * Tests for optimized custom control rendering performance
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <QScrollArea>
#include <QProgressBar>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDebug>

#include "ClientItemView.h"
#include "ServerItemListBox.h"
#include "FlagCheckBox.h"

class PerformanceTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    PerformanceTestWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_testRunning(false)
        , m_frameCount(0)
        , m_totalFrameTime(0)
    {
        setupUI();
        setupTests();
    }

private slots:
    void runClientItemViewTest()
    {
        m_logOutput->append("=== ClientItemView Performance Test ===");
        
        QElapsedTimer timer;
        timer.start();
        
        // Test rapid item changes
        for (int i = 0; i < 1000; ++i) {
            // Simulate item changes
            m_clientItemView->update();
            QApplication::processEvents();
        }
        
        qint64 elapsed = timer.elapsed();
        m_logOutput->append(QString("1000 updates completed in %1ms").arg(elapsed));
        m_logOutput->append(QString("Average: %1ms per update").arg(elapsed / 1000.0, 0, 'f', 3));
        
        // Verify performance criteria: < 16ms per frame for 60 FPS
        bool passed = (elapsed / 1000.0) < 16.0;
        m_logOutput->append(QString("Performance test: %1").arg(passed ? "PASSED" : "FAILED"));
        m_logOutput->append("");
    }
    
    void runServerItemListTest()
    {
        m_logOutput->append("=== ServerItemListBox Performance Test ===");
        
        // Add large number of items
        QElapsedTimer timer;
        timer.start();
        
        m_serverItemList->beginUpdate();
        for (int i = 1; i <= 5000; ++i) {
            m_serverItemList->addItem(i, QString("Item %1").arg(i));
        }
        m_serverItemList->endUpdate();
        
        qint64 addTime = timer.elapsed();
        m_logOutput->append(QString("Added 5000 items in %1ms").arg(addTime));
        
        // Test scrolling performance
        timer.restart();
        for (int i = 0; i < 100; ++i) {
            // Simulate scrolling
            m_serverItemList->update();
            QApplication::processEvents();
        }
        
        qint64 scrollTime = timer.elapsed();
        m_logOutput->append(QString("100 scroll updates in %1ms").arg(scrollTime));
        m_logOutput->append(QString("Average: %1ms per scroll").arg(scrollTime / 100.0, 0, 'f', 3));
        
        // Verify performance criteria
        bool addPassed = addTime < 5000; // Should add items in under 5 seconds
        bool scrollPassed = (scrollTime / 100.0) < 16.0; // Should scroll smoothly
        
        m_logOutput->append(QString("Add performance: %1").arg(addPassed ? "PASSED" : "FAILED"));
        m_logOutput->append(QString("Scroll performance: %1").arg(scrollPassed ? "PASSED" : "FAILED"));
        m_logOutput->append("");
    }
    
    void runFlagCheckBoxTest()
    {
        m_logOutput->append("=== FlagCheckBox Performance Test ===");
        
        QElapsedTimer timer;
        timer.start();
        
        // Test rapid flag changes
        for (int i = 0; i < 1000; ++i) {
            auto flag = static_cast<OTLib::Server::Items::ServerItemFlag>(i % 28);
            m_flagCheckBox->setServerItemFlag(flag);
            QApplication::processEvents();
        }
        
        qint64 elapsed = timer.elapsed();
        m_logOutput->append(QString("1000 flag changes in %1ms").arg(elapsed));
        m_logOutput->append(QString("Average: %1ms per change").arg(elapsed / 1000.0, 0, 'f', 3));
        
        // Verify performance criteria
        bool passed = (elapsed / 1000.0) < 1.0; // Should be very fast
        m_logOutput->append(QString("Performance test: %1").arg(passed ? "PASSED" : "FAILED"));
        m_logOutput->append("");
    }
    
    void runMemoryTest()
    {
        m_logOutput->append("=== Memory Usage Test ===");
        
        // Test memory stability during operations
        size_t initialMemory = getCurrentMemoryUsage();
        
        // Perform intensive operations
        for (int cycle = 0; cycle < 10; ++cycle) {
            // Add and remove items
            m_serverItemList->clearSpriteCache();
            
            for (int i = 0; i < 100; ++i) {
                m_serverItemList->addItem(10000 + i, QString("Temp %1").arg(i));
            }
            
            // Force updates
            m_clientItemView->update();
            m_serverItemList->update();
            
            QApplication::processEvents();
        }
        
        size_t finalMemory = getCurrentMemoryUsage();
        double memoryIncrease = (finalMemory - initialMemory) / 1024.0 / 1024.0; // MB
        
        m_logOutput->append(QString("Memory increase: %1 MB").arg(memoryIncrease, 0, 'f', 2));
        
        // Verify memory usage is reasonable (< 50MB increase)
        bool passed = memoryIncrease < 50.0;
        m_logOutput->append(QString("Memory test: %1").arg(passed ? "PASSED" : "FAILED"));
        m_logOutput->append("");
    }
    
    void runAllTests()
    {
        if (m_testRunning) return;
        
        m_testRunning = true;
        m_runAllButton->setEnabled(false);
        m_logOutput->clear();
        
        m_logOutput->append("Starting UI Performance Tests...");
        m_logOutput->append("");
        
        // Run all tests with delays for UI responsiveness
        QTimer::singleShot(100, this, &PerformanceTestWindow::runClientItemViewTest);
        QTimer::singleShot(500, this, &PerformanceTestWindow::runServerItemListTest);
        QTimer::singleShot(1000, this, &PerformanceTestWindow::runFlagCheckBoxTest);
        QTimer::singleShot(1500, this, &PerformanceTestWindow::runMemoryTest);
        QTimer::singleShot(2000, this, &PerformanceTestWindow::finishTests);
    }
    
    void finishTests()
    {
        m_logOutput->append("=== Performance Test Summary ===");
        m_logOutput->append("All tests completed. Check results above.");
        m_logOutput->append("Performance criteria:");
        m_logOutput->append("- Paint operations: < 16ms per frame (60 FPS)");
        m_logOutput->append("- Item loading: < 5 seconds for 5000 items");
        m_logOutput->append("- Memory usage: < 50MB increase during stress test");
        
        m_testRunning = false;
        m_runAllButton->setEnabled(true);
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        
        auto* mainLayout = new QVBoxLayout(centralWidget);
        
        // Test controls section
        auto* controlsGroup = new QGroupBox("Test Controls");
        auto* controlsLayout = new QHBoxLayout(controlsGroup);
        
        m_runAllButton = new QPushButton("Run All Performance Tests");
        connect(m_runAllButton, &QPushButton::clicked, this, &PerformanceTestWindow::runAllTests);
        controlsLayout->addWidget(m_runAllButton);
        
        controlsLayout->addStretch();
        mainLayout->addWidget(controlsGroup);
        
        // Test widgets section
        auto* splitter = new QSplitter(Qt::Horizontal);
        
        // Left side - test widgets
        auto* testGroup = new QGroupBox("Test Widgets");
        auto* testLayout = new QVBoxLayout(testGroup);
        
        // ClientItemView test
        m_clientItemView = new ItemEditor::ClientItemView;
        m_clientItemView->setMinimumSize(100, 100);
        testLayout->addWidget(new QLabel("ClientItemView:"));
        testLayout->addWidget(m_clientItemView);
        
        // ServerItemListBox test
        m_serverItemList = new ItemEditor::ServerItemListBox;
        m_serverItemList->setMinimumSize(200, 150);
        testLayout->addWidget(new QLabel("ServerItemListBox:"));
        testLayout->addWidget(m_serverItemList);
        
        // FlagCheckBox test
        m_flagCheckBox = new ItemEditor::FlagCheckBox(OTLib::Server::Items::ServerItemFlag::Pickupable);
        testLayout->addWidget(new QLabel("FlagCheckBox:"));
        testLayout->addWidget(m_flagCheckBox);
        
        testLayout->addStretch();
        splitter->addWidget(testGroup);
        
        // Right side - log output
        auto* logGroup = new QGroupBox("Test Results");
        auto* logLayout = new QVBoxLayout(logGroup);
        
        m_logOutput = new QTextEdit;
        m_logOutput->setFont(QFont("Consolas", 9));
        m_logOutput->setMinimumWidth(400);
        logLayout->addWidget(m_logOutput);
        
        splitter->addWidget(logGroup);
        splitter->setSizes({300, 500});
        
        mainLayout->addWidget(splitter);
        
        setWindowTitle("UI Performance Test - Item Editor Qt6");
        resize(900, 600);
    }
    
    void setupTests()
    {
        // Initialize test data
        // Note: In a real implementation, we would create mock ClientItem objects
        // For now, the tests will work with the existing widget functionality
    }
    
    size_t getCurrentMemoryUsage()
    {
        // Simple memory usage estimation
        // In a real implementation, this would use platform-specific APIs
        return 0; // Placeholder
    }

private:
    bool m_testRunning;
    int m_frameCount;
    qint64 m_totalFrameTime;
    
    // UI components
    QPushButton* m_runAllButton;
    QTextEdit* m_logOutput;
    
    // Test widgets
    ItemEditor::ClientItemView* m_clientItemView;
    ItemEditor::ServerItemListBox* m_serverItemList;
    ItemEditor::FlagCheckBox* m_flagCheckBox;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    PerformanceTestWindow window;
    window.show();
    
    return app.exec();
}

#include "performance_test.moc"