#include "FindItemDialog.h"
#include "ui_FindItemDialog.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QTimer>
#include <QRegularExpression>

FindItemDialog::FindItemDialog(const ServerItemList* itemList, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindItemDialog)
    , m_itemList(itemList)
    , m_currentResultIndex(-1)
    , m_hasSelectedItem(false)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    applyDarkTheme();
    
    // Focus on search term input
    ui->searchTermLineEdit->setFocus();
}

FindItemDialog::~FindItemDialog()
{
    delete ui;
}

ServerItem FindItemDialog::getSelectedItem() const
{
    return m_selectedItem;
}

bool FindItemDialog::hasSelectedItem() const
{
    return m_hasSelectedItem;
}

void FindItemDialog::setSearchById(ItemId id)
{
    ui->searchByComboBox->setCurrentIndex(0); // ID
    ui->searchTermLineEdit->setText(QString::number(id));
    ui->exactMatchCheckBox->setChecked(true);
    performSearch();
}

void FindItemDialog::setSearchByName(const QString& name)
{
    ui->searchByComboBox->setCurrentIndex(1); // Name
    ui->searchTermLineEdit->setText(name);
    ui->exactMatchCheckBox->setChecked(false);
    performSearch();
}

void FindItemDialog::setSearchByType(ServerItemType type)
{
    ui->searchByComboBox->setCurrentIndex(2); // Type
    ui->searchTermLineEdit->setText(QString::number(static_cast<int>(type)));
    ui->exactMatchCheckBox->setChecked(true);
    performSearch();
}

void FindItemDialog::setSearchByClientId(ClientId clientId)
{
    ui->searchByComboBox->setCurrentIndex(3); // Client ID
    ui->searchTermLineEdit->setText(QString::number(clientId));
    ui->exactMatchCheckBox->setChecked(true);
    performSearch();
}

void FindItemDialog::onSearchButtonClicked()
{
    performSearch();
}

void FindItemDialog::onClearButtonClicked()
{
    ui->searchTermLineEdit->clear();
    ui->exactMatchCheckBox->setChecked(false);
    ui->caseSensitiveCheckBox->setChecked(false);
    clearResults();
    ui->searchTermLineEdit->setFocus();
}

void FindItemDialog::onSearchTermChanged()
{
    // Perform real-time search as user types (with small delay)
    static QTimer* searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300); // 300ms delay
    
    disconnect(searchTimer, &QTimer::timeout, this, &FindItemDialog::performSearch);
    connect(searchTimer, &QTimer::timeout, this, &FindItemDialog::performSearch);
    
    searchTimer->start();
}

void FindItemDialog::onSearchByChanged()
{
    // Clear search term when changing search type
    ui->searchTermLineEdit->clear();
    clearResults();
    ui->searchTermLineEdit->setFocus();
}

void FindItemDialog::onResultItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    int index = ui->resultsListWidget->row(item);
    selectResult(index);
    highlightResult(index);
}

void FindItemDialog::onResultItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    int index = ui->resultsListWidget->row(item);
    selectResult(index);
    accept(); // Close dialog and accept selection
}

void FindItemDialog::onPreviousButtonClicked()
{
    if (m_currentResultIndex > 0) {
        selectResult(m_currentResultIndex - 1);
        highlightResult(m_currentResultIndex);
    }
}

void FindItemDialog::onNextButtonClicked()
{
    if (m_currentResultIndex < m_searchResults.size() - 1) {
        selectResult(m_currentResultIndex + 1);
        highlightResult(m_currentResultIndex);
    }
}

void FindItemDialog::performSearch()
{
    if (!m_itemList) {
        clearResults();
        return;
    }
    
    QString searchTerm = ui->searchTermLineEdit->text().trimmed();
    if (searchTerm.isEmpty()) {
        clearResults();
        return;
    }
    
    bool exactMatch = ui->exactMatchCheckBox->isChecked();
    bool caseSensitive = ui->caseSensitiveCheckBox->isChecked();
    
    // Perform search based on selected criteria
    SearchBy searchBy = getCurrentSearchBy();
    QList<ServerItem> results;
    
    switch (searchBy) {
        case SearchBy::Id:
            results = searchById(searchTerm, exactMatch);
            break;
        case SearchBy::Name:
            results = searchByName(searchTerm, exactMatch, caseSensitive);
            break;
        case SearchBy::Type:
            results = searchByType(searchTerm, exactMatch);
            break;
        case SearchBy::ClientId:
            results = searchByClientId(searchTerm, exactMatch);
            break;
    }
    
    m_searchResults = results;
    m_currentResultIndex = results.isEmpty() ? -1 : 0;
    
    updateResultsList();
    updateNavigationButtons();
    updateResultCount();
    
    // Select first result if available
    if (!results.isEmpty()) {
        selectResult(0);
        highlightResult(0);
    }
}

void FindItemDialog::clearResults()
{
    m_searchResults.clear();
    m_currentResultIndex = -1;
    m_hasSelectedItem = false;
    
    ui->resultsListWidget->clear();
    updateNavigationButtons();
    updateResultCount();
}

void FindItemDialog::updateResultsList()
{
    ui->resultsListWidget->clear();
    
    for (const ServerItem& item : m_searchResults) {
        QString displayText = formatItemForDisplay(item);
        QListWidgetItem* listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, QVariant::fromValue(item.id));
        ui->resultsListWidget->addItem(listItem);
    }
}

void FindItemDialog::updateNavigationButtons()
{
    bool hasResults = !m_searchResults.isEmpty();
    bool hasPrevious = hasResults && m_currentResultIndex > 0;
    bool hasNext = hasResults && m_currentResultIndex < m_searchResults.size() - 1;
    
    ui->previousButton->setEnabled(hasPrevious);
    ui->nextButton->setEnabled(hasNext);
}

void FindItemDialog::updateResultCount()
{
    int count = m_searchResults.size();
    QString text;
    
    if (count == 0) {
        text = "0 results found";
    } else if (count == 1) {
        text = "1 result found";
    } else {
        text = QString("%1 results found").arg(count);
    }
    
    ui->resultCountLabel->setText(text);
}

QList<ServerItem> FindItemDialog::searchById(const QString& searchTerm, bool exactMatch) const
{
    QList<ServerItem> results;
    bool ok;
    ItemId searchId = searchTerm.toUInt(&ok);
    
    if (!ok) return results;
    
    if (exactMatch) {
        // Exact ID match
        const ServerItem* item = m_itemList->findItem(searchId);
        if (item) {
            results.append(*item);
        }
    } else {
        // Partial ID match (search for IDs containing the digits)
        for (const ServerItem& item : *m_itemList) {
            QString itemIdStr = QString::number(item.id);
            if (itemIdStr.contains(searchTerm, Qt::CaseInsensitive)) {
                results.append(item);
            }
        }
    }
    
    return results;
}

QList<ServerItem> FindItemDialog::searchByName(const QString& searchTerm, bool exactMatch, bool caseSensitive) const
{
    QList<ServerItem> results;
    Qt::CaseSensitivity sensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    
    if (exactMatch) {
        // Use ServerItemList's built-in exact name search
        results = m_itemList->findItemsByName(searchTerm, true);
    } else {
        // Partial name match
        for (const ServerItem& item : *m_itemList) {
            if (item.name.contains(searchTerm, sensitivity)) {
                results.append(item);
            }
        }
    }
    
    return results;
}

QList<ServerItem> FindItemDialog::searchByType(const QString& searchTerm, bool exactMatch) const
{
    QList<ServerItem> results;
    bool ok;
    int searchTypeInt = searchTerm.toInt(&ok);
    
    if (ok) {
        ServerItemType searchType = static_cast<ServerItemType>(searchTypeInt);
        results = m_itemList->findItemsByType(searchType);
    } else if (!exactMatch) {
        // Search by type name (partial match)
        QString lowerSearchTerm = searchTerm.toLower();
        for (const ServerItem& item : *m_itemList) {
            QString typeName = QString::number(static_cast<int>(item.type));
            if (typeName.contains(lowerSearchTerm, Qt::CaseInsensitive)) {
                results.append(item);
            }
        }
    }
    
    return results;
}

QList<ServerItem> FindItemDialog::searchByClientId(const QString& searchTerm, bool exactMatch) const
{
    QList<ServerItem> results;
    bool ok;
    ClientId searchClientId = searchTerm.toUInt(&ok);
    
    if (!ok) return results;
    
    if (exactMatch) {
        // Exact client ID match
        for (const ServerItem& item : *m_itemList) {
            if (item.clientId == searchClientId) {
                results.append(item);
            }
        }
    } else {
        // Partial client ID match
        for (const ServerItem& item : *m_itemList) {
            QString clientIdStr = QString::number(item.clientId);
            if (clientIdStr.contains(searchTerm, Qt::CaseInsensitive)) {
                results.append(item);
            }
        }
    }
    
    return results;
}

FindItemDialog::SearchBy FindItemDialog::getCurrentSearchBy() const
{
    int index = ui->searchByComboBox->currentIndex();
    return static_cast<SearchBy>(index);
}

QString FindItemDialog::formatItemForDisplay(const ServerItem& item) const
{
    return QString("ID: %1 - %2 (Type: %3, Client ID: %4)")
           .arg(item.id)
           .arg(item.name.isEmpty() ? "Unnamed" : item.name)
           .arg(static_cast<int>(item.type))
           .arg(item.clientId);
}

void FindItemDialog::selectResult(int index)
{
    if (index >= 0 && index < m_searchResults.size()) {
        m_currentResultIndex = index;
        m_selectedItem = m_searchResults[index];
        m_hasSelectedItem = true;
        
        // Update list selection
        ui->resultsListWidget->setCurrentRow(index);
        
        updateNavigationButtons();
        emit itemSelected(m_selectedItem);
    }
}

void FindItemDialog::highlightResult(int index)
{
    if (index >= 0 && index < m_searchResults.size()) {
        emit itemHighlighted(m_searchResults[index]);
    }
}

void FindItemDialog::setupConnections()
{
    // Search controls
    connect(ui->searchButton, &QPushButton::clicked, this, &FindItemDialog::onSearchButtonClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &FindItemDialog::onClearButtonClicked);
    connect(ui->searchTermLineEdit, &QLineEdit::textChanged, this, &FindItemDialog::onSearchTermChanged);
    connect(ui->searchByComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &FindItemDialog::onSearchByChanged);
    
    // Results list
    connect(ui->resultsListWidget, &QListWidget::itemClicked, this, &FindItemDialog::onResultItemClicked);
    connect(ui->resultsListWidget, &QListWidget::itemDoubleClicked, this, &FindItemDialog::onResultItemDoubleClicked);
    
    // Navigation buttons
    connect(ui->previousButton, &QPushButton::clicked, this, &FindItemDialog::onPreviousButtonClicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &FindItemDialog::onNextButtonClicked);
    
    // Enter key in search field should trigger search
    connect(ui->searchTermLineEdit, &QLineEdit::returnPressed, this, &FindItemDialog::onSearchButtonClicked);
}

void FindItemDialog::setupUI()
{
    // Set dialog properties
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Set initial state
    clearResults();
    
    // Set focus policy
    ui->searchTermLineEdit->setFocus();
}

void FindItemDialog::applyDarkTheme()
{
    // Apply dark theme styling to match legacy system
    setStyleSheet(R"(
        QDialog {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 5px;
            margin-top: 1ex;
            color: #DCDCDC;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QLabel {
            color: #DCDCDC;
        }
        
        QLineEdit {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
        }
        
        QLineEdit:focus {
            border: 1px solid #6897BB;
        }
        
        QComboBox {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
        }
        
        QComboBox:focus {
            border: 1px solid #6897BB;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #DCDCDC;
        }
        
        QCheckBox {
            color: #DCDCDC;
        }
        
        QCheckBox::indicator {
            width: 13px;
            height: 13px;
        }
        
        QCheckBox::indicator:unchecked {
            background-color: #45494A;
            border: 1px solid #555555;
        }
        
        QCheckBox::indicator:checked {
            background-color: #6897BB;
            border: 1px solid #6897BB;
        }
        
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px 15px;
            color: #DCDCDC;
        }
        
        QPushButton:hover {
            background-color: #4C5052;
        }
        
        QPushButton:pressed {
            background-color: #3C3F41;
        }
        
        QPushButton:disabled {
            background-color: #3C3F41;
            color: #999999;
        }
        
        QListWidget {
            background-color: #2B2B2B;
            border: 1px solid #555555;
            color: #DCDCDC;
            alternate-background-color: #313335;
        }
        
        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #555555;
        }
        
        QListWidget::item:selected {
            background-color: #6897BB;
        }
        
        QListWidget::item:hover {
            background-color: #4C5052;
        }
    )");
}