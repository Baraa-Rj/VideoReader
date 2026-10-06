// Unit tests for ImageProcessor: folder loading, extension filtering, index
// bounds, and decode behaviour (including the silent-failure path).
#include <QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/processors/image_processor.h"

#include <opencv2/opencv.hpp>

class TestImageProcessor : public QObject
{
    Q_OBJECT

private slots:
    void missingFolderEmitsError();
    void emptyFolderEmitsError();
    void loadsSupportedImages();
    void ignoresUnsupportedExtensions();
    void findsImagesInSubdirectories();
    void indexBoundsAreEnforced();
    void decodesImageWithCorrectChannelOrder();
    void unreadableFileYieldsNullImage();
    void clearResetsState();
    void reloadReplacesPreviousSet();

private:
    // Writes a solid-colour image. Colour is given in BGR, as OpenCV expects.
    static bool writeImage(const QString &path, int b, int g, int r,
                           int width = 8, int height = 6)
    {
        cv::Mat mat(height, width, CV_8UC3, cv::Scalar(b, g, r));
        return cv::imwrite(path.toStdString(), mat);
    }
};

void TestImageProcessor::missingFolderEmitsError()
{
    ImageProcessor processor;
    QSignalSpy errors(&processor, &ImageProcessor::loadError);

    QVERIFY(!processor.loadImagesFromFolder("/nonexistent/path/for/testing"));
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.takeFirst().at(0).toString(),
             QString("Selected folder does not exist"));
    QCOMPARE(processor.getImageCount(), 0);
}

void TestImageProcessor::emptyFolderEmitsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ImageProcessor processor;
    QSignalSpy errors(&processor, &ImageProcessor::loadError);
    QSignalSpy loaded(&processor, &ImageProcessor::imagesLoaded);

    QVERIFY(!processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.takeFirst().at(0).toString(),
             QString("No valid image files found in the selected folder"));
    QCOMPARE(loaded.count(), 0);
    QCOMPARE(processor.getImageCount(), 0);
}

void TestImageProcessor::loadsSupportedImages()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeImage(dir.filePath("a.png"), 10, 20, 30));
    QVERIFY(writeImage(dir.filePath("b.jpg"), 40, 50, 60));
    QVERIFY(writeImage(dir.filePath("c.bmp"), 70, 80, 90));

    ImageProcessor processor;
    QSignalSpy loaded(&processor, &ImageProcessor::imagesLoaded);

    QVERIFY(processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(processor.getImageCount(), 3);
    QCOMPARE(loaded.count(), 1);
    QCOMPARE(loaded.takeFirst().at(0).toInt(), 3);
}

void TestImageProcessor::ignoresUnsupportedExtensions()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeImage(dir.filePath("keep.png"), 1, 2, 3));

    for (const QString &name : {"notes.txt", "clip.gif", "raw.tiff"}) {
        QFile file(dir.filePath(name));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("not an image");
    }

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(processor.getImageCount(), 1);
    QVERIFY(processor.getImagePath(0).endsWith("keep.png"));
}

void TestImageProcessor::findsImagesInSubdirectories()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath("nested/deeper"));
    QVERIFY(writeImage(dir.filePath("top.png"), 1, 1, 1));
    QVERIFY(writeImage(dir.filePath("nested/mid.png"), 2, 2, 2));
    QVERIFY(writeImage(dir.filePath("nested/deeper/low.png"), 3, 3, 3));

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(processor.getImageCount(), 3);
}

void TestImageProcessor::indexBoundsAreEnforced()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeImage(dir.filePath("only.png"), 5, 5, 5));

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));

    QVERIFY(processor.isValidIndex(0));
    QVERIFY(!processor.isValidIndex(-1));
    QVERIFY(!processor.isValidIndex(1));

    QVERIFY(processor.getImage(-1).isNull());
    QVERIFY(processor.getImage(1).isNull());
    QVERIFY(processor.getImagePath(-1).isEmpty());
    QVERIFY(processor.getImagePath(99).isEmpty());
}

void TestImageProcessor::decodesImageWithCorrectChannelOrder()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    // Pure red in BGR terms: blue=0, green=0, red=255. Use PNG to avoid JPEG
    // quantisation.
    QVERIFY(writeImage(dir.filePath("red.png"), 0, 0, 255, 4, 3));

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));

    const QImage image = processor.getImage(0);
    QVERIFY(!image.isNull());
    QCOMPARE(image.width(), 4);
    QCOMPARE(image.height(), 3);
    QCOMPARE(image.format(), QImage::Format_RGB888);
    // The BGR -> RGB conversion must make this read back as red, not blue.
    QCOMPARE(image.pixelColor(0, 0), QColor(255, 0, 0));
    QCOMPARE(image.pixelColor(3, 2), QColor(255, 0, 0));
}

void TestImageProcessor::unreadableFileYieldsNullImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    // Supported extension, garbage content: it is accepted into the path list
    // (filtering is extension-based) but cannot be decoded.
    QFile corrupt(dir.filePath("broken.png"));
    QVERIFY(corrupt.open(QIODevice::WriteOnly));
    corrupt.write("\x89PNG\r\n\x1a\n not really a png");
    corrupt.close();

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(processor.getImageCount(), 1);

    // Documents the current contract: a decode failure is reported only as a
    // null QImage, with no loadError signal.
    QSignalSpy errors(&processor, &ImageProcessor::loadError);
    QVERIFY(processor.getImage(0).isNull());
    QCOMPARE(errors.count(), 0);
}

void TestImageProcessor::clearResetsState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writeImage(dir.filePath("a.png"), 1, 2, 3));

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(dir.path()));
    QCOMPARE(processor.getImageCount(), 1);

    processor.clear();
    QCOMPARE(processor.getImageCount(), 0);
    QVERIFY(!processor.isValidIndex(0));
    QVERIFY(processor.getImage(0).isNull());
}

void TestImageProcessor::reloadReplacesPreviousSet()
{
    QTemporaryDir first;
    QTemporaryDir second;
    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QVERIFY(writeImage(first.filePath("a.png"), 1, 2, 3));
    QVERIFY(writeImage(first.filePath("b.png"), 4, 5, 6));
    QVERIFY(writeImage(second.filePath("c.png"), 7, 8, 9));

    ImageProcessor processor;
    QVERIFY(processor.loadImagesFromFolder(first.path()));
    QCOMPARE(processor.getImageCount(), 2);

    QVERIFY(processor.loadImagesFromFolder(second.path()));
    QCOMPARE(processor.getImageCount(), 1);
    QVERIFY(processor.getImagePath(0).endsWith("c.png"));

    // A failed load must leave nothing behind from the previous set.
    QVERIFY(!processor.loadImagesFromFolder("/nonexistent/path/for/testing"));
    QCOMPARE(processor.getImageCount(), 0);
}

QTEST_MAIN(TestImageProcessor)
#include "tst_image_processor.moc"
