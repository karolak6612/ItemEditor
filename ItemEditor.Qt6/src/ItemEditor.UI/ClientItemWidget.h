#pragma once

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include "../ItemEditor.Core/ClientItem.h"
#include "../ItemEditor.Plugins/ClientDataTypes.h"

/**
 * @brief Custom widget for displaying client item sprites with advanced visualization
 * 
 * Provides comprehensive sprite visualization with:
 * - 32x32 pixel sprite rendering with scaling
 * - Transparency support and background color indication
 * - Animation frame display and playback
 * - Zoom capabilities (1x to 8x)
 * - Multiple view modes (normal, transparency, outline)
 * - Frame navigation and animation controls
 * - Context menu for sprite operations
 */
class ClientItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientItemWidget(QWidget *parent = nullptr);
    ~ClientItemWidget();

    // Data management
    void setClientItem(const ClientItem* item);
    void setSpriteData(const QList<SpriteData>& sprites);
    void clearSprite();
    const ClientItem* getClientItem() const;

    // Display settings
    void setZoomLevel(int level); // 1-8x zoom
    int getZoomLevel() const;
    void setBackgroundColor(const QColor& color);
    QColor getBackgroundColor() const;
    void setShowTransparency(bool show);
    bool getShowTransparency() const;
    void setShowGrid(bool show);
    bool getShowGrid() const;

    // Animation control
    void setCurrentFrame(int frame);
    int getCurrentFrame() const;
    int getFrameCount() const;
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const;
    void setAnimationSpeed(int fps); // 1-30 FPS
    int getAnimationSpeed() const;

    // View modes
    enum ViewMode {
        Normal,
        TransparencyOnly,
        OutlineOnly,
        AlphaChannel
    };
    
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const;

    // Sprite information
    QSize getSpriteSize() const;
    QSize getActualSize() const; // Size with zoom applied
    bool hasValidSprite() const;
    bool hasAnimation() const;

    // Export functionality
    QPixmap getCurrentFramePixmap() const;
    QList<QPixmap> getAllFramesPixmaps() const;
    bool exportCurrentFrame(const QString& filePath) const;
    bool exportAllFrames(const QString& basePath) const;

signals:
    void spriteClicked(const QPoint& position);
    void spriteDoubleClicked(const QPoint& position);
    void frameChanged(int frame);
    void zoomChanged(int level);
    void animationStateChanged(bool playing);
    void contextMenuRequested(const QPoint& position);

public slots:
    void playAnimation();
    void pauseAnimation();
    void stopAnimation();
    void nextFrame();
    void previousFrame();
    void firstFrame();
    void lastFrame();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onAnimationTimer();
    void onZoomSliderChanged(int value);
    void onFrameSpinBoxChanged(int value);
    void onAnimationSpeedChanged(int value);
    void onViewModeChanged();
    void onBackgroundColorChanged();

private:
    // UI components
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_spriteCanvas;
    QHBoxLayout* m_controlsLayout;
    
    // Controls
    QGroupBox* m_zoomGroup;
    QSlider* m_zoomSlider;
    QLabel* m_zoomLabel;
    QPushButton* m_fitToWindowButton;
    
    QGroupBox* m_frameGroup;
    QSpinBox* m_frameSpinBox;
    QLabel* m_frameLabel;
    QPushButton* m_playButton;
    QPushButton* m_stopButton;
    QPushButton* m_prevFrameButton;
    QPushButton* m_nextFrameButton;
    
    QGroupBox* m_viewGroup;
    QComboBox* m_viewModeCombo;
    QCheckBox* m_showTransparencyCheck;
    QCheckBox* m_showGridCheck;
    QPushButton* m_backgroundColorButton;
    
    QGroupBox* m_animationGroup;
    QSlider* m_animationSpeedSlider;
    QLabel* m_animationSpeedLabel;

    // Data
    const ClientItem* m_clientItem;
    QList<SpriteData> m_spriteData;
    QList<QPixmap> m_framePixmaps;
    
    // Display settings
    int m_zoomLevel;
    QColor m_backgroundColor;
    bool m_showTransparency;
    bool m_showGrid;
    ViewMode m_viewMode;
    
    // Animation
    QTimer* m_animationTimer;
    int m_currentFrame;
    int m_animationSpeed; // FPS
    bool m_animationEnabled;
    bool m_isPlaying;
    
    // Rendering
    QPixmap m_currentPixmap;
    QPoint m_spriteOffset;
    QSize m_canvasSize;
    
    // Private methods
    void setupUI();
    void setupControls();
    void setupScrollArea();
    void connectSignals();
    void applyDarkTheme();
    void updateControls();
    void updatePixmaps();
    void updateCurrentPixmap();
    void renderSprite(QPainter& painter, const SpriteData& sprite, const QRect& rect);
    void renderTransparency(QPainter& painter, const QRect& rect);
    void renderGrid(QPainter& painter, const QRect& rect);
    void renderOutline(QPainter& painter, const QRect& rect);
    QPixmap createPixmapFromSprite(const SpriteData& sprite);
    QPixmap applyViewMode(const QPixmap& pixmap);
    QColor getPixelColor(const QPixmap& pixmap, const QPoint& position) const;
    QPoint mapToSpriteCoordinates(const QPoint& widgetPos) const;
    QRect getSpriteRect() const;
    void updateCanvasSize();
    void centerSprite();
    bool isValidFrameIndex(int frame) const;
    void clampCurrentFrame();
    QString getFrameText() const;
    QString getZoomText() const;
    void createContextMenu(const QPoint& position);
    
    // Color utilities
    static QColor blendColors(const QColor& base, const QColor& overlay, qreal alpha);
    static QPixmap createTransparencyBackground(const QSize& size);
    static QPixmap createCheckerboardPattern(const QSize& size, int checkerSize = 8);
};