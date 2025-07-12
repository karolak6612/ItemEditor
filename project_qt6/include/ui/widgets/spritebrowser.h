#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QGroupBox>
#include <QListWidget>
#include <QSplitter>
#include "otb/item.h"
#include "ui/widgets/clientitemview.h"

namespace UI {
namespace Widgets {

/**
 * @brief Comprehensive sprite browser and management widget
 * 
 * Provides visual browsing of all available sprites with thumbnails,
 * search/filter capabilities, and sprite assignment tools.
 * Based on C# sprite management functionality.
 */
class SpriteBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit SpriteBrowser(QWidget *parent = nullptr);
    virtual ~SpriteBrowser();

    // Event handling
    bool eventFilter(QObject *obj, QEvent *event) override;

    // Main interface
    void setClientItems(const QMap<quint16, ItemEditor::ClientItem>& items);
    void setCurrentItem(const ItemEditor::ClientItem* item);
    
    // Sprite operations
    void showSpriteCandidates(const ItemEditor::ClientItem* sourceItem);
    void assignSpriteToItem(quint32 spriteId, ItemEditor::ClientItem* targetItem);
    
    // Search and filtering
    void searchSprites(const QString& criteria);
    void filterByProperties();
    void clearFilters();

signals:
    void spriteSelected(quint32 spriteId);
    void spriteDoubleClicked(quint32 spriteId);
    void spriteAssignmentRequested(quint32 spriteId, ItemEditor::ClientItem* item);
    void candidateSelected(const ItemEditor::ClientItem* item);

private slots:
    void onSearchTextChanged();
    void onFilterChanged();
    void onZoomChanged(int value);
    void onViewModeChanged();
    void onSpriteClicked();
    void onSpriteDoubleClicked();
    void onShowCandidatesClicked();
    void onAssignSpriteClicked();
    void onAnalyzeSimilarityClicked();

private:
    void setupUI();
    void setupConnections();
    void populateSpriteGrid();
    void updateSpriteDisplay();
    void calculateSpriteSimilarity();
    void showSpriteDetails(quint32 spriteId);
    void updateCandidatesList(const QList<ItemEditor::ClientItem*>& candidates);
    
    // UI Components
    QSplitter* m_mainSplitter;
    
    // Control panel
    QGroupBox* m_controlsGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_filterTypeCombo;
    QComboBox* m_sortOrderCombo;
    QSlider* m_zoomSlider;
    QLabel* m_zoomLabel;
    QCheckBox* m_showOnlyUsedCheck;
    QCheckBox* m_showAnimationsCheck;
    QPushButton* m_showCandidatesButton;
    QPushButton* m_analyzeSimilarityButton;
    
    // Sprite grid area
    QScrollArea* m_spriteScrollArea;
    QWidget* m_spriteGridWidget;
    QGridLayout* m_spriteGridLayout;
    
    // Details panel
    QGroupBox* m_detailsGroup;
    ClientItemView* m_selectedSpriteView;
    QLabel* m_spriteIdLabel;
    QLabel* m_spriteSizeLabel;
    QLabel* m_spriteUsageLabel;
    QListWidget* m_usedByList;
    
    // Candidates panel
    QGroupBox* m_candidatesGroup;
    QListWidget* m_candidatesList;
    QLabel* m_candidatesCountLabel;
    QPushButton* m_assignSpriteButton;
    
    // Progress
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    // Data
    QMap<quint16, ItemEditor::ClientItem> m_clientItems;
    const ItemEditor::ClientItem* m_currentItem;
    QList<quint32> m_filteredSpriteIds;
    QList<ItemEditor::ClientItem*> m_spriteCandidates;
    quint32 m_selectedSpriteId;
    int m_zoomLevel;
    int m_spritesPerRow;
    
    // Sprite grid items
    QList<ClientItemView*> m_spriteViews;
};

} // namespace Widgets
} // namespace UI