#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include <QObject>
#include <QTimer>

class PlaybackController : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackController(QObject *parent = nullptr);
    ~PlaybackController();

    void setFPS(int fps);
    int getFPS() const;
    void start();
    void stop();
    void pause();
    bool isPlaying() const;
    void setFrameCount(int count);
    void setCurrentFrame(int frame);
    int getCurrentFrame() const;

signals:
    void frameChanged(int frameIndex);
    void playbackStarted();
    void playbackStopped();
    void playbackPaused();

private slots:
    void nextFrame();

private:
    QTimer *timer;
    int fps;
    int currentFrame;
    int frameCount;
    bool playing;
    
    void updateTimer();
};

#endif
