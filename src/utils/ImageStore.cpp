#include "ImageStore.h"

#include <QDir>
#include <QFile>
#include <QImageWriter>
#include <QStandardPaths>
#include <QUuid>

namespace {
constexpr int kMaxImageDim = 1280;
constexpr int kJpegQuality = 80;

QImage scaleForStorage(const QImage& image) {
    if (image.isNull()) {
        return image;
    }
    const int w = image.width();
    const int h = image.height();
    if (w <= kMaxImageDim && h <= kMaxImageDim) {
        return image;
    }
    return image.scaled(kMaxImageDim, kMaxImageDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
}  // namespace

QString ImageStore::imagesDir() {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(baseDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    if (!dir.exists("images")) {
        dir.mkpath("images");
    }
    return dir.filePath("images");
}

QString ImageStore::saveImage(const QImage& image, const QString& prefix) {
    if (image.isNull()) {
        return QString();
    }
    const QImage scaled = scaleForStorage(image);
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const bool hasAlpha = scaled.hasAlphaChannel();
    const QString ext = hasAlpha ? "png" : "jpg";
    const QString fileName = QString("%1_%2.%3").arg(prefix, id, ext);
    const QString filePath = QDir(imagesDir()).filePath(fileName);

    QImageWriter writer(filePath);
    if (!hasAlpha) {
        writer.setFormat("jpg");
        writer.setQuality(kJpegQuality);
    } else {
        writer.setFormat("png");
    }

    if (!writer.write(scaled)) {
        return QString();
    }
    return filePath;
}

QString ImageStore::saveImageFromFile(const QString& sourcePath, const QString& prefix) {
    if (sourcePath.isEmpty()) {
        return QString();
    }
    QImage image(sourcePath);
    return saveImage(image, prefix);
}

bool ImageStore::removeImageFile(const QString& path) {
    if (path.isEmpty()) {
        return false;
    }
    return QFile::remove(path);
}
