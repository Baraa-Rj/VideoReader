#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QString>

class QWidget;

class UIController : public QObject
{
    Q_OBJECT

public:
    explicit UIController(QObject *parent = nullptr);
    ~UIController();

    void setupUI(QWidget *centralWidget);
    void updateImage(const QImage &image);
    void setImageCount(int count);
    void setPlayButtonEnabled(bool enabled);
    void setPlayButtonText(const QString &text);
    void setFPSValue(int fps);
    void showLoadError(const QString &error);
    void updateWindowTitle(int imageCount);

signals:
    void loadFolderRequested();
    void playPauseRequested();
    void fpsChanged(int fps);

private:
    void setupConnections();

    QLabel *imageLabel;
    QPushButton *loadButton;
    QPushButton *playButton;
    QSpinBox *fpsSpinBox;
    QVBoxLayout *mainLayout;
    QHBoxLayout *fpsLayout;
    QHBoxLayout *buttonLayout;
};

#endif
