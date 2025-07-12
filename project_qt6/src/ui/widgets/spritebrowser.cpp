#include "ui/widgets/spritebrowser.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QScrollBar>

namespace UI {
namespace Widgets {

SpriteBrowser::SpriteBrowser(QWidget *parent)
    : QWidget(parent)
    , m_currentItem(nullptr)
    , m_selectedSpriteId(0)
    , m_zoomLevel(32)
    , m_spritesPerRow(10)
{
    setupUI();
    setupConnections();
}

SpriteBrowser::~SpriteBrowser()
{
    // Qt handles cleanup automatically
}

void SpriteBrowser::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Main splitter for resizable panels
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Left panel - Controls and sprite grid
    auto* leftWidget = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftWidget);
    
    // Controls group
    m_controlsGroup = new QGroupBox("Sprite Browser Controls", this);
    auto* controlsLayout = new QGridLayout(m_controlsGroup);
    
    // Search
    controlsLayout->addWidget(new QLabel("Search:", this), 0, 0);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search sprites by ID, usage, or properties...");
    controlsLayout->addWidget(m_searchEdit, 0, 1, 1, 2);
    
    // Filter controls
    controlsLayout->addWidget(new QLabel("Filter:", this), 1, 0);
    m_filterTypeCombo = new QComboBox(this);
    m_filterTypeCombo->addItems({"All Sprites", "Used Sprites", "Unused Sprites", "Animated Sprites", "Static Sprites"});
    controlsLayout->addWidget(m_filterTypeCombo, 1, 1);
    
    controlsLayout->addWidget(new QLabel("Sort:", this), 1, 2);
    m_sortOrderCombo = new QComboBox(this);
    m_sortOrderCombo->addItems({"By ID", "By Usage Count", "By Size", "By Similarity"});
    controlsLayout->addWidget(m_sortOrderCombo, 1, 3);
    
    // Zoom controls
    controlsLayout->addWidget(new QLabel("Zoom:", this), 2, 0);
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(16, 64);
    m_zoomSlider->setValue(32);
    controlsLayout->addWidget(m_zoomSlider, 2, 1);
    
    m_zoomLabel = new QLabel("32px", this);
    controlsLayout->addWidget(m_zoomLabel, 2, 2);
    
    // Options
    m_showOnlyUsedCheck = new QCheckBox("Show only used sprites", this);
    controlsLayout->addWidget(m_showOnlyUsedCheck, 3, 0, 1, 2);
    
    m_showAnimationsCheck = new QCheckBox("Show animations", this);
    controlsLayout->addWidget(m_showAnimationsCheck, 3, 2, 1, 2);
    
    // Action buttons
    m_showCandidatesButton = new QPushButton("Show Candidates", this);
    m_showCandidatesButton->setEnabled(false);
    controlsLayout->addWidget(m_showCandidatesButton, 4, 0);
    
    m_analyzeSimilarityButton = new QPushButton("Analyze Similarity", this);
    controlsLayout->addWidget(m_analyzeSimilarityButton, 4, 1);
    
    leftLayout->addWidget(m_controlsGroup);
    
    // Sprite grid scroll area
    m_spriteScrollArea = new QScrollArea(this);
    m_spriteScrollArea->setWidgetResizable(true);
    m_spriteScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_spriteScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_spriteGridWidget = new QWidget();
    m_spriteGridLayout = new QGridLayout(m_spriteGridWidget);
    m_spriteGridLayout->setSpacing(2);
    m_spriteScrollArea->setWidget(m_spriteGridWidget);
    
    leftLayout->addWidget(m_spriteScrollArea);
    
    // Progress and status
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    leftLayout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel("Ready", this);
    leftLayout->addWidget(m_statusLabel);
    
    m_mainSplitter->addWidget(leftWidget);
    
    // Right panel - Details and candidates
    auto* rightWidget = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightWidget);
    
    // Sprite details group
    m_detailsGroup = new QGroupBox("Sprite Details", this);
    auto* detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    // Selected sprite preview
    m_selectedSpriteView = new ClientItemView(this);
    m_selectedSpriteView->setMinimumSize(64, 64);
    m_selectedSpriteView->setMaximumSize(128, 128);
    detailsLayout->addWidget(m_selectedSpriteView);
    
    // Sprite information
    m_spriteIdLabel = new QLabel("No sprite selected", this);
    detailsLayout->addWidget(m_spriteIdLabel);
    
    m_spriteSizeLabel = new QLabel("", this);
    detailsLayout->addWidget(m_spriteSizeLabel);
    
    m_spriteUsageLabel = new QLabel("", this);
    detailsLayout->addWidget(m_spriteUsageLabel);
    
    // Used by list
    detailsLayout->addWidget(new QLabel("Used by items:", this));
    m_usedByList = new QListWidget(this);
    m_usedByList->setMaximumHeight(150);
    detailsLayout->addWidget(m_usedByList);
    
    rightLayout->addWidget(m_detailsGroup);
    
    // Candidates group
    m_candidatesGroup = new QGroupBox("Sprite Candidates", this);
    auto* candidatesLayout = new QVBoxLayout(m_candidatesGroup);
    
    m_candidatesCountLabel = new QLabel("0 candidates", this);
    candidatesLayout->addWidget(m_candidatesCountLabel);
    
    m_candidatesList = new QListWidget(this);
    candidatesLayout->addWidget(m_candidatesList);
    
    m_assignSpriteButton = new QPushButton("Assign Selected Sprite", this);
    m_assignSpriteButton->setEnabled(false);
    candidatesLayout->addWidget(m_assignSpriteButton);
    
    rightLayout->addWidget(m_candidatesGroup);
    
    m_mainSplitter->addWidget(rightWidget);
    
    // Set splitter proportions (left panel larger)
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(m_mainSplitter);
}

void SpriteBrowser::setupConnections()
{
    // Search and filter controls
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SpriteBrowser::onSearchTextChanged);
    connect(m_filterTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpriteBrowser::onFilterChanged);
    connect(m_sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpriteBrowser::onFilterChanged);
    connect(m_showOnlyUsedCheck, &QCheckBox::toggled, this, &SpriteBrowser::onFilterChanged);
    connect(m_showAnimationsCheck, &QCheckBox::toggled, this, &SpriteBrowser::onFilterChanged);
    
    // Zoom control
    connect(m_zoomSlider, &QSlider::valueChanged, this, &SpriteBrowser::onZoomChanged);
    
    // Action buttons
    connect(m_showCandidatesButton, &QPushButton::clicked, this, &SpriteBrowser::onShowCandidatesClicked);
    connect(m_analyzeSimilarityButton, &QPushButton::clicked, this, &SpriteBrowser::onAnalyzeSimilarityClicked);
    connect(m_assignSpriteButton, &QPushButton::clicked, this, &SpriteBrowser::onAssignSpriteClicked);
    
    // Candidates list
    connect(m_candidatesList, &QListWidget::itemSelectionChanged, [this]() {
        bool hasSelection = !m_candidatesList->selectedItems().isEmpty();
        m_assignSpriteButton->setEnabled(hasSelection && m_selectedSpriteId > 0);
    });
}

void SpriteBrowser::setClientItems(const QMap<quint16, ItemEditor::ClientItem>& items)
{
    m_clientItems = items;
    populateSpriteGrid();
}

void SpriteBrowser::setCurrentItem(const ItemEditor::ClientItem* item)
{
    m_currentItem = item;
    m_showCandidatesButton->setEnabled(item != nullptr);
    
    if (item) {
        // Auto-show candidates for the current item
        showSpriteCandidates(item);
    }
}

void SpriteBrowser::populateSpriteGrid()
{
    // Clear existing sprite views
    for (auto* view : m_spriteViews) {
        view->deleteLater();
    }
    m_spriteViews.clear();
    
    // Clear layout
    QLayoutItem* item;
    while ((item = m_spriteGridLayout->takeAt(0)) != nullptr) {
        delete item;
    }
    
    m_statusLabel->setText("Loading sprites...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, m_clientItems.size());
    m_progressBar->setValue(0);
    
    // Populate with client items (sprites)
    int row = 0, col = 0;
    int processed = 0;
    
    for (auto it = m_clientItems.begin(); it != m_clientItems.end(); ++it) {
        const auto& clientItem = it.value();
        
        // Apply filters
        if (m_showOnlyUsedCheck->isChecked()) {
            // Check if sprite is used by any server items
            bool isUsed = false;
            // TODO: Check against server items
            if (!isUsed) continue;
        }
        
        // Create sprite view
        auto* spriteView = new ClientItemView(this);
        spriteView->setClientItem(&clientItem);
        spriteView->setFixedSize(m_zoomLevel, m_zoomLevel);
        spriteView->setToolTip(QString("ID: %1\nClient ID: %2\nName: %3")
                              .arg(clientItem.ID)
                              .arg(clientItem.ID) // Client items use ID field
                              .arg(clientItem.Name.isEmpty() ? "Unnamed" : clientItem.Name));
        
        // Add click handling
        spriteView->installEventFilter(this);
        spriteView->setProperty("spriteId", clientItem.ID);
        
        m_spriteGridLayout->addWidget(spriteView, row, col);
        m_spriteViews.append(spriteView);
        
        col++;
        if (col >= m_spritesPerRow) {
            col = 0;
            row++;
        }
        
        processed++;
        m_progressBar->setValue(processed);
        
        // Process events periodically to keep UI responsive
        if (processed % 50 == 0) {
            QApplication::processEvents();
        }
    }
    
    m_progressBar->setVisible(false);
    m_statusLabel->setText(QString("Loaded %1 sprites").arg(m_spriteViews.size()));
}

void SpriteBrowser::showSpriteCandidates(const ItemEditor::ClientItem* sourceItem)
{
    if (!sourceItem) return;
    
    m_statusLabel->setText("Analyzing sprite candidates...");
    m_progressBar->setVisible(true);
    
    // Find similar sprites using signature comparison
    QList<ItemEditor::ClientItem*> candidates;
    
    // TODO: Implement sprite signature comparison
    // This would use the FFT-based similarity from the C# version
    
    // For now, add some placeholder candidates
    int count = 0;
    for (auto it = m_clientItems.begin(); it != m_clientItems.end() && count < 5; ++it) {
        if (it.value().ID != sourceItem->ID) {
            candidates.append(const_cast<ItemEditor::ClientItem*>(&it.value()));
            count++;
        }
    }
    
    updateCandidatesList(candidates);
    
    m_progressBar->setVisible(false);
    m_statusLabel->setText(QString("Found %1 candidates").arg(candidates.size()));
}

void SpriteBrowser::updateCandidatesList(const QList<ItemEditor::ClientItem*>& candidates)
{
    m_spriteCandidates = candidates;
    m_candidatesList->clear();
    
    for (const auto* candidate : candidates) {
        QString displayText = QString("ID: %1 | %2")
            .arg(candidate->ID)
            .arg(candidate->Name.isEmpty() ? "Unnamed" : candidate->Name);
            
        auto* listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, QVariant::fromValue(const_cast<ItemEditor::ClientItem*>(candidate)));
        m_candidatesList->addItem(listItem);
    }
    
    m_candidatesCountLabel->setText(QString("%1 candidates").arg(candidates.size()));
}

void SpriteBrowser::showSpriteDetails(quint32 spriteId)
{
    m_selectedSpriteId = spriteId;
    
    // Find the client item with this sprite ID
    auto it = m_clientItems.find(static_cast<quint16>(spriteId));
    if (it != m_clientItems.end()) {
        const auto& clientItem = it.value();
        
        m_selectedSpriteView->setClientItem(&clientItem);
        m_spriteIdLabel->setText(QString("Sprite ID: %1").arg(spriteId));
        m_spriteSizeLabel->setText(QString("Size: %1x%2").arg(clientItem.Width).arg(clientItem.Height));
        
        // TODO: Calculate usage count
        m_spriteUsageLabel->setText("Usage: Unknown");
        
        // TODO: Populate used by list
        m_usedByList->clear();
    }
}

// Slot implementations
void SpriteBrowser::onSearchTextChanged()
{
    // TODO: Implement search filtering
    QString searchText = m_searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        // Show all sprites
    } else {
        // Filter sprites based on search criteria
    }
}

void SpriteBrowser::onFilterChanged()
{
    populateSpriteGrid();
}

void SpriteBrowser::onZoomChanged(int value)
{
    m_zoomLevel = value;
    m_zoomLabel->setText(QString("%1px").arg(value));
    
    // Update sprites per row based on zoom level
    int availableWidth = m_spriteScrollArea->viewport()->width();
    m_spritesPerRow = qMax(1, availableWidth / (value + 4)); // +4 for spacing
    
    // Update all existing sprite views
    for (auto* view : m_spriteViews) {
        view->setFixedSize(value, value);
    }
    
    // Refresh grid layout
    populateSpriteGrid();
}

void SpriteBrowser::onShowCandidatesClicked()
{
    if (m_currentItem) {
        showSpriteCandidates(m_currentItem);
    }
}

void SpriteBrowser::onAssignSpriteClicked()
{
    auto selectedItems = m_candidatesList->selectedItems();
    if (selectedItems.isEmpty() || m_selectedSpriteId == 0) {
        return;
    }
    
    auto* targetItem = selectedItems.first()->data(Qt::UserRole).value<ItemEditor::ClientItem*>();
    if (targetItem) {
        emit spriteAssignmentRequested(m_selectedSpriteId, targetItem);
    }
}

void SpriteBrowser::onAnalyzeSimilarityClicked()
{
    // TODO: Implement comprehensive similarity analysis
    QMessageBox::information(this, "Analyze Similarity", 
                           "Sprite similarity analysis will be implemented in the next phase.");
}

bool SpriteBrowser::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto* spriteView = qobject_cast<ClientItemView*>(obj);
        if (spriteView) {
            quint32 spriteId = spriteView->property("spriteId").toUInt();
            showSpriteDetails(spriteId);
            emit spriteSelected(spriteId);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        auto* spriteView = qobject_cast<ClientItemView*>(obj);
        if (spriteView) {
            quint32 spriteId = spriteView->property("spriteId").toUInt();
            emit spriteDoubleClicked(spriteId);
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

} // namespace Widgets
} // namespace UI