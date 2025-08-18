#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <QStringList>
#include <QImage>
#include <QObject>

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor();

    bool loadImagesFromFolder(const QString &folderPath);
    QImage getImage(int index) const;
    int getImageCount() const;
    QString getImagePath(int index) const;
    bool isValidIndex(int index) const;
    void clear();

signals:
    void imagesLoaded(int count);
    void loadError(const QString &error);

private:
    QStringList imagePaths;
    QStringList supportedFormats;
    
    bool isValidImageFile(const QString &filePath) const;
};

#endif
