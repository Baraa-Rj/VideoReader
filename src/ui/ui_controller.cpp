#include "ui/ui_controller.h"
#include <QWidget>

UIController::UIController(QObject *parent)
    : QObject(parent)
    , imageLabel(nullptr)
    , loadButton(nullptr)
    , playButton(nullptr)
    , fpsSpinBox(nullptr)
    , mainLayout(nullptr)
    , fpsLayout(nullptr)
    , buttonLayout(nullptr)
{
}

UIController::~UIController()
{
}

void UIController::setupUI(QWidget *centralWidget)
{
    mainLayout = new QVBoxLayout(centralWidget);
    
    imageLabel = new QLabel("Click 'Load Folder' to select images");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(500, 300);
    mainLayout->addWidget(imageLabel);
    
    fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel("FPS:");
    fpsSpinBox = new QSpinBox();
    fpsSpinBox->setRange(1, 1000);
    fpsSpinBox->setValue(30);
    fpsSpinBox->setSuffix(" fps");
    
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(fpsSpinBox);
    fpsLayout->addStretch();
    mainLayout->addLayout(fpsLayout);
    
    buttonLayout = new QHBoxLayout();
    
    loadButton = new QPushButton("Load Folder");
    playButton = new QPushButton("Play");
    playButton->setEnabled(false);
    
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(playButton);
    
    mainLayout->addLayout(buttonLayout);
    
    setupConnections();
}

void UIController::updateImage(const QImage &image)
{
    if (imageLabel && !image.isNull()) {
        QImage scaledImage = image.scaled(
            imageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        imageLabel->setPixmap(QPixmap::fromImage(scaledImage));
    }
}

void UIController::setImageCount(int count)
{
    if (playButton) {
        playButton->setEnabled(count > 0);
    }
}

void UIController::setPlayButtonEnabled(bool enabled)
{
    if (playButton) {
        playButton->setEnabled(enabled);
    }
}

void UIController::setPlayButtonText(const QString &text)
{
    if (playButton) {
        playButton->setText(text);
    }
}

void UIController::setFPSValue(int fps)
{
    if (fpsSpinBox) {
        fpsSpinBox->setValue(fps);
    }
}

void UIController::showLoadError(const QString &error)
{
    if (imageLabel) {
        imageLabel->setText("Error: " + error);
    }
}

void UIController::updateWindowTitle(int imageCount)
{
    Q_UNUSED(imageCount)
}

void UIController::setupConnections()
{
    if (loadButton) {
        connect(loadButton, &QPushButton::clicked, this, &UIController::loadFolderRequested);
    }
    
    if (playButton) {
        connect(playButton, &QPushButton::clicked, this, &UIController::playPauseRequested);
    }
    
    if (fpsSpinBox) {
        connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &UIController::fpsChanged);
    }
}
