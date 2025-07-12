#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/resourcemanager.h"
#include "core/stylesheetmanager.h"

namespace ItemEditor {
namespace Core {

/**
 * @brief Convenience class for easy resource access
 * 
 * This class provides static methods for quick access to commonly used
 * resources without needing to directly interact with managers.
 */
class Resources
{
public:
    // Icon access shortcuts
    static QIcon newIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::NewIcon); }
    static QIcon openIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::OpenIcon); }
    static QIcon saveIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::SaveIcon); }
    static QIcon saveAsIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::SaveAsIcon); }
    static QIcon reloadIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::ReloadIcon); }
    static QIcon duplicateIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::DuplicateIcon); }
    static QIcon findIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::FindIcon); }
    static QIcon infoIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::InfoIcon); }
    static QIcon formIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::FormIcon); }
    static QIcon helpIcon() { return ResourceManager::instance().getIcon(ResourceManager::IconId::Help); }

    // Pixmap access shortcuts
    static QPixmap aboutBackground() { return ResourceManager::instance().getPixmap(ResourceManager::IconId::AboutBackground); }
    static QPixmap applicationForm() { return ResourceManager::instance().getPixmap(ResourceManager::IconId::ApplicationForm); }

    // Theme management shortcuts
    static bool applyDarkTheme() { return StylesheetManager::instance().applyTheme(StylesheetManager::Theme::Dark); }
    static bool applyLightTheme() { return StylesheetManager::instance().applyTheme(StylesheetManager::Theme::Light); }
    static QString getCurrentThemeName() { 
        return StylesheetManager::instance().getThemeName(StylesheetManager::instance().getCurrentTheme()); 
    }

    // Generic access methods
    static QIcon getIcon(ResourceManager::IconId iconId) { 
        return ResourceManager::instance().getIcon(iconId); 
    }
    
    static QIcon getIcon(const QString& resourcePath) { 
        return ResourceManager::instance().getIcon(resourcePath); 
    }
    
    static QPixmap getPixmap(ResourceManager::IconId iconId) { 
        return ResourceManager::instance().getPixmap(iconId); 
    }
    
    static QPixmap getPixmap(const QString& resourcePath) { 
        return ResourceManager::instance().getPixmap(resourcePath); 
    }
    
    static QString getStylesheet(const QString& stylesheetName) { 
        return ResourceManager::instance().getStylesheet(stylesheetName); 
    }

private:
    Resources() = delete; // Static class
};

} // namespace Core
} // namespace ItemEditor

#endif // RESOURCES_H