#pragma once

#include <QProgressDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

/**
 * @brief Custom progress dialog matching legacy system behavior
 * 
 * Provides progress indication for long-running operations with:
 * - Cancellation support with proper cleanup
 * - Progress reporting matching legacy system
 * - Automatic time estimation and remaining time display
 * - Modal operation with proper event handling
 */
class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    explicit ProgressDialog(const QString &labelText, const QString &cancelButtonText,
                           int minimum, int maximum, QWidget *parent = nullptr);
    ~ProgressDialog();

    // Enhanced progress reporting
    void setOperation(const QString& operation);
    void setDetailText(const QString& detail);
    void setProgressText(const QString& text);
    
    // Time estimation
    void enableTimeEstimation(bool enable = true);
    QString getElapsedTimeString() const;
    QString getEstimatedTimeString() const;
    QString getRemainingTimeString() const;
    
    // Cancellation handling
    void setCancellationEnabled(bool enabled);
    bool isCancellationRequested() const;
    void setCancellationCallback(std::function<void()> callback);
    
    // Progress updates with automatic UI refresh
    void setProgress(int value);
    void setProgress(int value, const QString& text);
    void incrementProgress(int delta = 1);
    
    // Batch operation support
    void setBatchOperation(int totalItems, const QString& itemName = "items");
    void setBatchProgress(int completedItems);
    void setBatchProgress(int completedItems, const QString& currentItem);
    
    // Auto-close functionality
    void setAutoClose(bool autoClose, int delayMs = 1000);
    
    // Show/hide with proper modal handling
    void showProgress();
    void hideProgress();

signals:
    void cancelled();
    void progressUpdated(int value);
    void operationCompleted();

public slots:
    void reset();
    void cancel();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onCancelClicked();
    void updateTimeDisplay();
    void autoCloseTimeout();

private:
    // UI components
    QLabel* m_operationLabel;
    QLabel* m_detailLabel;
    QLabel* m_timeLabel;
    QPushButton* m_cancelButton;
    QProgressBar* m_progressBar;
    
    // Time tracking
    QElapsedTimer m_elapsedTimer;
    QTimer* m_updateTimer;
    QTimer* m_autoCloseTimer;
    bool m_timeEstimationEnabled;
    
    // Cancellation handling
    bool m_cancellationEnabled;
    bool m_cancellationRequested;
    std::function<void()> m_cancellationCallback;
    
    // Batch operation tracking
    bool m_batchMode;
    int m_totalItems;
    int m_completedItems;
    QString m_itemName;
    
    // Auto-close settings
    bool m_autoClose;
    int m_autoCloseDelay;
    
    // Progress tracking for time estimation
    QList<QPair<int, qint64>> m_progressHistory; // value, timestamp
    
    // UI setup and styling
    void setupUI();
    void setupConnections();
    void applyDarkTheme();
    void updateLayout();
    
    // Time calculation helpers
    qint64 calculateEstimatedTime(int currentValue) const;
    QString formatTime(qint64 milliseconds) const;
    void updateProgressHistory(int value);
    void cleanupProgressHistory();
    
    // Text formatting helpers
    QString formatProgressText(int value, int maximum) const;
    QString formatBatchText() const;
};