#pragma once

#include <QTextEdit>
#include <QImage>

class ClipboardTextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit ClipboardTextEdit(QWidget* parent = nullptr);

signals:
    void imagePasted(const QImage& image);

protected:
    void insertFromMimeData(const QMimeData* source) override;
};
