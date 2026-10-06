// Unit tests for UIController, covering the widget leak when setupUI is given a
// null parent or called twice, plus the public state-setter guards.
#include <QtTest>
#include <QApplication>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QWidget>

#include "ui/ui_controller.h"

class TestUIController : public QObject
{
    Q_OBJECT

private slots:
    void setupUiBuildsWidgetTree();
    void setupUiWithNullParentCreatesNothing();
    void setupUiIsIdempotent();
    void settersBeforeSetupUiAreNoOps();
    void setImageCountControlsPlayButton();
    void playButtonTextIsSettable();
    void fpsSpinBoxEmitsFpsChanged();
    void showLoadErrorUpdatesLabel();
    void updateImageIgnoresNullImage();
};

void TestUIController::setupUiBuildsWidgetTree()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QVERIFY(parent.layout() != nullptr);
    QCOMPARE(parent.findChildren<QPushButton *>().size(), 2);
    QCOMPARE(parent.findChildren<QSpinBox *>().size(), 1);

    QSpinBox *spin = parent.findChild<QSpinBox *>();
    QVERIFY(spin);
    QCOMPARE(spin->value(), 30);
    QCOMPARE(spin->minimum(), 1);
    QCOMPARE(spin->maximum(), 1000);
}

// Regression test: a null parent leaves the layout unparented, so addWidget
// never adopts the children and every widget created here leaks.
void TestUIController::setupUiWithNullParentCreatesNothing()
{
    UIController controller;
    controller.setupUI(nullptr);

    // Must not crash, and the subsequent public calls must stay no-ops.
    controller.setImageCount(5);
    controller.setPlayButtonEnabled(true);
    controller.setPlayButtonText("Pause");
    controller.setFPSValue(60);
    controller.showLoadError("boom");
    controller.updateImage(QImage(4, 4, QImage::Format_RGB888));
    controller.updateWindowTitle(5);

    // A later, valid setup must still work.
    QWidget parent;
    controller.setupUI(&parent);
    QVERIFY(parent.layout() != nullptr);
    QCOMPARE(parent.findChildren<QPushButton *>().size(), 2);
}

// Regression test: a second setupUI used to overwrite the pointers to the first
// widget set (orphaning it) and attach a second layout to the same widget.
void TestUIController::setupUiIsIdempotent()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QLayout *firstLayout = parent.layout();
    QPushButton *firstPlayButton = parent.findChildren<QPushButton *>().value(1);
    QVERIFY(firstLayout);
    QVERIFY(firstPlayButton);

    controller.setupUI(&parent);

    QCOMPARE(parent.layout(), firstLayout);
    QCOMPARE(parent.findChildren<QPushButton *>().size(), 2);

    // The original widgets must still be the ones being driven.
    controller.setPlayButtonText("Pause");
    QCOMPARE(firstPlayButton->text(), QString("Pause"));
}

void TestUIController::settersBeforeSetupUiAreNoOps()
{
    UIController controller;
    controller.setImageCount(3);
    controller.setPlayButtonEnabled(true);
    controller.setPlayButtonText("Pause");
    controller.setFPSValue(120);
    controller.showLoadError("no widgets yet");
    controller.updateImage(QImage(2, 2, QImage::Format_RGB888));
    // Reaching here without a crash is the assertion.
    QVERIFY(true);
}

void TestUIController::setImageCountControlsPlayButton()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QPushButton *playButton = parent.findChildren<QPushButton *>().value(1);
    QVERIFY(playButton);
    QCOMPARE(playButton->text(), QString("Play"));
    QVERIFY(!playButton->isEnabled());

    controller.setImageCount(4);
    QVERIFY(playButton->isEnabled());

    controller.setImageCount(0);
    QVERIFY(!playButton->isEnabled());

    controller.setPlayButtonEnabled(true);
    QVERIFY(playButton->isEnabled());
}

void TestUIController::playButtonTextIsSettable()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QPushButton *playButton = parent.findChildren<QPushButton *>().value(1);
    QVERIFY(playButton);

    controller.setPlayButtonText("Pause");
    QCOMPARE(playButton->text(), QString("Pause"));
    controller.setPlayButtonText("Play");
    QCOMPARE(playButton->text(), QString("Play"));
}

void TestUIController::fpsSpinBoxEmitsFpsChanged()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);
    QSignalSpy spy(&controller, &UIController::fpsChanged);

    QSpinBox *spin = parent.findChild<QSpinBox *>();
    QVERIFY(spin);
    spin->setValue(45);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 45);

    // setFPSValue drives the same widget, so it round-trips the signal too.
    controller.setFPSValue(90);
    QCOMPARE(spin->value(), 90);
}

void TestUIController::showLoadErrorUpdatesLabel()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QLabel *imageLabel = parent.findChildren<QLabel *>().value(0);
    QVERIFY(imageLabel);

    controller.showLoadError("folder is empty");
    QCOMPARE(imageLabel->text(), QString("Error: folder is empty"));
}

void TestUIController::updateImageIgnoresNullImage()
{
    QWidget parent;
    UIController controller;
    controller.setupUI(&parent);

    QLabel *imageLabel = parent.findChildren<QLabel *>().value(0);
    QVERIFY(imageLabel);

    QImage valid(20, 10, QImage::Format_RGB888);
    valid.fill(Qt::green);
    controller.updateImage(valid);
    QVERIFY(!imageLabel->pixmap().isNull());

    // A null image must leave the previous frame in place rather than clearing
    // the label. This documents how a failed decode surfaces in the UI.
    const QPixmap previous = imageLabel->pixmap();
    controller.updateImage(QImage());
    QCOMPARE(imageLabel->pixmap().cacheKey(), previous.cacheKey());
}

QTEST_MAIN(TestUIController)
#include "tst_ui_controller.moc"
