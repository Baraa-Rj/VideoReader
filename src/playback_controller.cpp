#include "playback_controller.h"

PlaybackController::PlaybackController(QObject *parent)
    : QObject(parent)
    , fps(30)
    , currentFrame(0)
    , frameCount(0)
    , playing(false)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlaybackController::nextFrame);
}

PlaybackController::~PlaybackController()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

void PlaybackController::setFPS(int newFps)
{
    if (fps != newFps && newFps > 0) {
        fps = newFps;
        updateTimer();
    }
}

int PlaybackController::getFPS() const
{
    return fps;
}

void PlaybackController::start()
{
    if (frameCount > 0 && !playing) {
        playing = true;
        timer->start();
        emit playbackStarted();
    }
}

void PlaybackController::stop()
{
    if (playing) {
        playing = false;
        timer->stop();
        currentFrame = 0;
        emit playbackStopped();
        emit frameChanged(currentFrame);
    }
}

void PlaybackController::pause()
{
    if (playing) {
        playing = false;
        timer->stop();
        emit playbackPaused();
    }
}

bool PlaybackController::isPlaying() const
{
    return playing;
}

void PlaybackController::setFrameCount(int count)
{
    frameCount = count;
    if (currentFrame >= frameCount) {
        currentFrame = 0;
    }
}

void PlaybackController::setCurrentFrame(int frame)
{
    if (frame >= 0 && frame < frameCount) {
        currentFrame = frame;
        emit frameChanged(currentFrame);
    }
}

int PlaybackController::getCurrentFrame() const
{
    return currentFrame;
}

void PlaybackController::nextFrame()
{
    if (frameCount == 0) return;
    
    currentFrame++;
    if (currentFrame >= frameCount) {
        currentFrame = 0;
    }
    
    emit frameChanged(currentFrame);
}

void PlaybackController::updateTimer()
{
    if (fps > 0) {
        int interval = 1000 / fps;
        timer->setInterval(interval);
    }
}
