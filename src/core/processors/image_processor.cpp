#include "image_processor.h"
#include <QDir>
#include <QDirIterator>
#include <opencv2/opencv.hpp>

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
    supportedFormats << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
}

ImageProcessor::~ImageProcessor()
{
    clear();
}

bool ImageProcessor::loadImagesFromFolder(const QString &folderPath)
{
    clear();
    
    QDir folder(folderPath);
    if (!folder.exists()) {
        emit loadError("Selected folder does not exist");
        return false;
    }
    
    QDirIterator iterator(folderPath, supportedFormats, QDir::Files, QDirIterator::Subdirectories);
    
    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        if (isValidImageFile(filePath)) {
            imagePaths.append(filePath);
        }
    }
    
    if (imagePaths.isEmpty()) {
        emit loadError("No valid image files found in the selected folder");
        return false;
    }
    
    imagePaths.sort();
    
    emit imagesLoaded(imagePaths.size());
    return true;
}

QImage ImageProcessor::getImage(int index) const
{
    if (!isValidIndex(index)) {
        return QImage();
    }
    
    try {
        cv::Mat image = cv::imread(imagePaths[index].toStdString());
        if (!image.empty()) {
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            
            QImage qImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
            return qImage.copy();
        }
    } catch (...) {
    }
    
    return QImage();
}

QString ImageProcessor::getImagePath(int index) const
{
    if (isValidIndex(index)) {
        return imagePaths[index];
    }
    return QString();
}

int ImageProcessor::getImageCount() const
{
    return imagePaths.size();
}

bool ImageProcessor::isValidIndex(int index) const
{
    return index >= 0 && index < imagePaths.size();
}

void ImageProcessor::clear()
{
    imagePaths.clear();
}

bool ImageProcessor::isValidImageFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }
    
    QString extension = fileInfo.suffix().toLower();
    return extension == "jpg" || extension == "jpeg" || extension == "png" || extension == "bmp";
}
