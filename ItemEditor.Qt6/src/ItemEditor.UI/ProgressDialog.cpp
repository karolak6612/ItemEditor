#include "ProgressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QMessageBox>
#include <QThread>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QProgressDialog(parent)
    , m_operationLabel(nullptr)
    , m_detailLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_cancelButton(nullptr)
    , m_progressBar(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_autoCloseTimer(new QTimer(this))
    , m_timeEstimationEnabled(true)
    , m_cancellationEnabled(true)
    , m_cancellationRequested(false)
    , m_batchMode(false)
    , m_totalItems(0)
    , m_completedItems(0)
    , m_itemName("items")
    , m_autoClose(false)
    , m_autoCloseDelay(1000)
{
    setupUI();
    setupConnections();
    applyDarkTheme();
}

ProgressDialog::ProgressDialog(const QString &labelText, const QString &cancelButtonText,
                              int minimum, int maximum, QWidget *parent)
    : ProgressDialog(parent)
{
    setLabelText(labelText);
    setCancelButtonText(cancelButtonText);
    setRange(minimum, maximum);
}

ProgressDialog::~ProgressDialog()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_autoCloseTimer) {
        m_autoCloseTimer->stop();
    }
}

void ProgressDialog::setOperation(const QString& operation)
{
    if (m_operationLabel) {
        m_operationLabel->setText(operation);
    }
    setLabelText(operation);
}

void ProgressDialog::setDetailText(const QString& detail)
{
    if (m_detailLabel) {
        m_detailLabel->setText(detail);
        m_detailLabel->setVisible(!detail.isEmpty());
    }
}

void ProgressDialog::setProgressText(const QString& text)
{
    setLabelText(text);
}

void ProgressDialog::enableTimeEstimation(bool enable)
{
    m_timeEstimationEnabled = enable;
    if (m_timeLabel) {
        m_timeLabel->setVisible(enable);
    }
    
    if (enable && !m_updateTimer->isActive()) {
        m_updateTimer->start(1000); // Update every second
    } else if (!enable && m_updateTimer->isActive()) {
        m_updateTimer->stop();
    }
}

QString ProgressDialog::getElapsedTimeString() const
{
    if (!m_elapsedTimer.isValid()) {
        return "00:00:00";
    }
    return formatTime(m_elapsedTimer.elapsed());
}

QString ProgressDialog::getEstimatedTimeString() const
{
    if (!m_timeEstimationEnabled || !m_elapsedTimer.isValid()) {
        return "Unknown";
    }
    
    qint64 estimated = calculateEstimatedTime(value());
    return estimated > 0 ? formatTime(estimated) : "Unknown";
}

QString ProgressDialog::getRemainingTimeString() const
{
    if (!m_timeEstimationEnabled || !m_elapsedTimer.isValid()) {
        return "Unknown";
    }
    
    qint64 estimated = calculateEstimatedTime(value());
    qint64 elapsed = m_elapsedTimer.elapsed();
    qint64 remaining = estimated - elapsed;
    
    return remaining > 0 ? formatTime(remaining) : "00:00:00";
}

void ProgressDialog::setCancellationEnabled(bool enabled)
{
    m_cancellationEnabled = enabled;
    setCancelButtonText(enabled ? "Cancel" : "");
    
    if (m_cancelButton) {
        m_cancelButton->setVisible(enabled);
        m_cancelButton->setEnabled(enabled);
    }
}

bool ProgressDialog::isCancellationRequested() const
{
    return m_cancellationRequested;
}

void ProgressDialog::setCancellationCallback(std::function<void()> callback)
{
    m_cancellationCallback = callback;
}

void ProgressDialog::setProgress(int value)
{
    setValue(value);
    updateProgressHistory(value);
    
    if (m_batchMode) {
        setLabelText(formatBatchText());
    }
    
    emit progressUpdated(value);
    
    // Process events to keep UI responsive
    QApplication::processEvents();
    
    // Check for completion
    if (value >= maximum()) {
        emit operationCompleted();
        
        if (m_autoClose) {
            m_autoCloseTimer->start(m_autoCloseDelay);
        }
    }
}

void ProgressDialog::setProgress(int value, const QString& text)
{
    setLabelText(text);
    setProgress(value);
}

void ProgressDialog::incrementProgress(int delta)
{
    setProgress(value() + delta);
}

void ProgressDialog::setBatchOperation(int totalItems, const QString& itemName)
{
    m_batchMode = true;
    m_totalItems = totalItems;
    m_completedItems = 0;
    m_itemName = itemName;
    
    setRange(0, totalItems);
    setLabelText(formatBatchText());
}

void ProgressDialog::setBatchProgress(int completedItems)
{
    m_completedItems = completedItems;
    setProgress(completedItems);
}

void ProgressDialog::setBatchProgress(int completedItems, const QString& currentItem)
{
    m_completedItems = completedItems;
    setDetailText(QString("Processing: %1").arg(currentItem));
    setProgress(completedItems);
}

void ProgressDialog::setAutoClose(bool autoClose, int delayMs)
{
    m_autoClose = autoClose;
    m_autoCloseDelay = delayMs;
}

void ProgressDialog::showProgress()
{
    m_elapsedTimer.start();
    m_cancellationRequested = false;
    
    if (m_timeEstimationEnabled) {
        m_updateTimer->start(1000);
    }
    
    show();
    raise();
    activateWindow();
}

void ProgressDialog::hideProgress()
{
    if (m_updateTimer->isActive()) {
        m_updateTimer->stop();
    }
    if (m_autoCloseTimer->isActive()) {
        m_autoCloseTimer->stop();
    }
    
    hide();
}

void ProgressDialog::reset()
{
    QProgressDialog::reset();
    
    m_cancellationRequested = false;
    m_progressHistory.clear();
    
    if (m_detailLabel) {
        m_detailLabel->clear();
        m_detailLabel->setVisible(false);
    }
    
    if (m_timeLabel) {
        m_timeLabel->clear();
    }
    
    if (m_updateTimer->isActive()) {
        m_updateTimer->stop();
    }
    
    if (m_autoCloseTimer->isActive()) {
        m_autoCloseTimer->stop();
    }
}

void ProgressDialog::cancel()
{
    if (!m_cancellationEnabled) {
        return;
    }
    
    m_cancellationRequested = true;
    
    // Call cancellation callback if provided
    if (m_cancellationCallback) {
        m_cancellationCallback();
    }
    
    emit cancelled();
    QProgressDialog::cancel();
}

void ProgressDialog::closeEvent(QCloseEvent *event)
{
    if (m_cancellationEnabled && !m_cancellationRequested) {
        // Ask for confirmation before closing
        int result = QMessageBox::question(this, "Cancel Operation",
                                         "Are you sure you want to cancel the current operation?",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
        
        if (result == QMessageBox::Yes) {
            cancel();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void ProgressDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && m_cancellationEnabled) {
        cancel();
    } else {
        QProgressDialog::keyPressEvent(event);
    }
}

void ProgressDialog::showEvent(QShowEvent *event)
{
    QProgressDialog::showEvent(event);
    
    // Center on parent or screen
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void ProgressDialog::onCancelClicked()
{
    cancel();
}

void ProgressDialog::updateTimeDisplay()
{
    if (!m_timeLabel || !m_timeEstimationEnabled) {
        return;
    }
    
    QString timeText = QString("Elapsed: %1").arg(getElapsedTimeString());
    
    if (value() > minimum() && value() < maximum()) {
        timeText += QString(" | Remaining: %1").arg(getRemainingTimeString());
    }
    
    m_timeLabel->setText(timeText);
}

void ProgressDialog::autoCloseTimeout()
{
    m_autoCloseTimer->stop();
    accept();
}

void ProgressDialog::setupUI()
{
    // Set dialog properties
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(400);
    setMinimumHeight(150);
    
    // Create custom layout with additional labels
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Operation label (main text)
    m_operationLabel = new QLabel(this);
    m_operationLabel->setWordWrap(true);
    layout->addWidget(m_operationLabel);
    
    // Progress bar
    m_progressBar = new QProgressBar(this);
    layout->addWidget(m_progressBar);
    
    // Detail label (secondary text)
    m_detailLabel = new QLabel(this);
    m_detailLabel->setWordWrap(true);
    m_detailLabel->setVisible(false);
    layout->addWidget(m_detailLabel);
    
    // Time estimation label
    m_timeLabel = new QLabel(this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_timeLabel);
    
    // Cancel button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_cancelButton);
    
    layout->addLayout(buttonLayout);
    
    // Set the custom widget (this replaces the default QProgressDialog layout)
    // Note: We'll use the existing QProgressDialog functionality but enhance it
    updateLayout();
}

void ProgressDialog::setupConnections()
{
    // Timer connections
    connect(m_updateTimer, &QTimer::timeout, this, &ProgressDialog::updateTimeDisplay);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &ProgressDialog::autoCloseTimeout);
    
    // Cancel button connection
    if (m_cancelButton) {
        connect(m_cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancelClicked);
    }
}

void ProgressDialog::applyDarkTheme()
{
    setStyleSheet(R"(
        QProgressDialog {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        
        QLabel {
            color: #DCDCDC;
            font-size: 11px;
        }
        
        QProgressBar {
            border: 1px solid #555555;
            border-radius: 3px;
            background-color: #2B2B2B;
            text-align: center;
            color: #DCDCDC;
        }
        
        QProgressBar::chunk {
            background-color: #6897BB;
            border-radius: 2px;
        }
        
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px 15px;
            color: #DCDCDC;
            min-width: 80px;
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
    )");
}

void ProgressDialog::updateLayout()
{
    // Sync our custom progress bar with the dialog's progress bar
    if (m_progressBar) {
        m_progressBar->setRange(minimum(), maximum());
        m_progressBar->setValue(value());
    }
    
    // Sync operation label with dialog's label text
    if (m_operationLabel) {
        m_operationLabel->setText(labelText());
    }
}

qint64 ProgressDialog::calculateEstimatedTime(int currentValue) const
{
    if (m_progressHistory.isEmpty() || currentValue <= minimum()) {
        return 0;
    }
    
    // Use linear regression on recent progress history
    cleanupProgressHistory();
    
    if (m_progressHistory.size() < 2) {
        return 0;
    }
    
    // Calculate average rate of progress
    qint64 totalTime = m_progressHistory.last().second - m_progressHistory.first().second;
    int totalProgress = m_progressHistory.last().first - m_progressHistory.first().first;
    
    if (totalProgress <= 0 || totalTime <= 0) {
        return 0;
    }
    
    double rate = static_cast<double>(totalProgress) / totalTime; // progress per millisecond
    int remainingProgress = maximum() - currentValue;
    
    return static_cast<qint64>(remainingProgress / rate);
}

QString ProgressDialog::formatTime(qint64 milliseconds) const
{
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    return QString("%1:%2:%3")
           .arg(hours, 2, 10, QChar('0'))
           .arg(minutes, 2, 10, QChar('0'))
           .arg(seconds, 2, 10, QChar('0'));
}

void ProgressDialog::updateProgressHistory(int value)
{
    qint64 timestamp = m_elapsedTimer.isValid() ? m_elapsedTimer.elapsed() : 0;
    m_progressHistory.append(qMakePair(value, timestamp));
    
    // Keep only recent history (last 10 data points)
    while (m_progressHistory.size() > 10) {
        m_progressHistory.removeFirst();
    }
}

void ProgressDialog::cleanupProgressHistory()
{
    // Remove old entries (older than 30 seconds)
    qint64 currentTime = m_elapsedTimer.isValid() ? m_elapsedTimer.elapsed() : 0;
    qint64 cutoffTime = currentTime - 30000; // 30 seconds
    
    while (!m_progressHistory.isEmpty() && m_progressHistory.first().second < cutoffTime) {
        m_progressHistory.removeFirst();
    }
}

QString ProgressDialog::formatProgressText(int value, int maximum) const
{
    double percentage = maximum > 0 ? (static_cast<double>(value) / maximum) * 100.0 : 0.0;
    return QString("Progress: %1 / %2 (%3%)")
           .arg(value)
           .arg(maximum)
           .arg(percentage, 0, 'f', 1);
}

QString ProgressDialog::formatBatchText() const
{
    if (!m_batchMode) {
        return QString();
    }
    
    double percentage = m_totalItems > 0 ? (static_cast<double>(m_completedItems) / m_totalItems) * 100.0 : 0.0;
    return QString("Processing %1: %2 / %3 (%4%)")
           .arg(m_itemName)
           .arg(m_completedItems)
           .arg(m_totalItems)
           .arg(percentage, 0, 'f', 1);
}