/**
 * Item Editor Qt6 - Main Application Entry Point
 * Exact mirror of Legacy_App/csharp/Source/Program.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLoggingCategory>

#include "MainForm.h"
#include "Properties/version.h"

// Logging categories
Q_LOGGING_CATEGORY(itemEditor, "itemeditor")
Q_LOGGING_CATEGORY(plugins, "itemeditor.plugins")

/**
 * Global plugin services instance
 * Plugin services are now managed by MainForm
 */
namespace ItemEditor {
    // Plugin services managed by MainForm
}

/**
 * Initialize application settings and directories
 */
void initializeApplication()
{
    // Set application properties
    QApplication::setApplicationName("Item Editor");
    QApplication::setApplicationVersion(ItemEditor::Version::getVersionString());
    QApplication::setOrganizationName("OTTools");
    QApplication::setOrganizationDomain("ottools.org");
    
    // Set application icon
    QApplication::setWindowIcon(QIcon(":/icons/application.ico"));
    
    // Enable high DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

/**
 * Setup application style and theme
 */
void setupApplicationStyle()
{
    // Try to use a modern style
    QStringList preferredStyles = {"Fusion", "Windows", "WindowsVista"};
    
    for (const QString& styleName : preferredStyles) {
        if (QStyleFactory::keys().contains(styleName, Qt::CaseInsensitive)) {
            QApplication::setStyle(styleName);
            break;
        }
    }
    
    // Apply dark theme similar to DarkUI from C# version
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    QApplication::setPalette(darkPalette);
}

/**
 * Setup internationalization
 */
void setupTranslations(QApplication& app)
{
    // Load Qt translations
    QTranslator* qtTranslator = new QTranslator(&app);
    if (qtTranslator->load("qt_" + QLocale::system().name(),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(qtTranslator);
    }
    
    // Load application translations
    QTranslator* appTranslator = new QTranslator(&app);
    if (appTranslator->load("itemeditor_" + QLocale::system().name(),
                           ":/translations")) {
        app.installTranslator(appTranslator);
    }
}

/**
 * Initialize plugin system
 * Plugin initialization is now handled by MainForm
 */
bool initializePlugins()
{
    qCInfo(plugins) << "Plugin system will be initialized by MainForm";
    return true;
}

/**
 * Cleanup resources on application exit
 * Plugin cleanup is now handled by MainForm
 */
void cleanupApplication()
{
    qCInfo(itemEditor) << "Application cleanup completed";
}

/**
 * Main application entry point
 * Equivalent to C# Program.Main() method
 */
int main(int argc, char *argv[])
{
    // Initialize application
    initializeApplication();
    
    // Create QApplication instance
    // Equivalent to C#: Application.EnableVisualStyles() and Application.SetCompatibleTextRenderingDefault(false)
    QApplication app(argc, argv);
    
    // Setup application style and theme
    setupApplicationStyle();
    
    // Setup translations
    setupTranslations(app);
    
    // Initialize plugin system
    if (!initializePlugins()) {
        qCCritical(itemEditor) << "Failed to initialize plugin system. Exiting.";
        return -1;
    }
    
    // Create and show main window
    // Equivalent to C#: Application.Run(new MainForm())
    ItemEditor::MainForm mainWindow;
    mainWindow.show();
    
    qCInfo(itemEditor) << "Item Editor started successfully";
    
    // Setup cleanup on application exit
    QObject::connect(&app, &QApplication::aboutToQuit, cleanupApplication);
    
    // Start event loop
    int result = app.exec();
    
    qCInfo(itemEditor) << "Application exiting with code:" << result;
    
    return result;
}