#include "ClientItemWidget.h"
#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QDrag>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QStyleOption>
#include <QBitmap>
#include <cmath>

ClientItemWidget::ClientItemWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_spriteCanvas(nullptr)
    , m_controlsLayout(nullptr)
    , m_zoomGroup(nullptr)
    , m_zoomSlider(nullptr)
    , m_zoomLabel(nullptr)
    , m_fitToWindowButton(nullptr)
    , m_frameGroup(nullptr)
    , m_frameSpinBox(nullptr)
    , m_frameLabel(nullptr)
    , m_playButton(nullptr)
    , m_stopButton(nullptr)
    , m_prevFrameButton(nullptr)
    , m_nextFrameButton(nullptr)
    , m_viewGroup(nullptr)
    , m_viewModeCombo(nullptr)
    , m_showTransparencyCheck(nullptr)
    , m_showGridCheck(nullptr)
    , m_backgroundColorButton(nullptr)
    , m_animationGroup(nullptr)
    , m_animationSpeedSlider(nullptr)
    , m_animationSpeedLabel(nullptr)
    , m_clientItem(nullptr)
    , m_zoomLevel(4)
    , m_backgroundColor(64, 64, 64)
    , m_showTransparency(true)
    , m_showGrid(false)
    , m_viewMode(Normal)
    , m_animationTimer(nullptr)
    , m_currentFrame(0)
    , m_animationSpeed(10)
    , m_animationEnabled(true)
    , m_isPlaying(false)
    , m_spriteOffset(0, 0)
    , m_canvasSize(32, 32)
{
    setupUI();
    setupControls();
    setupScrollArea();
    connectSignals();
    applyDarkTheme();
    updateControls();
    
    // Initialize animation timer
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &ClientItemWidget::onAnimationTimer);
    
    // Set minimum size
    setMinimumSize(300, 400);
}

ClientItemWidget::~ClientItemWidget()
{
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
}

void ClientItemWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Create scroll area for sprite display
    setupScrollArea();
    
    // Create controls layout
    m_controlsLayout = new QHBoxLayout();
    m_controlsLayout->setSpacing(4);
    
    m_mainLayout->addWidget(m_scrollArea, 1); // Give scroll area most space
    m_mainLayout->addLayout(m_controlsLayout);
}

void ClientItemWidget::setupControls()
{
    // Zoom controls
    m_zoomGroup = new QGroupBox("Zoom", this);
    QVBoxLayout* zoomLayout = new QVBoxLayout(m_zoomGroup);
    
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(1, 8);
    m_zoomSlider->setValue(m_zoomLevel);
    m_zoomSlider->setTickPosition(QSlider::TicksBelow);
    m_zoomSlider->setTickInterval(1);
    
    m_zoomLabel = new QLabel(getZoomText(), this);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    
    m_fitToWindowButton = new QPushButton("Fit", this);
    m_fitToWindowButton->setMaximumWidth(40);
    
    QHBoxLayout* zoomControlLayout = new QHBoxLayout();
    zoomControlLayout->addWidget(m_zoomSlider);
    zoomControlLayout->addWidget(m_fitToWindowButton);
    
    zoomLayout->addWidget(m_zoomLabel);
    zoomLayout->addLayout(zoomControlLayout);
    
    // Frame controls
    m_frameGroup = new QGroupBox("Frame", this);
    QVBoxLayout* frameLayout = new QVBoxLayout(m_frameGroup);
    
    m_frameSpinBox = new QSpinBox(this);
    m_frameSpinBox->setRange(0, 0);
    m_frameSpinBox->setValue(0);
    
    m_frameLabel = new QLabel(getFrameText(), this);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    
    QHBoxLayout* frameButtonLayout = new QHBoxLayout();
    m_prevFrameButton = new QPushButton("◀", this);
    m_playButton = new QPushButton("▶", this);
    m_stopButton = new QPushButton("⏹", this);
    m_nextFrameButton = new QPushButton("▶", this);
    
    m_prevFrameButton->setMaximumWidth(30);
    m_playButton->setMaximumWidth(30);
    m_stopButton->setMaximumWidth(30);
    m_nextFrameButton->setMaximumWidth(30);
    
    frameButtonLayout->addWidget(m_prevFrameButton);
    frameButtonLayout->addWidget(m_playButton);
    frameButtonLayout->addWidget(m_stopButton);
    frameButtonLayout->addWidget(m_nextFrameButton);
    
    frameLayout->addWidget(m_frameLabel);
    frameLayout->addWidget(m_frameSpinBox);
    frameLayout->addLayout(frameButtonLayout);
    
    // View controls
    m_viewGroup = new QGroupBox("View", this);
    QVBoxLayout* viewLayout = new QVBoxLayout(m_viewGroup);
    
    m_viewModeCombo = new QComboBox(this);
    m_viewModeCombo->addItem("Normal", static_cast<int>(Normal));
    m_viewModeCombo->addItem("Transparency", static_cast<int>(TransparencyOnly));
    m_viewModeCombo->addItem("Outline", static_cast<int>(OutlineOnly));
    m_viewModeCombo->addItem("Alpha Channel", static_cast<int>(AlphaChannel));
    
    m_showTransparencyCheck = new QCheckBox("Show Transparency", this);
    m_showTransparencyCheck->setChecked(m_showTransparency);
    
    m_showGridCheck = new QCheckBox("Show Grid", this);
    m_showGridCheck->setChecked(m_showGrid);
    
    m_backgroundColorButton = new QPushButton("Background", this);
    m_backgroundColorButton->setStyleSheet(QString("background-color: %1").arg(m_backgroundColor.name()));
    
    viewLayout->addWidget(m_viewModeCombo);
    viewLayout->addWidget(m_showTransparencyCheck);
    viewLayout->addWidget(m_showGridCheck);
    viewLayout->addWidget(m_backgroundColorButton);
    
    // Animation controls
    m_animationGroup = new QGroupBox("Animation", this);
    QVBoxLayout* animationLayout = new QVBoxLayout(m_animationGroup);
    
    m_animationSpeedSlider = new QSlider(Qt::Horizontal, this);
    m_animationSpeedSlider->setRange(1, 30);
    m_animationSpeedSlider->setValue(m_animationSpeed);
    m_animationSpeedSlider->setTickPosition(QSlider::TicksBelow);
    m_animationSpeedSlider->setTickInterval(5);
    
    m_animationSpeedLabel = new QLabel(QString("%1 FPS").arg(m_animationSpeed), this);
    m_animationSpeedLabel->setAlignment(Qt::AlignCenter);
    
    animationLayout->addWidget(m_animationSpeedLabel);
    animationLayout->addWidget(m_animationSpeedSlider);
    
    // Add groups to controls layout
    m_controlsLayout->addWidget(m_zoomGroup);
    m_controlsLayout->addWidget(m_frameGroup);
    m_controlsLayout->addWidget(m_viewGroup);
    m_controlsLayout->addWidget(m_animationGroup);
}

void ClientItemWidget::setupScrollArea()
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setMinimumSize(200, 200);
    
    // Create sprite canvas
    m_spriteCanvas = new QWidget();
    m_spriteCanvas->setMinimumSize(32 * m_zoomLevel, 32 * m_zoomLevel);
    m_spriteCanvas->setStyleSheet("background-color: #404040;");
    
    m_scrollArea->setWidget(m_spriteCanvas);
}

void ClientItemWidget::connectSignals()
{
    // Zoom controls
    connect(m_zoomSlider, &QSlider::valueChanged, this, &ClientItemWidget::onZoomSliderChanged);
    connect(m_fitToWindowButton, &QPushButton::clicked, this, &ClientItemWidget::fitToWindow);
    
    // Frame controls
    connect(m_frameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &ClientItemWidget::onFrameSpinBoxChanged);
    connect(m_prevFrameButton, &QPushButton::clicked, this, &ClientItemWidget::previousFrame);
    connect(m_playButton, &QPushButton::clicked, this, &ClientItemWidget::playAnimation);
    connect(m_stopButton, &QPushButton::clicked, this, &ClientItemWidget::stopAnimation);
    connect(m_nextFrameButton, &QPushButton::clicked, this, &ClientItemWidget::nextFrame);
    
    // View controls
    connect(m_viewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientItemWidget::onViewModeChanged);
    connect(m_showTransparencyCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_showTransparency = checked;
        updateCurrentPixmap();
        m_spriteCanvas->update();
    });
    connect(m_showGridCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_showGrid = checked;
        m_spriteCanvas->update();
    });
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &ClientItemWidget::onBackgroundColorChanged);
    
    // Animation controls
    connect(m_animationSpeedSlider, &QSlider::valueChanged, this, &ClientItemWidget::onAnimationSpeedChanged);
}

void ClientItemWidget::applyDarkTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #555555;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 4px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 4px 0 4px;
        }
        QSlider::groove:horizontal {
            border: 1px solid #555555;
            height: 6px;
            background: #45494A;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #6897BB;
            border: 1px solid #555555;
            width: 14px;
            margin: -4px 0;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: #7BA7CB;
        }
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px 8px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #4C5052;
        }
        QPushButton:pressed {
            background-color: #3A3D3F;
        }
        QSpinBox {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 2px;
            border-radius: 2px;
        }
        QComboBox {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px;
            border-radius: 2px;
        }
        QCheckBox {
            spacing: 4px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
        }
        QCheckBox::indicator:unchecked {
            border: 1px solid #555555;
            background-color: #45494A;
        }
        QCheckBox::indicator:checked {
            border: 1px solid #6897BB;
            background-color: #6897BB;
        }
        QScrollArea {
            border: 1px solid #555555;
            background-color: #2B2B2B;
        }
    )");
}

// Data management methods
void ClientItemWidget::setClientItem(const ClientItem* item)
{
    m_clientItem = item;
    
    if (item) {
        // Extract sprite data from client item
        // This would typically come from the plugin system
        m_spriteData.clear();
        m_framePixmaps.clear();
        
        // For now, create placeholder sprite data
        // In a real implementation, this would be populated by the plugin
        updatePixmaps();
        updateControls();
    } else {
        clearSprite();
    }
}

void ClientItemWidget::setSpriteData(const QList<SpriteData>& sprites)
{
    m_spriteData = sprites;
    updatePixmaps();
    updateControls();
}

void ClientItemWidget::clearSprite()
{
    m_clientItem = nullptr;
    m_spriteData.clear();
    m_framePixmaps.clear();
    m_currentFrame = 0;
    m_currentPixmap = QPixmap();
    
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
    m_isPlaying = false;
    
    updateControls();
    if (m_spriteCanvas) {
        m_spriteCanvas->update();
    }
}

const ClientItem* ClientItemWidget::getClientItem() const
{
    return m_clientItem;
}

// Display settings
void ClientItemWidget::setZoomLevel(int level)
{
    level = qBound(1, level, 8);
    if (m_zoomLevel != level) {
        m_zoomLevel = level;
        m_zoomSlider->setValue(level);
        updateCanvasSize();
        updateCurrentPixmap();
        m_zoomLabel->setText(getZoomText());
        emit zoomChanged(level);
    }
}

int ClientItemWidget::getZoomLevel() const
{
    return m_zoomLevel;
}

void ClientItemWidget::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        m_backgroundColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
        updateCurrentPixmap();
        if (m_spriteCanvas) {
            m_spriteCanvas->update();
        }
    }
}

QColor ClientItemWidget::getBackgroundColor() const
{
    return m_backgroundColor;
}

void ClientItemWidget::setShowTransparency(bool show)
{
    if (m_showTransparency != show) {
        m_showTransparency = show;
        m_showTransparencyCheck->setChecked(show);
        updateCurrentPixmap();
        if (m_spriteCanvas) {
            m_spriteCanvas->update();
        }
    }
}

bool ClientItemWidget::getShowTransparency() const
{
    return m_showTransparency;
}

void ClientItemWidget::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        m_showGridCheck->setChecked(show);
        if (m_spriteCanvas) {
            m_spriteCanvas->update();
        }
    }
}

bool ClientItemWidget::getShowGrid() const
{
    return m_showGrid;
}

// Animation control
void ClientItemWidget::setCurrentFrame(int frame)
{
    if (isValidFrameIndex(frame) && m_currentFrame != frame) {
        m_currentFrame = frame;
        m_frameSpinBox->setValue(frame);
        updateCurrentPixmap();
        m_frameLabel->setText(getFrameText());
        if (m_spriteCanvas) {
            m_spriteCanvas->update();
        }
        emit frameChanged(frame);
    }
}

int ClientItemWidget::getCurrentFrame() const
{
    return m_currentFrame;
}

int ClientItemWidget::getFrameCount() const
{
    return m_framePixmaps.size();
}

void ClientItemWidget::setAnimationEnabled(bool enabled)
{
    m_animationEnabled = enabled;
    updateControls();
}

bool ClientItemWidget::isAnimationEnabled() const
{
    return m_animationEnabled;
}

void ClientItemWidget::setAnimationSpeed(int fps)
{
    fps = qBound(1, fps, 30);
    if (m_animationSpeed != fps) {
        m_animationSpeed = fps;
        m_animationSpeedSlider->setValue(fps);
        m_animationSpeedLabel->setText(QString("%1 FPS").arg(fps));
        
        if (m_animationTimer && m_animationTimer->isActive()) {
            m_animationTimer->setInterval(1000 / fps);
        }
    }
}

int ClientItemWidget::getAnimationSpeed() const
{
    return m_animationSpeed;
}

// View modes
void ClientItemWidget::setViewMode(ViewMode mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        m_viewModeCombo->setCurrentIndex(static_cast<int>(mode));
        updateCurrentPixmap();
        if (m_spriteCanvas) {
            m_spriteCanvas->update();
        }
    }
}

ClientItemWidget::ViewMode ClientItemWidget::getViewMode() const
{
    return m_viewMode;
}

// Sprite information
QSize ClientItemWidget::getSpriteSize() const
{
    return QSize(32, 32); // Standard sprite size
}

QSize ClientItemWidget::getActualSize() const
{
    QSize spriteSize = getSpriteSize();
    return QSize(spriteSize.width() * m_zoomLevel, spriteSize.height() * m_zoomLevel);
}

bool ClientItemWidget::hasValidSprite() const
{
    return !m_framePixmaps.isEmpty();
}

bool ClientItemWidget::hasAnimation() const
{
    return m_framePixmaps.size() > 1;
}

// Export functionality
QPixmap ClientItemWidget::getCurrentFramePixmap() const
{
    if (isValidFrameIndex(m_currentFrame)) {
        return m_framePixmaps[m_currentFrame];
    }
    return QPixmap();
}

QList<QPixmap> ClientItemWidget::getAllFramesPixmaps() const
{
    return m_framePixmaps;
}

bool ClientItemWidget::exportCurrentFrame(const QString& filePath) const
{
    QPixmap pixmap = getCurrentFramePixmap();
    if (!pixmap.isNull()) {
        return pixmap.save(filePath);
    }
    return false;
}

bool ClientItemWidget::exportAllFrames(const QString& basePath) const
{
    if (m_framePixmaps.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(basePath);
    QString dir = fileInfo.absolutePath();
    QString baseName = fileInfo.baseName();
    QString extension = fileInfo.suffix();
    
    for (int i = 0; i < m_framePixmaps.size(); ++i) {
        QString fileName = QString("%1/%2_frame_%3.%4")
            .arg(dir)
            .arg(baseName)
            .arg(i, 3, 10, QChar('0'))
            .arg(extension);
            
        if (!m_framePixmaps[i].save(fileName)) {
            return false;
        }
    }
    
    return true;
}

// Public slots
void ClientItemWidget::playAnimation()
{
    if (hasAnimation() && m_animationEnabled && !m_isPlaying) {
        m_isPlaying = true;
        m_animationTimer->start(1000 / m_animationSpeed);
        m_playButton->setText("⏸");
        emit animationStateChanged(true);
    } else if (m_isPlaying) {
        pauseAnimation();
    }
}

void ClientItemWidget::pauseAnimation()
{
    if (m_isPlaying) {
        m_isPlaying = false;
        m_animationTimer->stop();
        m_playButton->setText("▶");
        emit animationStateChanged(false);
    }
}

void ClientItemWidget::stopAnimation()
{
    if (m_isPlaying) {
        pauseAnimation();
    }
    setCurrentFrame(0);
}

void ClientItemWidget::nextFrame()
{
    if (hasAnimation()) {
        int nextFrame = (m_currentFrame + 1) % getFrameCount();
        setCurrentFrame(nextFrame);
    }
}

void ClientItemWidget::previousFrame()
{
    if (hasAnimation()) {
        int prevFrame = (m_currentFrame - 1 + getFrameCount()) % getFrameCount();
        setCurrentFrame(prevFrame);
    }
}

void ClientItemWidget::firstFrame()
{
    setCurrentFrame(0);
}

void ClientItemWidget::lastFrame()
{
    if (hasAnimation()) {
        setCurrentFrame(getFrameCount() - 1);
    }
}

void ClientItemWidget::zoomIn()
{
    setZoomLevel(m_zoomLevel + 1);
}

void ClientItemWidget::zoomOut()
{
    setZoomLevel(m_zoomLevel - 1);
}

void ClientItemWidget::resetZoom()
{
    setZoomLevel(4); // Default zoom level
}

void ClientItemWidget::fitToWindow()
{
    if (!m_scrollArea) {
        return;
    }
    
    QSize availableSize = m_scrollArea->viewport()->size();
    QSize spriteSize = getSpriteSize();
    
    int maxZoomX = availableSize.width() / spriteSize.width();
    int maxZoomY = availableSize.height() / spriteSize.height();
    int fitZoom = qMin(maxZoomX, maxZoomY);
    
    setZoomLevel(qBound(1, fitZoom, 8));
}

// Protected event handlers
void ClientItemWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    if (!m_spriteCanvas || m_spriteCanvas->parent() != m_scrollArea) {
        return;
    }
    
    QPainter painter(m_spriteCanvas);
    painter.fillRect(m_spriteCanvas->rect(), m_backgroundColor);
    
    if (hasValidSprite()) {
        QRect spriteRect = getSpriteRect();
        
        // Draw transparency background if enabled
        if (m_showTransparency) {
            renderTransparency(painter, spriteRect);
        }
        
        // Draw current frame
        if (!m_currentPixmap.isNull()) {
            painter.drawPixmap(spriteRect, m_currentPixmap);
        }
        
        // Draw grid if enabled
        if (m_showGrid) {
            renderGrid(painter, spriteRect);
        }
    }
}

void ClientItemWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && hasValidSprite()) {
        QPoint spritePos = mapToSpriteCoordinates(event->pos());
        emit spriteClicked(spritePos);
    }
    QWidget::mousePressEvent(event);
}

void ClientItemWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && hasValidSprite()) {
        QPoint spritePos = mapToSpriteCoordinates(event->pos());
        emit spriteDoubleClicked(spritePos);
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ClientItemWidget::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void ClientItemWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    centerSprite();
}

void ClientItemWidget::contextMenuEvent(QContextMenuEvent* event)
{
    createContextMenu(event->pos());
}

// Private slots
void ClientItemWidget::onAnimationTimer()
{
    if (hasAnimation() && m_isPlaying) {
        nextFrame();
    }
}

void ClientItemWidget::onZoomSliderChanged(int value)
{
    setZoomLevel(value);
}

void ClientItemWidget::onFrameSpinBoxChanged(int value)
{
    setCurrentFrame(value);
}

void ClientItemWidget::onAnimationSpeedChanged(int value)
{
    setAnimationSpeed(value);
}

void ClientItemWidget::onViewModeChanged()
{
    ViewMode mode = static_cast<ViewMode>(m_viewModeCombo->currentData().toInt());
    setViewMode(mode);
}

void ClientItemWidget::onBackgroundColorChanged()
{
    QColor color = QColorDialog::getColor(m_backgroundColor, this, "Select Background Color");
    if (color.isValid()) {
        setBackgroundColor(color);
    }
}

// Private methods
void ClientItemWidget::updateControls()
{
    bool hasSprite = hasValidSprite();
    bool hasAnim = hasAnimation();
    
    // Update frame controls
    m_frameSpinBox->setEnabled(hasSprite);
    m_frameSpinBox->setRange(0, qMax(0, getFrameCount() - 1));
    m_frameSpinBox->setValue(m_currentFrame);
    
    m_frameLabel->setText(getFrameText());
    
    m_prevFrameButton->setEnabled(hasAnim);
    m_playButton->setEnabled(hasAnim && m_animationEnabled);
    m_stopButton->setEnabled(hasAnim);
    m_nextFrameButton->setEnabled(hasAnim);
    
    // Update animation controls
    m_animationSpeedSlider->setEnabled(hasAnim && m_animationEnabled);
    
    // Update play button text
    m_playButton->setText(m_isPlaying ? "⏸" : "▶");
}

void ClientItemWidget::updatePixmaps()
{
    m_framePixmaps.clear();
    
    if (m_spriteData.isEmpty()) {
        // Create placeholder pixmap for testing
        QPixmap placeholder(32, 32);
        placeholder.fill(Qt::transparent);
        
        QPainter painter(&placeholder);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawRect(0, 0, 31, 31);
        painter.drawLine(0, 0, 31, 31);
        painter.drawLine(0, 31, 31, 0);
        painter.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, "?");
        
        m_framePixmaps.append(placeholder);
    } else {
        // Convert sprite data to pixmaps
        for (const SpriteData& sprite : m_spriteData) {
            QPixmap pixmap = createPixmapFromSprite(sprite);
            m_framePixmaps.append(pixmap);
        }
    }
    
    clampCurrentFrame();
    updateCurrentPixmap();
}

void ClientItemWidget::updateCurrentPixmap()
{
    if (isValidFrameIndex(m_currentFrame)) {
        m_currentPixmap = applyViewMode(m_framePixmaps[m_currentFrame]);
        
        // Scale pixmap according to zoom level
        if (m_zoomLevel != 1) {
            QSize scaledSize = getSpriteSize() * m_zoomLevel;
            m_currentPixmap = m_currentPixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
    } else {
        m_currentPixmap = QPixmap();
    }
    
    updateCanvasSize();
}

void ClientItemWidget::renderSprite(QPainter& painter, const SpriteData& sprite, const QRect& rect)
{
    Q_UNUSED(painter)
    Q_UNUSED(sprite)
    Q_UNUSED(rect)
    // Implementation would render sprite data to painter
}

void ClientItemWidget::renderTransparency(QPainter& painter, const QRect& rect)
{
    // Draw checkerboard pattern for transparency
    QPixmap checkerboard = createCheckerboardPattern(rect.size(), 4 * m_zoomLevel);
    painter.drawPixmap(rect, checkerboard);
}

void ClientItemWidget::renderGrid(QPainter& painter, const QRect& rect)
{
    painter.save();
    painter.setPen(QPen(QColor(128, 128, 128, 128), 1));
    
    // Draw vertical lines
    for (int x = rect.left(); x <= rect.right(); x += m_zoomLevel) {
        painter.drawLine(x, rect.top(), x, rect.bottom());
    }
    
    // Draw horizontal lines
    for (int y = rect.top(); y <= rect.bottom(); y += m_zoomLevel) {
        painter.drawLine(rect.left(), y, rect.right(), y);
    }
    
    painter.restore();
}

void ClientItemWidget::renderOutline(QPainter& painter, const QRect& rect)
{
    painter.save();
    painter.setPen(QPen(Qt::red, 2));
    painter.drawRect(rect);
    painter.restore();
}

QPixmap ClientItemWidget::createPixmapFromSprite(const SpriteData& sprite)
{
    // This would convert SpriteData to QPixmap
    // For now, create a placeholder
    Q_UNUSED(sprite)
    
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::cyan, 1));
    painter.drawRect(0, 0, 31, 31);
    painter.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, QString::number(sprite.id));
    
    return pixmap;
}

QPixmap ClientItemWidget::applyViewMode(const QPixmap& pixmap)
{
    switch (m_viewMode) {
        case Normal:
            return pixmap;
            
        case TransparencyOnly: {
            QPixmap result = pixmap;
            // Extract alpha channel and make it visible
            QBitmap mask = pixmap.mask();
            if (!mask.isNull()) {
                result.fill(Qt::white);
                result.setMask(mask);
            }
            return result;
        }
        
        case OutlineOnly: {
            QPixmap result(pixmap.size());
            result.fill(Qt::transparent);
            QPainter painter(&result);
            painter.setPen(QPen(Qt::red, 1));
            // Draw outline based on alpha channel
            // This is a simplified implementation
            painter.drawRect(result.rect());
            return result;
        }
        
        case AlphaChannel: {
            QPixmap result(pixmap.size());
            result.fill(Qt::black);
            QPainter painter(&result);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            // Draw alpha channel as grayscale
            // This is a simplified implementation
            painter.fillRect(result.rect(), Qt::white);
            return result;
        }
    }
    
    return pixmap;
}

QColor ClientItemWidget::getPixelColor(const QPixmap& pixmap, const QPoint& position) const
{
    if (pixmap.isNull() || !QRect(QPoint(0, 0), pixmap.size()).contains(position)) {
        return QColor();
    }
    
    QImage image = pixmap.toImage();
    return QColor(image.pixel(position));
}

QPoint ClientItemWidget::mapToSpriteCoordinates(const QPoint& widgetPos) const
{
    QRect spriteRect = getSpriteRect();
    if (!spriteRect.contains(widgetPos)) {
        return QPoint(-1, -1);
    }
    
    QPoint relativePos = widgetPos - spriteRect.topLeft();
    return QPoint(relativePos.x() / m_zoomLevel, relativePos.y() / m_zoomLevel);
}

QRect ClientItemWidget::getSpriteRect() const
{
    if (!m_spriteCanvas) {
        return QRect();
    }
    
    QSize actualSize = getActualSize();
    QSize canvasSize = m_spriteCanvas->size();
    
    int x = (canvasSize.width() - actualSize.width()) / 2;
    int y = (canvasSize.height() - actualSize.height()) / 2;
    
    return QRect(QPoint(x, y), actualSize);
}

void ClientItemWidget::updateCanvasSize()
{
    if (!m_spriteCanvas) {
        return;
    }
    
    QSize actualSize = getActualSize();
    QSize minSize = actualSize + QSize(20, 20); // Add padding
    
    m_spriteCanvas->setMinimumSize(minSize);
    m_spriteCanvas->resize(minSize);
    
    centerSprite();
}

void ClientItemWidget::centerSprite()
{
    // Canvas centering is handled by scroll area alignment
}

bool ClientItemWidget::isValidFrameIndex(int frame) const
{
    return frame >= 0 && frame < m_framePixmaps.size();
}

void ClientItemWidget::clampCurrentFrame()
{
    if (!m_framePixmaps.isEmpty()) {
        m_currentFrame = qBound(0, m_currentFrame, m_framePixmaps.size() - 1);
    } else {
        m_currentFrame = 0;
    }
}

QString ClientItemWidget::getFrameText() const
{
    if (hasValidSprite()) {
        return QString("Frame %1 of %2").arg(m_currentFrame + 1).arg(getFrameCount());
    }
    return "No frames";
}

QString ClientItemWidget::getZoomText() const
{
    return QString("%1x").arg(m_zoomLevel);
}

void ClientItemWidget::createContextMenu(const QPoint& position)
{
    QMenu contextMenu(this);
    
    QAction* exportFrameAction = contextMenu.addAction("Export Current Frame...");
    QAction* exportAllAction = contextMenu.addAction("Export All Frames...");
    contextMenu.addSeparator();
    QAction* copyAction = contextMenu.addAction("Copy to Clipboard");
    contextMenu.addSeparator();
    QAction* resetZoomAction = contextMenu.addAction("Reset Zoom");
    QAction* fitToWindowAction = contextMenu.addAction("Fit to Window");
    
    exportFrameAction->setEnabled(hasValidSprite());
    exportAllAction->setEnabled(hasAnimation());
    copyAction->setEnabled(hasValidSprite());
    
    QAction* selectedAction = contextMenu.exec(mapToGlobal(position));
    
    if (selectedAction == exportFrameAction) {
        QString fileName = QFileDialog::getSaveFileName(this, "Export Frame", 
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
            "PNG Files (*.png);;All Files (*)");
        if (!fileName.isEmpty()) {
            exportCurrentFrame(fileName);
        }
    } else if (selectedAction == exportAllAction) {
        QString fileName = QFileDialog::getSaveFileName(this, "Export All Frames", 
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
            "PNG Files (*.png);;All Files (*)");
        if (!fileName.isEmpty()) {
            exportAllFrames(fileName);
        }
    } else if (selectedAction == copyAction) {
        QPixmap pixmap = getCurrentFramePixmap();
        if (!pixmap.isNull()) {
            QApplication::clipboard()->setPixmap(pixmap);
        }
    } else if (selectedAction == resetZoomAction) {
        resetZoom();
    } else if (selectedAction == fitToWindowAction) {
        fitToWindow();
    }
    
    emit contextMenuRequested(position);
}

// Static utility methods
QColor ClientItemWidget::blendColors(const QColor& base, const QColor& overlay, qreal alpha)
{
    qreal invAlpha = 1.0 - alpha;
    return QColor(
        static_cast<int>(base.red() * invAlpha + overlay.red() * alpha),
        static_cast<int>(base.green() * invAlpha + overlay.green() * alpha),
        static_cast<int>(base.blue() * invAlpha + overlay.blue() * alpha),
        static_cast<int>(base.alpha() * invAlpha + overlay.alpha() * alpha)
    );
}

QPixmap ClientItemWidget::createTransparencyBackground(const QSize& size)
{
    return createCheckerboardPattern(size, 8);
}

QPixmap ClientItemWidget::createCheckerboardPattern(const QSize& size, int checkerSize)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    painter.fillRect(pixmap.rect(), Qt::white);
    
    QColor lightGray(240, 240, 240);
    QColor darkGray(200, 200, 200);
    
    for (int y = 0; y < size.height(); y += checkerSize) {
        for (int x = 0; x < size.width(); x += checkerSize) {
            bool isEven = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            painter.fillRect(x, y, checkerSize, checkerSize, isEven ? lightGray : darkGray);
        }
    }
    
    return pixmap;
}