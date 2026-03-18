#ifndef KAPRAXIS_UTILS_IMAGESTORE_H_
#define KAPRAXIS_UTILS_IMAGESTORE_H_

#include <QImage>
#include <QString>

namespace ImageStore {
QString imagesDir();
QString saveImage(const QImage& image, const QString& prefix);
QString saveImageFromFile(const QString& sourcePath, const QString& prefix);
bool removeImageFile(const QString& path);
}

#endif  // KAPRAXIS_UTILS_IMAGESTORE_H_
