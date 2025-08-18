#include "core/controllers/playback_controller.h"
#include <chrono>

PlaybackController::PlaybackController(QObject *parent)
    : QObject(parent)
    , playing(false)
    , stopRequested(false)
    , fps(30)
    , currentFrame(0)
    , frameCount(0)
{
}

PlaybackController::~PlaybackController()
{
    stop();
}

void PlaybackController::setFPS(int newFps)
{
    if (newFps > 0) {
        fps = newFps;
    }
}

int PlaybackController::getFPS() const
{
    return fps;
}

void PlaybackController::start()
{
    if (frameCount <= 0) return;
    
    if (playing.load()) return;
    
    playing.store(true);
    stopRequested.store(false);
    
    if (!playbackThread.joinable()) {
        playbackThread = std::thread(&PlaybackController::runLoop, this);
    }
    
    emit playbackStarted();
}

void PlaybackController::stop()
{
    if (!playing.load() && !playbackThread.joinable()) return;
    
    stopRequested.store(true);
    playing.store(false);
    
    if (playbackThread.joinable()) {
        playbackThread.join();
    }
    
    currentFrame = 0;
    emit playbackStopped();
    emit frameChanged(0);
}

void PlaybackController::pause()
{
    if (playing.exchange(false)) {
        emit playbackPaused();
    }
}

bool PlaybackController::isPlaying() const
{
    return playing.load();
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
        emit frameChanged(frame);
    }
}

int PlaybackController::getCurrentFrame() const
{
    return currentFrame;
}

void PlaybackController::runLoop()
{
    using namespace std::chrono;
    
    while (!stopRequested.load()) {
        if (playing.load()) {
            if (frameCount > 0) {
                currentFrame++;
                if (currentFrame >= frameCount) {
                    currentFrame = 0;
                }
                emit frameChanged(currentFrame);
            }
            
            auto frameTime = milliseconds(1000 / fps);
            std::this_thread::sleep_for(frameTime);
        } else {
            std::this_thread::sleep_for(milliseconds(16));
        }
    }
}
