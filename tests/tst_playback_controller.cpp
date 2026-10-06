// Unit tests for PlaybackController, covering the shutdown use-after-free, the
// data race on the shared playback state, and the high-FPS busy spin.
#include <QtTest>
#include <QCoreApplication>
#include <QObject>
#include <QSignalSpy>

#include "core/controllers/playback_controller.h"

#include <atomic>
#include <thread>

namespace {

// Stands in for MainWindow: owns a PlaybackController as a QObject child and
// connects a slot to frameChanged, exactly as MainWindow::setupConnections does.
class OwnerWidgetLike : public QObject
{
    Q_OBJECT
public:
    explicit OwnerWidgetLike(QObject *parent = nullptr) : QObject(parent)
    {
        controller = new PlaybackController(this);
        connect(controller, &PlaybackController::frameChanged,
                this, &OwnerWidgetLike::onFrameChanged);
        connect(controller, &PlaybackController::playbackStopped,
                this, &OwnerWidgetLike::onPlaybackStopped);
    }

    PlaybackController *controller = nullptr;
    int signalsAfterTeardown = 0;
    bool tornDown = false;

private slots:
    void onFrameChanged(int) { if (tornDown) ++signalsAfterTeardown; }
    void onPlaybackStopped() { if (tornDown) ++signalsAfterTeardown; }
};

// Records frameChanged on the main thread. QSignalSpy connects directly, so a
// spy on a signal emitted by the playback thread would itself race; a queued
// connection delivers each emission through the main event loop instead.
class FrameRecorder : public QObject
{
    Q_OBJECT
public:
    explicit FrameRecorder(PlaybackController *controller)
    {
        connect(controller, &PlaybackController::frameChanged,
                this, &FrameRecorder::record, Qt::QueuedConnection);
    }

    QList<int> frames;

private slots:
    void record(int frameIndex) { frames.append(frameIndex); }
};

} // namespace

class TestPlaybackController : public QObject
{
    Q_OBJECT

private slots:
    void initialStateIsStoppedAt30Fps();
    void setFpsIgnoresNonPositiveValues();
    void setFrameCountResetsOutOfRangeCurrentFrame();
    void setCurrentFrameRejectsOutOfRangeIndices();
    void startDoesNothingWithoutFrames();
    void startAdvancesFramesAndEmits();
    void pauseStopsAdvancingAndKeepsPosition();
    void stopResetsToFirstFrameAndEmits();
    void stopWithoutStartIsSilent();
    void destructorEmitsNoSignals();
    void highFpsDoesNotBusySpin();
    void concurrentSetFpsWhileRunningIsSafe();
};

void TestPlaybackController::initialStateIsStoppedAt30Fps()
{
    PlaybackController controller;
    QCOMPARE(controller.getFPS(), 30);
    QCOMPARE(controller.getCurrentFrame(), 0);
    QVERIFY(!controller.isPlaying());
}

void TestPlaybackController::setFpsIgnoresNonPositiveValues()
{
    PlaybackController controller;

    controller.setFPS(60);
    QCOMPARE(controller.getFPS(), 60);

    controller.setFPS(0);
    QCOMPARE(controller.getFPS(), 60);

    controller.setFPS(-5);
    QCOMPARE(controller.getFPS(), 60);

    controller.setFPS(1);
    QCOMPARE(controller.getFPS(), 1);
}

void TestPlaybackController::setFrameCountResetsOutOfRangeCurrentFrame()
{
    PlaybackController controller;

    controller.setFrameCount(10);
    controller.setCurrentFrame(7);
    QCOMPARE(controller.getCurrentFrame(), 7);

    // Shrinking the set below the current position must rewind to the start.
    controller.setFrameCount(3);
    QCOMPARE(controller.getCurrentFrame(), 0);

    // Growing it must leave the position alone.
    controller.setCurrentFrame(2);
    controller.setFrameCount(20);
    QCOMPARE(controller.getCurrentFrame(), 2);
}

void TestPlaybackController::setCurrentFrameRejectsOutOfRangeIndices()
{
    PlaybackController controller;
    controller.setFrameCount(5);
    QSignalSpy spy(&controller, &PlaybackController::frameChanged);

    controller.setCurrentFrame(-1);
    QCOMPARE(controller.getCurrentFrame(), 0);
    QCOMPARE(spy.count(), 0);

    controller.setCurrentFrame(5);
    QCOMPARE(controller.getCurrentFrame(), 0);
    QCOMPARE(spy.count(), 0);

    controller.setCurrentFrame(4);
    QCOMPARE(controller.getCurrentFrame(), 4);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 4);
}

void TestPlaybackController::startDoesNothingWithoutFrames()
{
    PlaybackController controller;
    QSignalSpy started(&controller, &PlaybackController::playbackStarted);

    controller.start();

    QVERIFY(!controller.isPlaying());
    QCOMPARE(started.count(), 0);
}

void TestPlaybackController::startAdvancesFramesAndEmits()
{
    PlaybackController controller;
    QSignalSpy started(&controller, &PlaybackController::playbackStarted);
    FrameRecorder recorder(&controller);

    controller.setFrameCount(3);
    controller.setFPS(200);
    controller.start();

    QVERIFY(controller.isPlaying());
    QCOMPARE(started.count(), 1);

    QTRY_VERIFY_WITH_TIMEOUT(recorder.frames.size() >= 5, 2000);
    controller.stop();

    // Frames must stay inside the valid range while looping.
    for (int index : recorder.frames) {
        QVERIFY(index >= 0);
        QVERIFY(index < 3);
    }
}

void TestPlaybackController::pauseStopsAdvancingAndKeepsPosition()
{
    PlaybackController controller;
    QSignalSpy paused(&controller, &PlaybackController::playbackPaused);

    controller.setFrameCount(100);
    controller.setFPS(200);
    controller.start();
    QTest::qWait(100);

    controller.pause();
    QCOMPARE(paused.count(), 1);
    QVERIFY(!controller.isPlaying());

    // pause() keeps the position, unlike stop().
    const int frameAtPause = controller.getCurrentFrame();
    QTest::qWait(150);
    QCOMPARE(controller.getCurrentFrame(), frameAtPause);

    // Pausing again must not emit a second time.
    controller.pause();
    QCOMPARE(paused.count(), 1);

    controller.stop();
}

void TestPlaybackController::stopResetsToFirstFrameAndEmits()
{
    PlaybackController controller;
    controller.setFrameCount(50);
    controller.setFPS(200);
    controller.start();
    QTest::qWait(80);

    QSignalSpy stopped(&controller, &PlaybackController::playbackStopped);
    FrameRecorder recorder(&controller);

    controller.stop();
    QCoreApplication::processEvents();

    QVERIFY(!controller.isPlaying());
    QCOMPARE(controller.getCurrentFrame(), 0);
    QCOMPARE(stopped.count(), 1);
    // stop() announces the rewind; it is the last frame reported.
    QVERIFY(!recorder.frames.isEmpty());
    QCOMPARE(recorder.frames.last(), 0);
}

void TestPlaybackController::stopWithoutStartIsSilent()
{
    PlaybackController controller;
    QSignalSpy stopped(&controller, &PlaybackController::playbackStopped);
    QSignalSpy frames(&controller, &PlaybackController::frameChanged);

    controller.stop();
    controller.stop();

    QCOMPARE(stopped.count(), 0);
    QCOMPARE(frames.count(), 0);
}

// Regression test for the shutdown use-after-free: ~PlaybackController used to
// call stop(), which emits frameChanged/playbackStopped. Because
// QWidget::~QWidget deletes all children before QObject::~QObject severs the
// connections, those signals reached slots on a half-destroyed parent that then
// dereferenced already-freed siblings.
void TestPlaybackController::destructorEmitsNoSignals()
{
    OwnerWidgetLike owner;
    owner.controller->setFrameCount(10);
    owner.controller->setFPS(200);
    owner.controller->start();
    QTest::qWait(80);

    // Emissions from the destructor happen on this thread, so they reach the
    // owner's slots synchronously; frames queued earlier by the playback thread
    // are not delivered because no events are processed here.
    owner.tornDown = true;
    delete owner.controller;
    owner.controller = nullptr;

    QCOMPARE(owner.signalsAfterTeardown, 0);
}

// Regression test for the busy spin: 1000 / fps truncates to 0 above 1000 fps,
// which removed the sleep entirely. Measured ~351,000 frames per 200 ms before
// the fix; the 1 ms floor caps it near 1000 per second.
void TestPlaybackController::highFpsDoesNotBusySpin()
{
    PlaybackController controller;
    controller.setFrameCount(10000000);
    controller.setFPS(5000);
    controller.start();
    QTest::qWait(200);
    const int advanced = controller.getCurrentFrame();
    controller.stop();

    QVERIFY2(advanced > 0, "playback should still advance at high fps");
    QVERIFY2(advanced < 2000,
             qPrintable(QString("advanced %1 frames in 200ms; the 1ms sleep "
                                "floor should cap this near 200").arg(advanced)));
}

// Exercises the data race ThreadSanitizer reported on fps/currentFrame/
// frameCount. It passes either way without a sanitizer, but makes the race
// reproducible under -fsanitize=thread.
void TestPlaybackController::concurrentSetFpsWhileRunningIsSafe()
{
    PlaybackController controller;
    controller.setFrameCount(1000);
    controller.setFPS(500);
    controller.start();

    std::atomic<int> observed{0};
    std::thread hammer([&controller, &observed] {
        for (int i = 0; i < 2000; ++i) {
            controller.setFPS(1 + (i % 500));
            observed.store(controller.getCurrentFrame());
            controller.getFPS();
        }
    });
    hammer.join();

    QVERIFY(controller.getFPS() > 0);
    QVERIFY(observed.load() >= 0);
    controller.stop();
    QCOMPARE(controller.getCurrentFrame(), 0);
}

QTEST_MAIN(TestPlaybackController)
#include "tst_playback_controller.moc"
