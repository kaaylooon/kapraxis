#include "QuestoesPage.h"

#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEasingCurve>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

#include "../repo/questao_repo_sqlite.h"
#include "../utils/ImageStore.h"
#include "ClipboardTextEdit.h"

namespace krepo = kapraxis::repo;

static QString tagChipStyle(const QString& tag) {
    const uint h = qHash(tag.toLower());
    const int hue = static_cast<int>(h % 360);
    const QColor border = QColor::fromHsl(hue, 120, 120);
    const QColor text = QColor::fromHsl(hue, 180, 140);
    return QString("border:1px solid %1;color:%2;font-weight:700;").arg(border.name(), text.name());
}
QString QuestoesPage::buildTagsHtml(const QStringList& tags) {
    if (tags.isEmpty()) {
        return "<span style='padding: 6px 14px; border-radius: 18px; border: 1px solid #3e3e3e; "
               "color: #cfcfcf; font-weight:700;'>" + tr("No tags") + "</span>";
    }

    QString html;
    for (const auto& tag : tags) {
        const QString style = tagChipStyle(tag);
        html += QString(
                    "<span style='padding: 6px 14px; border-radius: 18px; %1 margin-right: 8px; "
                    "margin-bottom: 6px; display: inline-block;'>%2</span>")
                    .arg(style, tag.toHtmlEscaped());
    }
    return html;
}

static QLocale localePtBr() {
    return QLocale(QLocale::Portuguese, QLocale::Brazil);
}

QString QuestoesPage::groupKeyFor(const QDateTime& dt, const QString& mode) {
    const QLocale loc = localePtBr();
    const QDate date = dt.date();
    if (mode == "day") {
        return loc.toString(date, "dddd");
    }
    if (mode == "week") {
        int weekYear = 0;
        const int week = date.weekNumber(&weekYear);
        return tr("Week %1").arg(week);
    }
    if (mode == "month") {
        return loc.toString(date, "MMMM");
    }
    if (mode == "year") {
        return loc.toString(date, "yyyy");
    }
    return QString();
}

static QListWidgetItem* addGroupHeader(QListWidget* list, const QString& title) {
    auto* item = new QListWidgetItem(list);
    item->setFlags(Qt::ItemIsEnabled);
    item->setData(Qt::UserRole + 99, true);
    item->setData(Qt::UserRole + 100, "group-header");


    auto* headerWidget = new QWidget();
    headerWidget->setObjectName("groupHeaderWidget");
    headerWidget->setProperty("class", "group-header");

    auto* layout = new QHBoxLayout(headerWidget);
    layout->setContentsMargins(12, 8, 12, 8);

    auto* label = new QLabel(title);
    label->setObjectName("groupHeaderLabel");
    label->setProperty("class", "group-header-label");

    QFont font = label->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 2);
    label->setFont(font);

    layout->addWidget(label);
    layout->addStretch();

    headerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    item->setSizeHint(QSize(list->viewport()->width(), 40));
    list->addItem(item);
    list->setItemWidget(item, headerWidget);

    return item;
}

static void clearLayout(QVBoxLayout* layout) {
    while (layout->count() > 0) {
        QLayoutItem* item = layout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            delete item->layout();
        }
        delete item;
    }
}

static void renderImageListFullWidth(QScrollArea* scroll, QHBoxLayout* layout,
                                     const QStringList& paths) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    if (paths.isEmpty()) {
        return;
    }

    const int targetHeight = 300;
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    for (const auto& path : paths) {
        QPixmap pix(path);
        if (pix.isNull()) {
            continue;
        }

        auto* container = new QWidget();
        container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto* containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(4, 4, 4, 4);
        containerLayout->setSpacing(0);

        auto* label = new QLabel();
        label->setAlignment(Qt::AlignCenter);

        const QPixmap scaled = pix.scaledToHeight(targetHeight, Qt::SmoothTransformation);
        label->setPixmap(scaled);
        label->setFixedHeight(targetHeight);
        label->setFixedWidth(scaled.width());

        containerLayout->addStretch();
        containerLayout->addWidget(label);
        containerLayout->addStretch();
        layout->addWidget(container);
    }

    layout->addStretch();

    QTimer::singleShot(0, [scroll, layout]() {
        int totalWidth = 0;
        for (int i = 0; i < layout->count(); ++i) {
            auto* item = layout->itemAt(i);
            if (item && item->widget()) {
                totalWidth += item->widget()->width() + layout->spacing();
            }
        }

        if (totalWidth <= scroll->viewport()->width()) {
            scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else {
            scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
    });
}

static QString buildKeepTextFromList(const QJsonArray& listContent) {
    QStringList lines;
    for (const auto& entry : listContent) {
        if (entry.isObject()) {
            const QJsonObject obj = entry.toObject();
            const QString text = obj.value("text").toString();
            if (!text.isEmpty()) {
                const bool checked = obj.value("isChecked").toBool(false);
                lines.append(QString("%1 %2").arg(checked ? "[x]" : "[ ]", text));
            }
        } else if (entry.isString()) {
            const QString text = entry.toString();
            if (!text.isEmpty()) {
                lines.append(text);
            }
        }
    }
    return lines.join("\n");
}

static QStringList keepLabelsToTags(const QJsonArray& labels) {
    QStringList tags;
    for (const auto& entry : labels) {
        if (!entry.isObject()) continue;
        const QJsonObject obj = entry.toObject();
        const QString name = obj.value("name").toString();
        if (!name.isEmpty()) {
            tags.append(name);
        }
    }
    return tags;
}

static QDateTime keepTimestampToDateTime(const QJsonObject& obj) {
    const qint64 created =
        static_cast<qint64>(obj.value("createdTimestampUsec").toVariant().toLongLong());
    const qint64 edited =
        static_cast<qint64>(obj.value("userEditedTimestampUsec").toVariant().toLongLong());
    const qint64 usec = edited > 0 ? edited : created;
    if (usec <= 0) {
        return QDateTime::currentDateTime();
    }
    return QDateTime::fromMSecsSinceEpoch(usec / 1000);
}

static void configureImageList(QListWidget* list) {
    list->setViewMode(QListView::IconMode);
    list->setResizeMode(QListView::Adjust);
    list->setMovement(QListView::Static);
    list->setSpacing(6);
    list->setIconSize(QSize(96, 96));
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setUniformItemSizes(true);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setMaximumHeight(130);
}

static void addImageItem(QListWidget* list, const QString& path, bool showText) {
    if (path.isEmpty()) {
        return;
    }
    QPixmap pix(path);
    QIcon icon;
    if (!pix.isNull()) {
        icon = QIcon(pix.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    const QString name = QFileInfo(path).fileName();
    auto* item = new QListWidgetItem(icon, showText ? name : QString());
    item->setData(Qt::UserRole, path);
    item->setToolTip(path);
    list->addItem(item);
}

static void preencherListaImagens(QListWidget* list, const QStringList& paths, bool showText) {
    list->clear();
    for (const auto& path : paths) {
        addImageItem(list, path, showText);
    }
}

static QPixmap loadThumbnail(const QString& path, int size) {
    if (path.isEmpty()) return QPixmap();
    const QString key = QString("thumb:%1:%2").arg(path).arg(size);
    QPixmap cached;
    if (QPixmapCache::find(key, &cached)) {
        return cached;
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    reader.setScaledSize(QSize(size, size));
    QImage img = reader.read();
    if (img.isNull()) {
        QPixmap fallback(path);
        if (!fallback.isNull()) {
            cached = fallback.scaled(size, size, Qt::KeepAspectRatioByExpanding,
                                     Qt::SmoothTransformation);
            QPixmapCache::insert(key, cached);
            return cached;
        }
        return QPixmap();
    }
    cached = QPixmap::fromImage(img).scaled(size, size, Qt::KeepAspectRatioByExpanding,
                                            Qt::SmoothTransformation);
    QPixmapCache::insert(key, cached);
    return cached;
}

static QStringList coletarPaths(QListWidget* list) {
    QStringList paths;
    for (int i = 0; i < list->count(); ++i) {
        auto* item = list->item(i);
        paths.append(item->data(Qt::UserRole).toString());
    }
    return paths;
}

QuestoesPage::QuestoesPage(QWidget* parent) : QWidget(parent) {
    repo = new krepo::QuestaoRepoSQLite;

    setupShortcuts();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    autoSaveTimer->setInterval(1000);

    auto* searchLayout = new QHBoxLayout();
    txtBusca = new QLineEdit();
    txtBusca->setObjectName("txtBusca");
    txtBusca->setPlaceholderText(tr("Search..."));
    txtBusca->setClearButtonEnabled(true);
    txtBusca->setMinimumWidth(800);

    searchLayout->addStretch();
    searchLayout->addWidget(txtBusca);
    searchLayout->addStretch();
    mainLayout->addLayout(searchLayout);

    auto* filtersLayout = new QHBoxLayout();

    comboFilterTag = new QComboBox();
    comboFilterTag->setObjectName("comboGroup");
    comboFilterTag->addItem(tr("Show All"), "");
    comboFilterTag->addItem(tr("Without content"), "sem_resposta");
    comboFilterTag->addItem(tr("With content"), "com_resposta");
    comboFilterTag->addItem(tr("Without tags"), "sem_tags");
    comboFilterTag->addItem(tr("With tags"), "com_tags");
    comboFilterTag->addItem(tr("Today"), "hoje");
    comboFilterTag->addItem(tr("Last 7 days"), "ultimos_7");
    comboFilterTag->addItem(tr("Last 30 days"), "ultimos_30");
    comboFilterTag->setEditable(false);

    comboGroupBy = new QComboBox();
    comboGroupBy->setObjectName("comboGroup");
    comboGroupBy->addItem(tr("No grouping"), "");
    comboGroupBy->addItem(tr("Group by day"), "day");
    comboGroupBy->addItem(tr("Group by week"), "week");
    comboGroupBy->addItem(tr("Group by month"), "month");
    comboGroupBy->addItem(tr("Group by year"), "year");
    comboGroupBy->setEditable(false);

    filtersLayout->addStretch();
    filtersLayout->addWidget(comboFilterTag);
    filtersLayout->addWidget(comboGroupBy);
    filtersLayout->addStretch();
    mainLayout->addLayout(filtersLayout);

    lista = new QListWidget();
    lista->setObjectName("lista");
    lista->setSelectionMode(QAbstractItemView::SingleSelection);
    lista->setViewMode(QListView::ListMode);
    lista->setResizeMode(QListView::Adjust);
    lista->setSpacing(8);
    lista->setWordWrap(true);
    lista->setUniformItemSizes(false);
    lista->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    lista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lista->setDropIndicatorShown(false);
    lista->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(lista, 1);

    setupDetailsPanel();

    contentStack = new QStackedWidget();
    listPage = new QWidget();
    listPage->setObjectName("questoesListPage");
    detailPage = new QWidget();
    detailPage->setObjectName("questoesDetailPage");

    auto* listPageLayout = new QVBoxLayout(listPage);
    listPageLayout->setContentsMargins(0, 0, 0, 0);
    listPageLayout->setSpacing(0);
    listPageLayout->addWidget(lista);

    btnVoltarLista = new QPushButton(tr("Return"));
    btnVoltarLista->setObjectName("btnVoltarLista");
    btnVoltarLista->setCursor(Qt::PointingHandCursor);
    btnVoltarLista->setFixedWidth(120);

    auto* detailPageLayout = new QVBoxLayout(detailPage);
    detailPageLayout->setSpacing(10);
    auto* detailHeader = new QHBoxLayout();
    detailHeader->addWidget(btnVoltarLista, 0, Qt::AlignLeft);
    detailHeader->addStretch();
    detailPageLayout->addLayout(detailHeader);
    detailPageLayout->addWidget(detailsPanelGroup, 1);

    contentStack->addWidget(listPage);
    contentStack->addWidget(detailPage);

    QLayoutItem* oldLayout = mainLayout->takeAt(mainLayout->indexOf(lista));
    delete oldLayout;
    mainLayout->addWidget(contentStack, 1);

    connect(txtBusca, &QLineEdit::textChanged, this, &QuestoesPage::buscarQuestoes);
    connect(lista, &QListWidget::itemClicked, this, &QuestoesPage::mostrarDetalhes);
    connect(lista, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem*) {
                if (!current) {
                    detailsPanelGroup->setVisible(false);
                    contentStack->setCurrentWidget(listPage);
                    return;
                }
                detailsPanelGroup->setVisible(true);
                mostrarDetalhes(current);
            });
    connect(comboFilterTag, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QuestoesPage::filtrarQuestoes);
    connect(comboGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QuestoesPage::filtrarQuestoes);

    connect(txtEnunciado, &QTextEdit::textChanged, [this]() {
        if (!lista->currentItem()) return;
        autoSaveTimer->start();
    });

    connect(txtResposta, &QTextEdit::textChanged, [this]() {
        if (!lista->currentItem()) return;
        autoSaveTimer->start();
    });

    connect(autoSaveTimer, &QTimer::timeout, this, &QuestoesPage::salvarAlteracoes);

    connect(qApp, &QApplication::focusChanged, [this](QWidget* old, QWidget* now) {
        Q_UNUSED(now)
        if (old == txtEnunciado || old == txtResposta) {
            if (autoSaveTimer->isActive()) {
                autoSaveTimer->stop();
                salvarAlteracoes();
            }
        }
    });


    connect(lista->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int) { carregarMais(); });
    connect(btnVoltarLista, &QPushButton::clicked,
            [this]() { contentStack->setCurrentWidget(listPage); });

    setupCampoAdicionalConnections();
    setupImagePasteConnections();

    detailsPanelGroup->setVisible(false);
    contentStack->setCurrentWidget(listPage);
    recarregar();
}

void QuestoesPage::setupDetailsPanel() {
    detailsPanelGroup = new QGroupBox();
    detailsPanelGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* detailsPanelLayout = new QVBoxLayout(detailsPanelGroup);
    detailsPanelLayout->setSpacing(15);

    // Statement group
    auto* enunciadoGroup = new QGroupBox();
    auto* enunciadoLayout = new QVBoxLayout(enunciadoGroup);
    txtEnunciado = new ClipboardTextEdit();
    txtEnunciado->setPlaceholderText(tr("Type the statement..."));
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    txtEnunciado->setMinimumHeight(120);
    enunciadoLayout->addWidget(txtEnunciado);
    detailsPanelLayout->addWidget(enunciadoGroup);

    imagensEnunciadoGroup = new QGroupBox();
    imagensEnunciadoGroup->setObjectName("imagensGroup");
    auto* imagensEnunciadoLayout = new QVBoxLayout(imagensEnunciadoGroup);
    scrollImagensEnunciado = new QScrollArea();
    scrollImagensEnunciado->setWidgetResizable(true);
    scrollImagensEnunciado->setFrameShape(QFrame::NoFrame);
    scrollImagensEnunciado->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollImagensEnunciado->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    enunciadoImagesContainer = new QWidget();
    enunciadoImagesLayout = new QHBoxLayout(enunciadoImagesContainer);
    enunciadoImagesLayout->setSpacing(12);
    enunciadoImagesLayout->setContentsMargins(8, 8, 8, 8);
    enunciadoImagesLayout->setAlignment(Qt::AlignLeft);

    scrollImagensEnunciado->setWidget(enunciadoImagesContainer);
    imagensEnunciadoLayout->addWidget(scrollImagensEnunciado);
    detailsPanelLayout->addWidget(imagensEnunciadoGroup);
    imagensEnunciadoGroup->setVisible(false);

    // Additional field button
    btnCampoAdicional = new QPushButton(tr("Add additional field"));
    btnCampoAdicional->setObjectName("btnCampoAdicional");
    btnCampoAdicional->setCursor(Qt::PointingHandCursor);
    btnCampoAdicional->setFixedHeight(32);
    detailsPanelLayout->addWidget(btnCampoAdicional);

    // Additional field group
    respostaGroup = new QGroupBox(tr("Additional field"));
    auto* respostaLayout = new QVBoxLayout(respostaGroup);
    txtResposta = new ClipboardTextEdit();
    txtResposta->setPlaceholderText(tr("Type the additional field..."));
    txtResposta->setWordWrapMode(QTextOption::WordWrap);
    txtResposta->setMinimumHeight(100);
    respostaLayout->addWidget(txtResposta);
    detailsPanelLayout->addWidget(respostaGroup);
    respostaGroup->setVisible(false);

    imagensRespostaGroup = new QGroupBox(tr("Additional field attachments"));
    imagensRespostaGroup->setObjectName("imagensGroup");
    auto* imagensRespostaLayout = new QVBoxLayout(imagensRespostaGroup);
    scrollImagensResposta = new QScrollArea();
    scrollImagensResposta->setWidgetResizable(true);
    scrollImagensResposta->setFrameShape(QFrame::NoFrame);
    scrollImagensResposta->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollImagensResposta->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    respostaImagesContainer = new QWidget();
    respostaImagesLayout = new QHBoxLayout(respostaImagesContainer);
    respostaImagesLayout->setSpacing(12);
    respostaImagesLayout->setContentsMargins(8, 8, 8, 8);
    respostaImagesLayout->setAlignment(Qt::AlignLeft);

    scrollImagensResposta->setWidget(respostaImagesContainer);
    imagensRespostaLayout->addWidget(scrollImagensResposta);
    detailsPanelLayout->addWidget(imagensRespostaGroup);
    imagensRespostaGroup->setVisible(false);

    auto* infoGroup = new QGroupBox();
    auto* infoLayout = new QHBoxLayout(infoGroup);
    lblTags = new QLabel(tr("No tags"));
    lblTags->setTextFormat(Qt::RichText);
    lblData = new QLabel("--/--/----");
    lblData->setObjectName("lblData");
    lblInfo = new QLabel("");
    lblInfo->setObjectName("lblData");

    infoLayout->addWidget(lblTags);
    infoLayout->addStretch();
    infoLayout->addWidget(lblData);
    infoLayout->addWidget(lblInfo);
    detailsPanelLayout->addStretch();
    detailsPanelLayout->addWidget(infoGroup);
}

void QuestoesPage::setupCampoAdicionalConnections() {
    connect(btnCampoAdicional, &QPushButton::clicked, [this]() {
        if (!lista->currentItem()) return;
        const int id = lista->currentItem()->data(Qt::UserRole).toInt();
        Questao q = repo->BuscarPorId(id);
        if (q.id == 0) return;

        if (respostaGroup->isVisible()) {
            const int resposta =
                QMessageBox::question(this, tr("Remove additional field"),
                                      tr("Do you want to remove the additional field and delete its content?"),
                                      QMessageBox::Yes | QMessageBox::No);
            if (resposta != QMessageBox::Yes) {
                return;
            }
            q.resposta.clear();
            q.respostaImagens.clear();
            repo->Atualizar(q);
            txtResposta->clear();
            detalheRespostaPaths.clear();
            renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout,
                                     detalheRespostaPaths);
            imagensRespostaGroup->setVisible(false);
            respostaGroup->setVisible(false);
            btnCampoAdicional->setText(tr("Add additional field"));
        } else {
            respostaGroup->setVisible(true);
            btnCampoAdicional->setText(tr("Remove additional field"));
            txtResposta->setFocus();
        }
    });
}

void QuestoesPage::setupImagePasteConnections() {
    auto addImageToCurrent = [this](const QImage& image, const QString& tipo) {
        QListWidgetItem* item = lista->currentItem();
        if (!item) return;
        const QString saved = ImageStore::saveImage(image, tipo);
        if (saved.isEmpty()) {
            QMessageBox::warning(this, tr("Image"), tr("Could not paste the image."));
            return;
        }
        const int id = item->data(Qt::UserRole).toInt();
        Questao q = repo->BuscarPorId(id);
        if (q.id == 0) return;
        if (tipo == "enunciado") {
            q.enunciadoImagens.append(saved);
        } else {
            q.respostaImagens.append(saved);
            if (!respostaGroup->isVisible()) {
                respostaGroup->setVisible(true);
                btnCampoAdicional->setText(tr("Remove additional field"));
            }
        }
        repo->Atualizar(q);
        detalheEnunciadoPaths = q.enunciadoImagens;
        detalheRespostaPaths = q.respostaImagens;
        renderImageListFullWidth(scrollImagensEnunciado, enunciadoImagesLayout,
                                 detalheEnunciadoPaths);
        renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
        imagensEnunciadoGroup->setVisible(!detalheEnunciadoPaths.isEmpty());
        imagensRespostaGroup->setVisible(!detalheRespostaPaths.isEmpty());
    };

    connect(txtEnunciado, &ClipboardTextEdit::imagePasted,
            [addImageToCurrent](const QImage& image) { addImageToCurrent(image, "enunciado"); });
    connect(txtResposta, &ClipboardTextEdit::imagePasted,
            [addImageToCurrent](const QImage& image) { addImageToCurrent(image, "resposta"); });
}

void QuestoesPage::setupShortcuts() {
    atalhoNovaQuestao = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(atalhoNovaQuestao, &QShortcut::activated, this, &QuestoesPage::adicionarQuestao);

    atalhoSalvar = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(atalhoSalvar, &QShortcut::activated, [this]() {
        if (lista->currentItem()) {
            lblInfo->setText(tr("Editing..."));
        }
    });

    atalhoBusca = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(atalhoBusca, &QShortcut::activated, this, &QuestoesPage::focarBusca);
}

void QuestoesPage::adicionarQuestao() {
    Questao q;
    if (!abrirDialogQuestao(q, tr("New question"), tr("Create question"), tr("Save"))) {
        return;
    }
    repo->Salvar(q);
    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
}

void QuestoesPage::editarQuestao() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    Questao questao = repo->BuscarPorId(id);
    if (questao.id == 0) return;
    if (!abrirDialogQuestao(questao, tr("Edit question"), tr("Edit question"), tr("Save changes"))) {
        return;
    }
    repo->Atualizar(questao);
    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
}

void QuestoesPage::excluirQuestao() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    int resposta = QMessageBox::question(this, tr("Confirm deletion"),
                                         tr("Are you sure you want to delete this question?"),
                                         QMessageBox::Yes | QMessageBox::No);
    if (resposta == QMessageBox::Yes) {
        repo->Excluir(id);
        krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
        recarregar();
    }
}

void QuestoesPage::excluirTodasQuestoes() {
    const int resposta = QMessageBox::question(
        this, tr("Delete all questions"),
        tr("Are you sure you want to delete all questions? This action cannot be undone."),
        QMessageBox::Yes | QMessageBox::No);
    if (resposta != QMessageBox::Yes) return;
    repo->ExcluirTodas();
    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
    contentStack->setCurrentWidget(listPage);
}

void QuestoesPage::importarKeepJson() {
    const QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Select Google Keep JSON files"), QString(), tr("JSON (*.json)"));
    if (files.isEmpty()) return;

    int imported = 0;
    int skipped = 0;
    for (const auto& filePath : files) {
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly)) {
            skipped++;
            continue;
        }
        const QByteArray data = file.readAll();
        file.close();
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            skipped++;
            continue;
        }
        const QJsonObject obj = doc.object();
        const QString title = obj.value("title").toString().trimmed();
        QString text = obj.value("textContent").toString().trimmed();
        if (text.isEmpty() && obj.contains("listContent") && obj.value("listContent").isArray()) {
            text = buildKeepTextFromList(obj.value("listContent").toArray()).trimmed();
        }
        const bool hasText = !title.isEmpty() || !text.isEmpty();

        Questao q;
        if (title.isEmpty()) {
            q.enunciado = text;
        } else if (text.isEmpty()) {
            q.enunciado = title;
        } else {
            q.enunciado = QString("%1\n%2").arg(title, text);
        }
        q.resposta = QString();
        q.tags = keepLabelsToTags(obj.value("labels").toArray());
        q.criadaEm = keepTimestampToDateTime(obj);

        if (obj.contains("attachments") && obj.value("attachments").isArray()) {
            const QJsonArray atts = obj.value("attachments").toArray();
            const QString baseDir = QFileInfo(filePath).absolutePath();
            for (const auto& attVal : atts) {
                if (!attVal.isObject()) continue;
                const QJsonObject att = attVal.toObject();
                const QString mime = att.value("mimetype").toString();
                const QString relPath = att.value("filePath").toString();
                if (!mime.startsWith("image/") || relPath.isEmpty()) continue;
                const QString fullPath = QDir(baseDir).filePath(relPath);
                const QString saved = ImageStore::saveImageFromFile(fullPath, "enunciado");
                if (!saved.isEmpty()) {
                    q.enunciadoImagens.append(saved);
                }
            }
        }

        if (!hasText && q.enunciadoImagens.isEmpty()) {
            skipped++;
            continue;
        }
        if (q.enunciado.trimmed().isEmpty()) {
            q.enunciado = q.criadaEm.toString("dd/MM/yyyy HH:mm");
        }
        repo->Salvar(q);
        imported++;
    }

    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
    QMessageBox::information(this, tr("Import completed"),
                             tr("Imported: %1\nSkipped: %2").arg(imported).arg(skipped));
}

void QuestoesPage::salvarAlteracoes() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    Questao q = repo->BuscarPorId(id);
    if (q.id == 0) return;

    bool modificado = false;

    // Salvar enunciado
    QString novoEnunciado = txtEnunciado->toPlainText().trimmed();
    if (q.enunciado != novoEnunciado) {
        q.enunciado = novoEnunciado;
        modificado = true;
    }

    // Salvar resposta (se o grupo estiver visível)
    if (respostaGroup->isVisible()) {
        QString novaResposta = txtResposta->toPlainText();
        if (q.resposta != novaResposta) {
            q.resposta = novaResposta;
            modificado = true;
        }
    }

    if (modificado) {
        repo->Atualizar(q);

        // Atualizar o preview na lista
        item->setData(Qt::UserRole + 1, q.enunciado);
        item->setData(Qt::UserRole + 3, !q.resposta.trimmed().isEmpty());

        // Atualizar o widget do item na lista
        int row = lista->row(item);
        if (row >= 0) {
            // Recriar o item para atualizar a visualização
            // Esta parte é mais complexa, vamos apenas atualizar os dados
            // e depois recarregar a lista? Ou podemos atualizar o texto diretamente

            // Opção mais simples: recarregar a lista mantendo a seleção
            // Mas isso pode ser pesado. Vamos atualizar apenas o card se possível
            QWidget* cardWidget = lista->itemWidget(item);
            if (cardWidget) {
                // Atualizar os textos nos labels
                auto* contentWidget = cardWidget->findChild<QWidget*>("cardContent");
                if (contentWidget) {
                    auto* lblTitulo = contentWidget->findChild<QLabel*>("cardTitle");
                    if (lblTitulo) {
                        QString preview = q.enunciado;
                        preview.replace("\n", " ");
                        preview = preview.simplified();
                        const int maxLength = 80;
                        if (preview.length() > maxLength) {
                            preview = preview.left(maxLength) + "...";
                        }
                        lblTitulo->setText(preview);
                    }

                    auto* lblRespostaPreview = contentWidget->findChild<QLabel*>("cardSubtitle");
                    if (lblRespostaPreview) {
                        QString preview = q.resposta.trimmed();
                        if (preview.isEmpty()) {
                            preview = tr("No response");
                        } else {
                            preview.replace("\n", " ");
                            preview = preview.simplified();
                            const int maxLength = 100;
                            if (preview.length() > maxLength) {
                                preview = preview.left(maxLength) + "...";
                            }
                        }
                        lblRespostaPreview->setText(preview);
                    }
                }
            }
        }

        lblInfo->setText(tr("Saved") + " " + QTime::currentTime().toString("hh:mm:ss"));
        QTimer::singleShot(2000, [this]() {
            if (lblInfo->text().startsWith(tr("Saved"))) {
                lblInfo->clear();
            }
        });
    }
}

void QuestoesPage::focarBusca() {
    txtBusca->setFocus();
    txtBusca->selectAll();
}

void QuestoesPage::destacarItem(QListWidgetItem* item) {
    if (!item) return;
    QTimer::singleShot(100, [item]() { item->setBackground(QColor(254, 239, 195)); });
    QTimer::singleShot(800, [item]() { item->setBackground(QBrush()); });
}

void QuestoesPage::buscarQuestoes(const QString& texto) {
    if (texto.isEmpty()) {
        recarregar();
        return;
    }
    QSignalBlocker blocker(lista->verticalScrollBar());
    loadingItem = nullptr;
    lista->clear();
    renderQueue.clear();
    renderIndex = 0;
    renderGroupMode.clear();
    renderCurrentGroup.clear();
    const auto todasQuestoes = repo->ListarBasicoCached();
    for (const auto& q : todasQuestoes) {
        if (q.enunciado.contains(texto, Qt::CaseInsensitive) ||
            q.resposta.contains(texto, Qt::CaseInsensitive) ||
            q.tags.join(" ").contains(texto, Qt::CaseInsensitive)) {
            renderQueue.append(q);
        }
    }
    iniciarPaginacao(renderQueue, QString());
}

void QuestoesPage::mostrarDetalhes(QListWidgetItem* item) {
    if (!item) return;
    if (item->data(Qt::UserRole + 99).toBool()) return;

    // Bloquear sinais durante o carregamento para evitar auto-save indesejado
    txtEnunciado->blockSignals(true);
    txtResposta->blockSignals(true);

    detailsPanelGroup->setVisible(true);
    contentStack->setCurrentWidget(detailPage);

    int id = item->data(Qt::UserRole).toInt();
    const Questao q = repo->BuscarPorId(id);
    if (q.id == 0) {
        txtEnunciado->blockSignals(false);
        txtResposta->blockSignals(false);
        return;
    }

    txtEnunciado->setText(q.enunciado);
    txtResposta->setText(q.resposta);
    lblTags->setText(buildTagsHtml(q.tags));
    lblData->setText(q.criadaEm.toString("dd/MM/yyyy hh:mm"));

    detalheEnunciadoPaths = q.enunciadoImagens;
    detalheRespostaPaths = q.respostaImagens;
    renderImageListFullWidth(scrollImagensEnunciado, enunciadoImagesLayout, detalheEnunciadoPaths);
    renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
    imagensEnunciadoGroup->setVisible(!detalheEnunciadoPaths.isEmpty());
    imagensRespostaGroup->setVisible(!detalheRespostaPaths.isEmpty());

    const bool hasResposta = !q.resposta.trimmed().isEmpty() || !q.respostaImagens.isEmpty();
    respostaGroup->setVisible(hasResposta);
    btnCampoAdicional->setText(hasResposta ? tr("Remove additional field")
                                           : tr("Add additional field"));

    // Reativar sinais após carregar
    txtEnunciado->blockSignals(false);
    txtResposta->blockSignals(false);
}

void QuestoesPage::filtrarQuestoes(int index) {
    Q_UNUSED(index)
    recarregar();
}

void QuestoesPage::recarregar() {
    QSignalBlocker blocker(lista->verticalScrollBar());
    loadingItem = nullptr;
    lista->clear();

    renderQueue.clear();
    renderIndex = 0;
    renderGroupMode.clear();
    renderCurrentGroup.clear();
    const auto todasQuestoes = repo->ListarBasicoCached();
    QList<Questao> questoesFiltradas;

    QString filtro = comboFilterTag->currentData().toString();
    const QString agrupamento = comboGroupBy->currentData().toString();
    const QDate hoje = QDate::currentDate();

    for (const auto& q : todasQuestoes) {
        bool incluir = true;
        if (filtro == "sem_resposta" && !q.resposta.trimmed().isEmpty()) {
            incluir = false;
        } else if (filtro == "com_resposta" && q.resposta.trimmed().isEmpty()) {
            incluir = false;
        } else if (filtro == "sem_tags" && !q.tags.isEmpty()) {
            incluir = false;
        } else if (filtro == "com_tags" && q.tags.isEmpty()) {
            incluir = false;
        } else if (filtro == "hoje" && q.criadaEm.date() != hoje) {
            incluir = false;
        } else if (filtro == "ultimos_7" && q.criadaEm.date() < hoje.addDays(-6)) {
            incluir = false;
        } else if (filtro == "ultimos_30" && q.criadaEm.date() < hoje.addDays(-29)) {
            incluir = false;
        }
        if (incluir) {
            questoesFiltradas.append(q);
        }
    }

    if (agrupamento.isEmpty()) {
        iniciarPaginacao(questoesFiltradas, QString());
    } else {
        iniciarPaginacao(questoesFiltradas, agrupamento);
    }

    if (lista->count() == 0) {
        detailsPanelGroup->setVisible(false);
        contentStack->setCurrentWidget(listPage);
    }
}

void QuestoesPage::adicionarItemLista(const Questao& q) {
    auto* item = new QListWidgetItem(lista);
    item->setData(Qt::UserRole, q.id);
    item->setData(Qt::UserRole + 1, q.enunciado);
    item->setData(Qt::UserRole + 2, q.tags);
    item->setData(Qt::UserRole + 3, !q.resposta.trimmed().isEmpty());
    item->setData(Qt::UserRole + 99, false);

    QString enunciadoPreview = q.enunciado;
    enunciadoPreview.replace("\n", " ");
    enunciadoPreview.replace("\r", " ");
    enunciadoPreview = enunciadoPreview.simplified();

    const int enunciadoMaxLength = 60;
    if (enunciadoPreview.length() > enunciadoMaxLength) {
        enunciadoPreview = enunciadoPreview.left(enunciadoMaxLength) + "...";
    }

    QString respostaPreview = q.resposta.trimmed();

       if (respostaPreview.isEmpty()) {
           respostaPreview = tr("No response");
       } else {
           respostaPreview.replace("\n", " ");
           respostaPreview.replace("\r", " ");
           respostaPreview = respostaPreview.simplified();

           const int respostaMaxLength = 80;

           if (respostaPreview.length() > respostaMaxLength) {
                respostaPreview = respostaPreview.left(respostaMaxLength) + "...";
            }
;       }
    q.resposta.trimmed().isEmpty() ? tr("No response") : q.resposta;

    auto* card = new QFrame();
    card->setObjectName("listaCard");
    card->setFrameShape(QFrame::NoFrame);
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setAttribute(Qt::WA_Hover, true);


    auto* cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(14, 12, 14, 12);
    cardLayout->setSpacing(12);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* iconContainer = new QWidget();
    iconContainer->setFixedSize(40, 40);
    iconContainer->setAttribute(Qt::WA_TranslucentBackground);
    auto* iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setAlignment(Qt::AlignCenter);

    auto* iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(40, 40);

    QString mainTag = q.tags.isEmpty() ? "?" : q.tags.first();
    QString initial = mainTag.isEmpty() ? "?" : mainTag.left(1).toUpper();

    uint hash = qHash(mainTag.toLower());
    int hue = static_cast<int>(hash % 360);
    QColor bgColor = QColor::fromHsl(hue, 70, 140);
    QColor textColor = QColor::fromHsl(hue, 180, 220);

    iconLabel->setText(initial);
    iconLabel->setStyleSheet(QString(
        "QLabel {"
        "    background-color: %1;"
        "    color: %2;"
        "    border-radius: 20px;"
        "    font-weight: 500;"
        "    font-size: 18px;"
        "}"
    ).arg(bgColor.name(), textColor.name()));

    iconLayout->addWidget(iconLabel);

    auto* contentWidget = new QWidget();
    contentWidget->setObjectName("cardContent");
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(6);

    // Título (enunciado)
    auto* lblTitulo = new QLabel(enunciadoPreview);
    lblTitulo->setObjectName("cardTitle");
    lblTitulo->setTextFormat(Qt::PlainText);
    lblTitulo->setWordWrap(true);
    lblTitulo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lblTitulo->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Subtítulo (resposta preview)
    auto* lblRespostaPreview = new QLabel(respostaPreview);
    lblRespostaPreview->setObjectName("cardSubtitle");
    lblRespostaPreview->setTextFormat(Qt::PlainText);
    lblRespostaPreview->setWordWrap(true);
    lblRespostaPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lblRespostaPreview->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 4, 0, 0);
    bottomRow->setSpacing(8);

    auto* lblDataCard = new QLabel(q.criadaEm.toString("dd/MM/yyyy"));
    lblDataCard->setObjectName("lblData");

    bottomRow->addWidget(lblDataCard, Qt::AlignRight);

    contentLayout->addWidget(lblTitulo);
    contentLayout->addWidget(lblRespostaPreview);
    contentLayout->addLayout(bottomRow);

    // ===== MONTAGEM DO CARD =====
    cardLayout->addWidget(iconContainer);      // Ícone à esquerda
    cardLayout->addWidget(contentWidget, 1);   // Conteúdo expande

    // Ajustar tamanho do card
    card->setMinimumWidth(200);
    card->adjustSize();
    item->setSizeHint(card->sizeHint());
    lista->addItem(item);
    lista->setItemWidget(item, card);
}

void QuestoesPage::atualizarTamanhoItensLista() {
    const int viewportWidth = lista->viewport()->width();
    const int cardWidth = qMax(200, viewportWidth - 32);
    for (int i = 0; i < lista->count(); ++i) {
        auto* item = lista->item(i);
        if (item->data(Qt::UserRole + 99).toBool()) continue;
        auto* card = lista->itemWidget(item);
        if (!card) continue;
        card->setFixedWidth(cardWidth);
        card->adjustSize();
        item->setSizeHint(card->sizeHint());
    }
    lista->doItemsLayout();
}

void QuestoesPage::iniciarPaginacao(const QList<Questao>& list, const QString& agrupamento) {
    renderQueue = list;
    renderIndex = 0;
    renderGroupMode = agrupamento;
    renderCurrentGroup.clear();
    lastScrollValue = 0;
    forceNextLoad = true;

    removerLoadingItem();
    if (renderQueue.isEmpty()) {
        atualizarTamanhoItensLista();
        return;
    }

    loadingItem = new QListWidgetItem(tr("Loading..."));
    loadingItem->setFlags(Qt::NoItemFlags);
    loadingItem->setData(Qt::UserRole + 99, true);
    lista->addItem(loadingItem);
    carregarMais();
}

void QuestoesPage::carregarMais() {
    if (renderQueue.isEmpty()) return;
    if (loadingItem && lista->row(loadingItem) < 0) {
        loadingItem = nullptr;
    }
    if (renderIndex >= renderQueue.size()) {
        removerLoadingItem();
        atualizarTamanhoItensLista();
        return;
    }

    QScrollBar* sb = lista->verticalScrollBar();
    if (sb) {
        const int cur = sb->value();
        const int max = sb->maximum();
        if (!forceNextLoad) {
            if (cur <= lastScrollValue) return;
            if (max > 0 && cur < max - 40) {
                lastScrollValue = cur;
                return;
            }
        }
        lastScrollValue = cur;
    }
    forceNextLoad = false;

    if (loadingItem) {
        const int row = lista->row(loadingItem);
        if (row >= 0) {
            delete lista->takeItem(row);
        }
        loadingItem = nullptr;
    }

    const int batchSize = 20;
    int count = 0;
    while (renderIndex < renderQueue.size() && count < batchSize) {
        const Questao& q = renderQueue[renderIndex++];
        if (!renderGroupMode.isEmpty()) {
            const QString key = groupKeyFor(q.criadaEm, renderGroupMode);
            if (key != renderCurrentGroup) {
                renderCurrentGroup = key;
                addGroupHeader(lista, renderCurrentGroup);
            }
        }
        adicionarItemLista(q);
        count++;
    }

    if (renderIndex < renderQueue.size()) {
        loadingItem = new QListWidgetItem(tr("Loading..."));
        loadingItem->setFlags(Qt::NoItemFlags);
        loadingItem->setData(Qt::UserRole + 99, true);
        lista->addItem(loadingItem);
    }

    if (renderIndex >= renderQueue.size()) {
        removerLoadingItem();
        atualizarTamanhoItensLista();
    }
}

void QuestoesPage::removerLoadingItem() {
    if (!loadingItem) return;
    const int row = lista->row(loadingItem);
    if (row >= 0) {
        delete lista->takeItem(row);
    } else {
        delete loadingItem;
    }
    loadingItem = nullptr;
}

bool QuestoesPage::abrirDialogQuestao(Questao& ioQuestao, const QString& titulo,
                                      const QString& header, const QString& actionLabel) {
    QDialog dialog(this);
    dialog.setWindowTitle(titulo);
    dialog.setModal(true);
    dialog.setMinimumWidth(600);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);

    auto* headerLabel = new QLabel(header);
    headerLabel->setStyleSheet("font-size: 20px; font-weight: 600;");
    layout->addWidget(headerLabel);

    auto* formGroup = new QGroupBox();
    auto* formLayout = new QVBoxLayout(formGroup);
    formLayout->setSpacing(10);

    auto* lblEnunciado = new QLabel(tr("Statement"));
    auto* edtEnunciado = new ClipboardTextEdit();
    edtEnunciado->setPlaceholderText(tr("Type the statement..."));
    edtEnunciado->setText(ioQuestao.enunciado);

    auto* listaEnunciadoImgs = new QListWidget();
    configureImageList(listaEnunciadoImgs);

    auto* enunciadoImgButtons = new QHBoxLayout();
    auto* btnAddEnunciadoImg = new QPushButton(tr("Add image"));
    auto* btnRemEnunciadoImg = new QPushButton(tr("Remove selected"));
    enunciadoImgButtons->addWidget(btnAddEnunciadoImg);
    enunciadoImgButtons->addWidget(btnRemEnunciadoImg);
    enunciadoImgButtons->addStretch();

    auto* btnCampoAdicionalDialog = new QPushButton(tr("Add additional field"));
    btnCampoAdicionalDialog->setObjectName("btnCampoAdicionalDialog");
    btnCampoAdicionalDialog->setCursor(Qt::PointingHandCursor);

    auto* respostaContainer = new QWidget();
    auto* respostaContainerLayout = new QVBoxLayout(respostaContainer);
    respostaContainerLayout->setContentsMargins(0, 0, 0, 0);
    respostaContainerLayout->setSpacing(10);

    auto* lblResposta = new QLabel(tr("Additional field"));
    auto* edtResposta = new ClipboardTextEdit();
    edtResposta->setPlaceholderText(tr("Type the additional field..."));
    edtResposta->setText(ioQuestao.resposta);

    auto* listaRespostaImgs = new QListWidget();
    configureImageList(listaRespostaImgs);

    auto* respostaImgButtons = new QHBoxLayout();
    auto* btnAddRespostaImg = new QPushButton(tr("Add image"));
    auto* btnRemRespostaImg = new QPushButton(tr("Remove selected"));
    respostaImgButtons->addWidget(btnAddRespostaImg);
    respostaImgButtons->addWidget(btnRemRespostaImg);
    respostaImgButtons->addStretch();

    respostaContainerLayout->addWidget(lblResposta);
    respostaContainerLayout->addWidget(edtResposta);
    respostaContainerLayout->addWidget(listaRespostaImgs);
    respostaContainerLayout->addLayout(respostaImgButtons);

    auto* lblTags = new QLabel(tr("Tags"));
    auto* edtTags = new QLineEdit();
    edtTags->setPlaceholderText(tr("Ex.: algebra, functions, geometry"));
    edtTags->setText(ioQuestao.tags.join(", "));

    formLayout->addWidget(lblEnunciado);
    formLayout->addWidget(edtEnunciado);
    formLayout->addWidget(listaEnunciadoImgs);
    formLayout->addLayout(enunciadoImgButtons);
    formLayout->addWidget(btnCampoAdicionalDialog);
    formLayout->addWidget(respostaContainer);
    formLayout->addWidget(lblTags);
    formLayout->addWidget(edtTags);

    layout->addWidget(formGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(actionLabel);
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    layout->addWidget(buttons);

    preencherListaImagens(listaEnunciadoImgs, ioQuestao.enunciadoImagens, true);
    preencherListaImagens(listaRespostaImgs, ioQuestao.respostaImagens, true);

    QSet<QString> newlyAdded;
    const bool hasRespostaInicial =
        !ioQuestao.resposta.trimmed().isEmpty() || !ioQuestao.respostaImagens.isEmpty();
    respostaContainer->setVisible(hasRespostaInicial);
    btnCampoAdicionalDialog->setText(hasRespostaInicial ? tr("Remove additional field")
                                                        : tr("Add additional field"));

    auto addImagePath = [&](QListWidget* list, const QString& path) {
        if (path.isEmpty()) return;
        addImageItem(list, path, true);
        newlyAdded.insert(path);
    };

    auto addImageFromFile = [&](QListWidget* list, const QString& prefix) {
        const QStringList files = QFileDialog::getOpenFileNames(
            &dialog, tr("Select images"), QString(), tr("Images (*.png *.jpg *.jpeg *.bmp)"));
        for (const auto& file : files) {
            const QString saved = ImageStore::saveImageFromFile(file, prefix);
            if (saved.isEmpty()) {
                QMessageBox::warning(&dialog, tr("Image"),
                                     tr("Could not load the selected image."));
                continue;
            }
            addImagePath(list, saved);
        }
    };

    auto addImageFromClipboard = [&](QListWidget* list, const QImage& image,
                                     const QString& prefix) {
        const QString saved = ImageStore::saveImage(image, prefix);
        if (saved.isEmpty()) {
            QMessageBox::warning(&dialog, tr("Image"),
                                 tr("Could not paste the image from clipboard."));
            return;
        }
        addImagePath(list, saved);
    };

    auto removeSelectedImage = [&](QListWidget* list) {
        const auto items = list->selectedItems();
        for (auto* item : items) {
            delete list->takeItem(list->row(item));
        }
    };

    connect(btnAddEnunciadoImg, &QPushButton::clicked,
            [&]() { addImageFromFile(listaEnunciadoImgs, "enunciado"); });
    connect(btnAddRespostaImg, &QPushButton::clicked,
            [&]() { addImageFromFile(listaRespostaImgs, "resposta"); });
    connect(btnRemEnunciadoImg, &QPushButton::clicked,
            [&]() { removeSelectedImage(listaEnunciadoImgs); });
    connect(btnRemRespostaImg, &QPushButton::clicked,
            [&]() { removeSelectedImage(listaRespostaImgs); });
    connect(btnCampoAdicionalDialog, &QPushButton::clicked, [&]() {
        if (respostaContainer->isVisible()) {
            const int resposta =
                QMessageBox::question(&dialog, tr("Remove additional field"),
                                      tr("Do you want to remove the additional field and delete its content?"),
                                      QMessageBox::Yes | QMessageBox::No);
            if (resposta != QMessageBox::Yes) return;
            edtResposta->clear();
            listaRespostaImgs->clear();
            respostaContainer->setVisible(false);
            btnCampoAdicionalDialog->setText(tr("Add additional field"));
        } else {
            respostaContainer->setVisible(true);
            btnCampoAdicionalDialog->setText(tr("Remove additional field"));
            edtResposta->setFocus();
        }
    });
    connect(edtEnunciado, &ClipboardTextEdit::imagePasted, [&](const QImage& image) {
        addImageFromClipboard(listaEnunciadoImgs, image, "enunciado");
    });
    connect(edtResposta, &ClipboardTextEdit::imagePasted, [&](const QImage& image) {
        if (!respostaContainer->isVisible()) {
            respostaContainer->setVisible(true);
            btnCampoAdicionalDialog->setText(tr("Remove additional field"));
        }
        addImageFromClipboard(listaRespostaImgs, image, "resposta");
    });

    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        if (edtEnunciado->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, tr("Validation"), tr("The statement cannot be empty."));
            return;
        }
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        const QStringList finalPaths =
            coletarPaths(listaEnunciadoImgs) + coletarPaths(listaRespostaImgs);
        for (const auto& path : newlyAdded) {
            if (!finalPaths.contains(path)) {
                ImageStore::removeImageFile(path);
            }
        }
        return false;
    }

    const QString tagsRaw = edtTags->text();
    QStringList tags = tagsRaw.split(",", Qt::SkipEmptyParts);
    for (QString& t : tags) {
        t = t.trimmed();
    }
    tags.removeAll(QString());

    const QStringList finalEnunciadoImgs = coletarPaths(listaEnunciadoImgs);
    const QStringList finalRespostaImgs = coletarPaths(listaRespostaImgs);
    const QStringList finalAll = finalEnunciadoImgs + finalRespostaImgs;
    for (const auto& path : newlyAdded) {
        if (!finalAll.contains(path)) {
            ImageStore::removeImageFile(path);
        }
    }

    ioQuestao.enunciado = edtEnunciado->toPlainText().trimmed();
    ioQuestao.resposta = edtResposta->toPlainText().trimmed();
    ioQuestao.tags = tags;
    ioQuestao.enunciadoImagens = finalEnunciadoImgs;
    ioQuestao.respostaImagens = finalRespostaImgs;
    if (!ioQuestao.criadaEm.isValid()) {
        ioQuestao.criadaEm = QDateTime::currentDateTime();
    }
    return true;
}

void QuestoesPage::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete && lista->currentItem()) {
        excluirQuestao();
    }
    QWidget::keyPressEvent(event);
}

void QuestoesPage::showEvent(QShowEvent* event) {
    recarregar();
    QWidget::showEvent(event);
}

void QuestoesPage::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (contentStack && contentStack->currentWidget() == detailPage) {
        renderImageListFullWidth(scrollImagensEnunciado, enunciadoImagesLayout,
                                 detalheEnunciadoPaths);
        renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
    }
    atualizarTamanhoItensLista();
}
