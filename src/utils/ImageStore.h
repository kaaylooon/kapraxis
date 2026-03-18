#pragma once

#include <QImage>
#include <QString>

namespace ImageStore {
QString imagesDir();
QString saveImage(const QImage& image, const QString& prefix);
QString saveImageFromFile(const QString& sourcePath, const QString& prefix);
bool removeImageFile(const QString& path);
}
