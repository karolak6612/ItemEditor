#pragma once

#include <QDialog>
#include <QListWidgetItem>
#include "../ItemEditor.Core/ServerItemList.h"
#include "../ItemEditor.Core/ServerItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FindItemDialog; }
QT_END_NAMESPACE

/**
 * @brief Dialog for finding items with identical search functionality to legacy system
 * 
 * Provides comprehensive search capabilities including:
 * - Search by ID, Name, Type, or Client ID
 * - Exact match and case-sensitive options
 * - Search result navigation and highlighting
 * - Real-time search as user types
 */
class FindItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindItemDialog(const ServerItemList* itemList, QWidget *parent = nullptr);
    ~FindItemDialog();

    // Get the selected item from search results
    ServerItem getSelectedItem() const;
    bool hasSelectedItem() const;
    
    // Set initial search parameters
    void setSearchById(ItemId id);
    void setSearchByName(const QString& name);
    void setSearchByType(ServerItemType type);
    void setSearchByClientId(ClientId clientId);

signals:
    void itemSelected(const ServerItem& item);
    void itemHighlighted(const ServerItem& item);

private slots:
    void onSearchButtonClicked();
    void onClearButtonClicked();
    void onSearchTermChanged();
    void onSearchByChanged();
    void onResultItemClicked(QListWidgetItem* item);
    void onResultItemDoubleClicked(QListWidgetItem* item);
    void onPreviousButtonClicked();
    void onNextButtonClicked();

private:
    enum class SearchBy {
        Id,
        Name,
        Type,
        ClientId
    };

    Ui::FindItemDialog *ui;
    const ServerItemList* m_itemList;
    QList<ServerItem> m_searchResults;
    int m_currentResultIndex;
    ServerItem m_selectedItem;
    bool m_hasSelectedItem;

    // Search functionality
    void performSearch();
    void clearResults();
    void updateResultsList();
    void updateNavigationButtons();
    void updateResultCount();
    
    // Search algorithms matching legacy system behavior
    QList<ServerItem> searchById(const QString& searchTerm, bool exactMatch) const;
    QList<ServerItem> searchByName(const QString& searchTerm, bool exactMatch, bool caseSensitive) const;
    QList<ServerItem> searchByType(const QString& searchTerm, bool exactMatch) const;
    QList<ServerItem> searchByClientId(const QString& searchTerm, bool exactMatch) const;
    
    // Helper methods
    SearchBy getCurrentSearchBy() const;
    QString formatItemForDisplay(const ServerItem& item) const;
    void selectResult(int index);
    void highlightResult(int index);
    
    // UI initialization
    void setupConnections();
    void setupUI();
    void applyDarkTheme();
};