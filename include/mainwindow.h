#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSpinBox>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadFolder();
    void playPause();
    void fpsChanged();

private:
    QLabel *imageLabel;
    QPushButton *loadButton;
    QPushButton *playButton;
    QSpinBox *fpsSpinBox;
    
    QStringList imagePaths;      // Image file paths
    int currentIndex;            // Current frame index
    bool isPlaying;              // Playing state
    int fps;                     // Frames per second
    
    // Timer
    QTimer *timer;
    
    // Helper functions
    void setupUI();              // Setup user interface
    void loadImages(const QString &folderPath);
    void showImage(int index);
    void updateButtons();
    void updateTimer();
    void nextFrame();            // Internal function for timer
};

#endif // MAINWINDOW_H
