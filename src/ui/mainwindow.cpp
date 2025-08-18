#include "mainwindow.h"
#include "image_processor.h"
#include "playback_controller.h"
#include "ui_controller.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Simple Image Player");
    setMinimumSize(600, 400);
    
    imageProcessor = new ImageProcessor(this);
    playbackController = new PlaybackController(this);
    uiController = new UIController(this);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    uiController->setupUI(centralWidget);
    
    setupConnections();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupConnections()
{
    connect(uiController, &UIController::loadFolderRequested, this, &MainWindow::onLoadFolderRequested);
    connect(uiController, &UIController::playPauseRequested, this, &MainWindow::onPlayPauseRequested);
    connect(uiController, &UIController::fpsChanged, this, &MainWindow::onFPSChanged);
    
    connect(imageProcessor, &ImageProcessor::imagesLoaded, this, &MainWindow::onImagesLoaded);
    connect(imageProcessor, &ImageProcessor::loadError, this, &MainWindow::onLoadError);
    
    connect(playbackController, &PlaybackController::frameChanged, this, &MainWindow::onFrameChanged);
    connect(playbackController, &PlaybackController::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(playbackController, &PlaybackController::playbackStopped, this, &MainWindow::onPlaybackStopped);
    connect(playbackController, &PlaybackController::playbackPaused, this, &MainWindow::onPlaybackPaused);
}

void MainWindow::onLoadFolderRequested()
{
    loadFolder();
}

void MainWindow::onPlayPauseRequested()
{
    playPause();
}

void MainWindow::onFPSChanged(int fps)
{
    playbackController->setFPS(fps);
}

void MainWindow::onImagesLoaded(int count)
{
    playbackController->setFrameCount(count);
    uiController->setImageCount(count);
    setWindowTitle(QString("Simple Image Player - %1 images").arg(count));
    
    if (count > 0) {
        QImage firstImage = imageProcessor->getImage(0);
        uiController->updateImage(firstImage);
    }
}

void MainWindow::onLoadError(const QString &error)
{
    uiController->showLoadError(error);
    QMessageBox::warning(this, "Load Error", error);
}

void MainWindow::onFrameChanged(int frameIndex)
{
    QImage image = imageProcessor->getImage(frameIndex);
    uiController->updateImage(image);
}

void MainWindow::onPlaybackStarted()
{
    uiController->setPlayButtonText("Pause");
}

void MainWindow::onPlaybackStopped()
{
    uiController->setPlayButtonText("Play");
}

void MainWindow::onPlaybackPaused()
{
    uiController->setPlayButtonText("Play");
}

void MainWindow::loadFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Folder with Images",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
    );
    
    if (!folderPath.isEmpty()) {
        imageProcessor->loadImagesFromFolder(folderPath);
    }
}

void MainWindow::playPause()
{
    if (imageProcessor->getImageCount() == 0) return;
    
    if (playbackController->isPlaying()) {
        playbackController->pause();
    } else {
        playbackController->start();
    }
}
