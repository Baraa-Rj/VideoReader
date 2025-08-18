#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

class ImageProcessor;
class PlaybackController;
class UIController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImagesLoaded(int count);
    void onLoadError(const QString &error);
    void onFrameChanged(int frameIndex);
    void onPlaybackStarted();
    void onPlaybackStopped();
    void onPlaybackPaused();
    void onLoadFolderRequested();
    void onPlayPauseRequested();
    void onFPSChanged(int fps);

private:
    ImageProcessor *imageProcessor;
    PlaybackController *playbackController;
    UIController *uiController;
    
    void setupConnections();
    void loadFolder();
    void playPause();
};

#endif
