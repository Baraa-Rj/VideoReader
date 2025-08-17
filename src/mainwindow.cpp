#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentIndex(0)
    , isPlaying(false)
    , fps(10)
{
    setWindowTitle("Simple Image Player");
    setMinimumSize(600, 400);
    
    setupUI();
    
    // Create timer for auto-play
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::nextFrame);
    updateTimer();
}

MainWindow::~MainWindow()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // Image display
    imageLabel = new QLabel("Click 'Load Folder' to select images");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(500, 300);
    imageLabel->setStyleSheet("QLabel { border: 2px solid gray; background-color: lightgray; }");
    layout->addWidget(imageLabel);
    
    // FPS control
    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel("FPS:");
    fpsSpinBox = new QSpinBox();
    fpsSpinBox->setRange(1, 60);
    fpsSpinBox->setValue(fps);
    fpsSpinBox->setSuffix(" fps");
    connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::fpsChanged);
    
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(fpsSpinBox);
    fpsLayout->addStretch();
    layout->addLayout(fpsLayout);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    loadButton = new QPushButton("Load Folder");
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadFolder);
    
    playButton = new QPushButton("Play");
    playButton->setEnabled(false);
    connect(playButton, &QPushButton::clicked, this, &MainWindow::playPause);
    
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(playButton);
    
    layout->addLayout(buttonLayout);
}

void MainWindow::loadFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Folder with Images",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
    );
    
    if (!folderPath.isEmpty()) {
        loadImages(folderPath);
    }
}

void MainWindow::loadImages(const QString &folderPath)
{
    QDir folder(folderPath);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
    
    imagePaths = folder.entryList(filters, QDir::Files, QDir::Name);
    
    if (imagePaths.isEmpty()) {
        QMessageBox::warning(this, "No Images", "No image files found in the selected folder.");
        return;
    }
    
    // Convert to full paths
    for (int i = 0; i < imagePaths.size(); ++i) {
        imagePaths[i] = folder.absoluteFilePath(imagePaths[i]);
    }
    
    currentIndex = 0;
    showImage(0);
    updateButtons();
    
    setWindowTitle(QString("Simple Image Player - %1 images").arg(imagePaths.size()));
}

void MainWindow::showImage(int index)
{
    if (index < 0 || index >= imagePaths.size()) {
        return;
    }
    
    try {
        cv::Mat image = cv::imread(imagePaths[index].toStdString());
        if (!image.empty()) {
            // Convert BGR to RGB
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            
            // Convert to QImage
            QImage qImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
            
            // Scale to fit label
            QImage scaledImage = qImage.scaled(
                imageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            
            imageLabel->setPixmap(QPixmap::fromImage(scaledImage));
        }
    } catch (...) {
        imageLabel->setText("Failed to load image");
    }
}

void MainWindow::playPause()
{
    if (imagePaths.isEmpty()) return;
    
    if (isPlaying) {
        timer->stop();
        playButton->setText("Play");
        isPlaying = false;
    } else {
        timer->start();
        playButton->setText("Pause");
        isPlaying = true;
    }
}

void MainWindow::nextFrame()
{
    if (imagePaths.isEmpty()) return;
    
    currentIndex++;
    if (currentIndex >= imagePaths.size()) {
        currentIndex = 0; // Loop back
    }
    
    showImage(currentIndex);
}

void MainWindow::fpsChanged()
{
    fps = fpsSpinBox->value();
    updateTimer();
}

void MainWindow::updateTimer()
{
    int interval = 1000 / fps; // Convert FPS to milliseconds
    timer->setInterval(interval);
}

void MainWindow::updateButtons()
{
    bool hasImages = !imagePaths.isEmpty();
    playButton->setEnabled(hasImages);
}
