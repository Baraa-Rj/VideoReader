#include "core/controllers/playback_controller.h"
#include <chrono>
#include <iostream>

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
    // Do not call stop(): it emits signals, and by the time a child QObject is
    // destroyed its peers may already be gone (QWidget::~QWidget deletes all
    // children before QObject::~QObject severs the connections).
    joinPlaybackThread();
}

void PlaybackController::setFPS(int newFps)
{
    if (newFps > 0) {
        fps.store(newFps);
    }
}

int PlaybackController::getFPS() const
{
    return fps.load();
}

void PlaybackController::start()
{
    if (frameCount.load() <= 0) return;
    
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
    
    joinPlaybackThread();
    
    currentFrame.store(0);
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
    frameCount.store(count);
    if (currentFrame.load() >= count) {
        currentFrame.store(0);
    }
}

void PlaybackController::setCurrentFrame(int frame)
{
    if (frame >= 0 && frame < frameCount.load()) {
        currentFrame.store(frame);
        emit frameChanged(frame);
    }
}

int PlaybackController::getCurrentFrame() const
{
    return currentFrame.load();
}

void PlaybackController::joinPlaybackThread()
{
    stopRequested.store(true);
    playing.store(false);
    
    if (playbackThread.joinable()) {
        playbackThread.join();
    }
}

void PlaybackController::runLoop()
{
    using namespace std::chrono;
    
    auto startTime = steady_clock::now();
    int elapsedSeconds = 0;
    
    while (!stopRequested.load()) {
        if (playing.load()) {
            const int totalFrames = frameCount.load();
            if (totalFrames > 0) {
                int nextFrame = currentFrame.load() + 1;
                if (nextFrame >= totalFrames) {
                    nextFrame = 0;
                }
                currentFrame.store(nextFrame);
                emit frameChanged(nextFrame);
            }
            
            // Never sleep for 0 ms: integer division truncates to zero above
            // 1000 fps, which turns this loop into a core-pegging busy spin.
            const int currentFps = fps.load();
            int intervalMs = currentFps > 0 ? 1000 / currentFps : 1000;
            if (intervalMs < 1) {
                intervalMs = 1;
            }
            auto frameTime = milliseconds(intervalMs);
            std::this_thread::sleep_for(frameTime);
            
            auto currentTime = steady_clock::now();
            auto elapsed = duration_cast<seconds>(currentTime - startTime).count();
            
            if (elapsed > elapsedSeconds) {
                elapsedSeconds = elapsed;
                std::cout << "Elapsed time: " << elapsedSeconds << " seconds" << std::endl;
            }
        } else {
            std::this_thread::sleep_for(milliseconds(16));
        }
    }
}
