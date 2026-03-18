#include "ClipboardTextEdit.h"

#include <QImage>
#include <QMimeData>

ClipboardTextEdit::ClipboardTextEdit(QWidget* parent)
    : QTextEdit(parent) {}

void ClipboardTextEdit::insertFromMimeData(const QMimeData* source) {
    if (source && source->hasImage()) {
        const QImage image = qvariant_cast<QImage>(source->imageData());
        if (!image.isNull()) {
            emit imagePasted(image);
            return;
        }
    }
    QTextEdit::insertFromMimeData(source);
}
