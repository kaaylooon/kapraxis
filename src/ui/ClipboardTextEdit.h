#ifndef KAPRAXIS_UI_CLIPBOARDTEXTEDIT_H_
#define KAPRAXIS_UI_CLIPBOARDTEXTEDIT_H_

#include <QImage>
#include <QMimeData>
#include <QTextEdit>

class ClipboardTextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit ClipboardTextEdit(QWidget* parent = nullptr);

signals:
    void imagePasted(const QImage& image);

protected:
    void insertFromMimeData(const QMimeData* source) override;
};

#endif  // KAPRAXIS_UI_CLIPBOARDTEXTEDIT_H_
