#include "QuestoesPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QDateTime>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QFrame>
#include <QFile>
#include <QLineEdit>
#include <QComboBox>
#include <QPainter>
#include <QShortcut>
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QPixmapCache>
#include <QSet>
#include <QDate>
#include <QLocale>
#include <QStackedWidget>
#include <QScrollArea>
#include <QResizeEvent>
#include <QColor>
#include <QImageReader>

#include "../repo/questao_repo_sqlite.h"
#include "../utils/ImageStore.h"
#include "ClipboardTextEdit.h"

namespace krepo = kapraxis::repo;

static QString tagChipStyle(const QString& tag) {
    const uint h = qHash(tag.toLower());
    const int hue = static_cast<int>(h % 360);
    const QColor border = QColor::fromHsl(hue, 120, 120);
    const QColor text = QColor::fromHsl(hue, 180, 140);
    return QString("border:1px solid %1;color:%2;font-weight:700;")
        .arg(border.name(), text.name());
}

static QString buildTagsHtml(const QStringList& tags) {
    if (tags.isEmpty()) {
        return "<span style='padding: 6px 14px; border-radius: 999px; border: 1px solid #3e3e3e; color: #cfcfcf; font-weight:700;'>Sem tags</span>";
    }

    QString html;
    for (const auto& tag : tags) {
        const QString style = tagChipStyle(tag);
        html += QString("<span style='padding: 6px 14px; border-radius: 999px; %1 margin-right: 8px; margin-bottom: 6px; display: inline-block;'>%2</span>")
            .arg(style, tag.toHtmlEscaped());
    }
    return html;
}

static QLocale localePtBr() {
    return QLocale(QLocale::Portuguese, QLocale::Brazil);
}

static QString groupKeyFor(const QDateTime& dt, const QString& mode) {
    const QLocale loc = localePtBr();
    const QDate date = dt.date();
    if (mode == "day") {
        return loc.toString(date, "dddd");
    }
    if (mode == "week") {
        int weekYear = 0;
        const int week = date.weekNumber(&weekYear);
        return QString("Semana %1").arg(week);
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
    auto* item = new QListWidgetItem(title, list);
    item->setFlags(Qt::ItemIsEnabled);
    item->setData(Qt::UserRole + 99, true);
    item->setSizeHint(QSize(item->sizeHint().width(), 34));
    item->setBackground(QColor(45, 45, 45));
    item->setForeground(QColor(255, 255, 255));
    QFont f = item->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 3);
    item->setFont(f);
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

static void renderImageListFullWidth(QScrollArea* scroll, QVBoxLayout* layout, const QStringList& paths) {
    clearLayout(layout);

    const int viewportWidth = scroll->viewport()->width();
    const int targetWidth = qMax(200, viewportWidth - 16);

    for (const auto& path : paths) {
        QPixmap pix(path);
        if (pix.isNull()) {
            continue;
        }
        auto* label = new QLabel();
        label->setAlignment(Qt::AlignCenter);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        const QPixmap scaled = pix.scaledToWidth(targetWidth, Qt::SmoothTransformation);
        label->setPixmap(scaled);
        label->setMinimumHeight(scaled.height());
        layout->addWidget(label);
    }
    layout->addStretch();
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
    const qint64 created = static_cast<qint64>(obj.value("createdTimestampUsec").toVariant().toLongLong());
    const qint64 edited = static_cast<qint64>(obj.value("userEditedTimestampUsec").toVariant().toLongLong());
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
            cached = fallback.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmapCache::insert(key, cached);
            return cached;
        }
        return QPixmap();
    }
    cached = QPixmap::fromImage(img).scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
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

QuestoesPage::QuestoesPage(QWidget* parent)
    : QWidget(parent)
{
    repo = new krepo::QuestaoRepoSQLite;
    
    setupShortcuts();

    //carregarEstilo();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10); 

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    autoSaveTimer->setInterval(1000);

    
    auto* filterGroup = new QGroupBox();
    auto* filterLayout = new QHBoxLayout(filterGroup);
    
    txtBusca = new QLineEdit();
    txtBusca->setObjectName("txtBusca");
    txtBusca->setPlaceholderText("Buscar...");
    txtBusca->setClearButtonEnabled(false);


    comboFilterTag = new QComboBox();
    comboFilterTag->setObjectName("comboFilterTag");
    comboFilterTag->addItem("Todas", "");
    comboFilterTag->addItem("Sem resposta", "sem_resposta");
    comboFilterTag->addItem("Com resposta", "com_resposta");
    comboFilterTag->addItem("Sem tags", "sem_tags");
    comboFilterTag->addItem("Com tags", "com_tags");
    comboFilterTag->addItem("Hoje", "hoje");
    comboFilterTag->addItem("Ultimos 7 dias", "ultimos_7");
    comboFilterTag->addItem("Ultimos 30 dias", "ultimos_30");
    comboFilterTag->setEditable(false);

    comboGroupBy = new QComboBox();
    comboGroupBy->setObjectName("comboGroupBy");
    comboGroupBy->addItem("Sem agrupamento", "");
    comboGroupBy->addItem("Agrupar por dia", "day");
    comboGroupBy->addItem("Agrupar por semana", "week");
    comboGroupBy->addItem("Agrupar por mes", "month");
    comboGroupBy->addItem("Agrupar por ano", "year");
    comboGroupBy->setEditable(false);

    filterLayout->addWidget(txtBusca, 1);
    filterLayout->addStretch();
    filterLayout->addWidget(comboFilterTag);
    filterLayout->addWidget(comboGroupBy);
    
    auto* listPanelGroup = new QGroupBox();
    listPanelGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* listPanelLayout = new QVBoxLayout(listPanelGroup);
    
    lista = new QListWidget();
    lista->setObjectName("lista");
    lista->setSelectionMode(QAbstractItemView::SingleSelection);
    lista->setViewMode(QListView::ListMode);
    lista->setResizeMode(QListView::Adjust);
    //lista->setGridSize(QSize(200, 150));
    lista->setSpacing(8);
    lista->setWordWrap(true);
    lista->setUniformItemSizes(false);
    lista->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    lista->setContentsMargins(0, 0, 0, 0);
    lista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lista->setDropIndicatorShown(false);
    
    listPanelLayout->addWidget(lista);
    
    detailsPanelGroup = new QGroupBox();
    detailsPanelGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* detailsPanelLayout = new QVBoxLayout(detailsPanelGroup);
    detailsPanelLayout->setSpacing(15);

    auto* enunciadoGroup = new QGroupBox();
    auto* enunciadoLayout = new QVBoxLayout(enunciadoGroup);
    txtEnunciado = new ClipboardTextEdit();
    txtEnunciado->setPlaceholderText("Digite o enunciado...");
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    enunciadoLayout->addWidget(txtEnunciado);

    respostaGroup = new QGroupBox();
    auto* respostaLayout = new QVBoxLayout(respostaGroup);
    txtResposta = new ClipboardTextEdit();
    txtResposta->setPlaceholderText("Digite a resposta...");
    txtResposta->setWordWrapMode(QTextOption::WordWrap);
    respostaLayout->addWidget(txtResposta);
    respostaGroup->setObjectName("respostaGroup");

    btnCampoAdicional = new QPushButton("Adicionar campo adicional");
    btnCampoAdicional->setObjectName("btnCampoAdicional");
    btnCampoAdicional->setCursor(Qt::PointingHandCursor);

    imagensEnunciadoGroup = new QGroupBox("Imagens do enunciado");
    imagensEnunciadoGroup->setObjectName("imagensEnunciadoGroup");
    auto* imagensEnunciadoLayout = new QVBoxLayout(imagensEnunciadoGroup);
    scrollImagensEnunciado = new QScrollArea();
    scrollImagensEnunciado->setWidgetResizable(true);
    scrollImagensEnunciado->setFrameShape(QFrame::NoFrame);
    enunciadoImagesContainer = new QWidget();
    enunciadoImagesLayout = new QVBoxLayout(enunciadoImagesContainer);
    enunciadoImagesLayout->setSpacing(12);
    enunciadoImagesLayout->setContentsMargins(0, 0, 0, 0);
    scrollImagensEnunciado->setWidget(enunciadoImagesContainer);
    imagensEnunciadoLayout->addWidget(scrollImagensEnunciado);
    imagensEnunciadoGroup->setVisible(false);

    imagensRespostaGroup = new QGroupBox("Imagens do campo adicional");
    imagensRespostaGroup->setObjectName("imagensRespostaGroup");
    auto* imagensRespostaLayout = new QVBoxLayout(imagensRespostaGroup);
    scrollImagensResposta = new QScrollArea();
    scrollImagensResposta->setWidgetResizable(true);
    scrollImagensResposta->setFrameShape(QFrame::NoFrame);
    respostaImagesContainer = new QWidget();
    respostaImagesLayout = new QVBoxLayout(respostaImagesContainer);
    respostaImagesLayout->setSpacing(12);
    respostaImagesLayout->setContentsMargins(0, 0, 0, 0);
    scrollImagensResposta->setWidget(respostaImagesContainer);
    imagensRespostaLayout->addWidget(scrollImagensResposta);
    imagensRespostaGroup->setVisible(false);

    auto* infoGroup = new QGroupBox();
    auto* infoLayout = new QHBoxLayout(infoGroup);
    lblTags = new QLabel("Sem tags");
    lblTags->setTextFormat(Qt::RichText);
    lblData = new QLabel("--/--/-lblData---");
    lblData->setObjectName("lblData");
    
    lblInfo = new QLabel(""); 
    lblInfo->setObjectName("lblData");
    
    infoLayout->addWidget(lblTags);
    infoLayout->addStretch();
    infoLayout->addWidget(lblData);
    infoLayout->addWidget(lblInfo);
    
    detailsPanelLayout->addWidget(enunciadoGroup);
    detailsPanelLayout->addWidget(imagensEnunciadoGroup);
    detailsPanelLayout->addWidget(btnCampoAdicional);
    detailsPanelLayout->addWidget(respostaGroup);
    detailsPanelLayout->addWidget(imagensRespostaGroup);
    detailsPanelLayout->addStretch();
    detailsPanelLayout->addWidget(infoGroup);
    
    auto* buttonGroup = new QGroupBox();
    buttonGroup->setFlat(true);
    auto* buttonLayout = new QHBoxLayout(buttonGroup);

    btnAdd = new QPushButton("Adicionar");
    btnAdd->setObjectName("btnAdd");
    btnAdd->setCursor(Qt::PointingHandCursor);
    
    btnEdit = new QPushButton("Editar");
    btnEdit->setObjectName("btnEdit");
    btnEdit->setCursor(Qt::PointingHandCursor);
    
    btnDelete = new QPushButton("Excluir");
    btnDelete->setObjectName("btnDelete");
    btnDelete->setCursor(Qt::PointingHandCursor);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addStretch();

    contentStack = new QStackedWidget();
    listPage = new QWidget();
    listPage->setObjectName("questoesListPage");
    detailPage = new QWidget();
    detailPage->setObjectName("questoesDetailPage");

    auto* listPageLayout = new QVBoxLayout(listPage);
    listPageLayout->setSpacing(10);
    listPageLayout->addWidget(filterGroup);
    listPageLayout->addWidget(listPanelGroup, 1);
    listPageLayout->addWidget(buttonGroup);

    btnVoltarLista = new QPushButton("Voltar");
    btnVoltarLista->setObjectName("btnVoltarLista");
    btnVoltarLista->setCursor(Qt::PointingHandCursor);

    auto* detailPageLayout = new QVBoxLayout(detailPage);
    detailPageLayout->setSpacing(10);
    auto* detailHeader = new QHBoxLayout();
    detailHeader->addWidget(btnVoltarLista, 0, Qt::AlignLeft);
    detailHeader->addStretch();
    detailPageLayout->addLayout(detailHeader);
    detailPageLayout->addWidget(detailsPanelGroup, 1);

    contentStack->addWidget(listPage);
    contentStack->addWidget(detailPage);

    mainLayout->addWidget(contentStack, 1);
    

    connect(txtBusca, &QLineEdit::textChanged, this, &QuestoesPage::buscarQuestoes);
    connect(btnAdd, &QPushButton::clicked,
            this, &QuestoesPage::adicionarQuestao);
    connect(btnEdit, &QPushButton::clicked,
            this, &QuestoesPage::editarQuestao);
    connect(btnDelete, &QPushButton::clicked,
            this, &QuestoesPage::excluirQuestao);
    connect(lista, &QListWidget::itemClicked,
            this, &QuestoesPage::mostrarDetalhes);
    connect(lista, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem*) {
        if (!current) {
            detailsPanelGroup->setVisible(false);
            contentStack->setCurrentWidget(listPage);
            return;
        }
        detailsPanelGroup->setVisible(true);
        mostrarDetalhes(current);
    });
    connect(comboFilterTag, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestoesPage::filtrarQuestoes);
    connect(comboGroupBy, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestoesPage::filtrarQuestoes);
    connect(txtResposta, &QTextEdit::textChanged, [this]() {
        if (!lista->currentItem()) return;
        autoSaveTimer->start();
    });

    connect(autoSaveTimer, &QTimer::timeout, this, &QuestoesPage::salvarResposta);
    connect(lista->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        carregarMais();
    });

    connect(btnVoltarLista, &QPushButton::clicked, [this]() {
        contentStack->setCurrentWidget(listPage);
    });

    connect(btnCampoAdicional, &QPushButton::clicked, [this]() {
        if (!lista->currentItem()) return;
        const int id = lista->currentItem()->data(Qt::UserRole).toInt();
        Questao q = repo->BuscarPorId(id);
        if (q.id == 0) return;

        if (respostaGroup->isVisible()) {
            const int resposta = QMessageBox::question(
                this,
                "Remover campo adicional",
                "Deseja remover o campo adicional e apagar o conteudo?",
                QMessageBox::Yes | QMessageBox::No
            );
            if (resposta != QMessageBox::Yes) {
                return;
            }
            q.resposta.clear();
            q.respostaImagens.clear();
            repo->Atualizar(q);
            txtResposta->clear();
            detalheRespostaPaths.clear();
            renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
            imagensRespostaGroup->setVisible(false);
            respostaGroup->setVisible(false);
            btnCampoAdicional->setText("Adicionar campo adicional");
        } else {
            respostaGroup->setVisible(true);
            btnCampoAdicional->setText("Remover campo adicional");
            txtResposta->setFocus();
        }
    });

    auto addImageToCurrent = [this](const QImage& image, const QString& tipo) {
        QListWidgetItem* item = lista->currentItem();
        if (!item) {
            return;
        }
        const QString saved = ImageStore::saveImage(image, tipo);
        if (saved.isEmpty()) {
            QMessageBox::warning(this, "Imagem", "Nao foi possivel colar a imagem.");
            return;
        }
        const int id = item->data(Qt::UserRole).toInt();
        Questao q = repo->BuscarPorId(id);
        if (q.id == 0) {
            return;
        }
        if (tipo == "enunciado") {
            q.enunciadoImagens.append(saved);
        } else {
            q.respostaImagens.append(saved);
            if (!respostaGroup->isVisible()) {
                respostaGroup->setVisible(true);
                btnCampoAdicional->setText("Remover campo adicional");
            }
        }
        repo->Atualizar(q);
        detalheEnunciadoPaths = q.enunciadoImagens;
        detalheRespostaPaths = q.respostaImagens;
        renderImageListFullWidth(scrollImagensEnunciado, enunciadoImagesLayout, detalheEnunciadoPaths);
        renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
        imagensEnunciadoGroup->setVisible(!detalheEnunciadoPaths.isEmpty());
        imagensRespostaGroup->setVisible(!detalheRespostaPaths.isEmpty());
    };

    connect(txtEnunciado, &ClipboardTextEdit::imagePasted, [addImageToCurrent](const QImage& image) {
        addImageToCurrent(image, "enunciado");
    });
    connect(txtResposta, &ClipboardTextEdit::imagePasted, [addImageToCurrent](const QImage& image) {
        addImageToCurrent(image, "resposta");
    });

    detailsPanelGroup->setVisible(false);
    contentStack->setCurrentWidget(listPage);
    recarregar();
}

void QuestoesPage::adicionarQuestao() {
    Questao q;
    if (!abrirDialogQuestao(q, "Nova questão", "Adicionar questão", "Salvar")) {
        return;
    }
    
    repo->Salvar(q);
    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
}

void QuestoesPage::setupShortcuts() {
    atalhoNovaQuestao = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(atalhoNovaQuestao, &QShortcut::activated, this, &QuestoesPage::adicionarQuestao);

    atalhoSalvar = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(atalhoSalvar, &QShortcut::activated, [this]() {
        QTimer::singleShot(5000, [this]() {
            if (lista->currentItem()) {
                lblInfo->setText("Editando...");
            } else {
                lblInfo->setText("Selecione...");
            }
        });
    });

    atalhoBusca = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(atalhoBusca, &QShortcut::activated, this, &QuestoesPage::focarBusca);
}

void QuestoesPage::salvarResposta() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;
    if (!respostaGroup->isVisible()) return;
    
    int id = item->data(Qt::UserRole).toInt();
    Questao q = repo->BuscarPorId(id);
    
    if (q.id != 0) {
        q.resposta = txtResposta->toPlainText();
        repo->Atualizar(q);
        
        // Atualiza indicador visual na lista
        bool temResposta = !q.resposta.isEmpty();
        item->setData(Qt::UserRole + 2, temResposta);
        
        // Feedback sutil
        lblInfo->setText("+ " + QTime::currentTime().toString("hh:mm:ss"));
    }
}

void QuestoesPage::focarBusca() {
    txtBusca->setFocus();
    txtBusca->selectAll();
}

void QuestoesPage::destacarItem(QListWidgetItem* item) {
    if(!item) return;

    QTimer::singleShot(100, [item]() {
        item->setBackground(QColor(254, 239, 195));
    });
    
    QTimer::singleShot(800, [item]() {
        item->setBackground(QBrush());
    });
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


void QuestoesPage::editarQuestao() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    
    Questao questao = repo->BuscarPorId(id);

    if (questao.id == 0) return; 

    if (!abrirDialogQuestao(questao, "Editar questão", "Editar questão", "Salvar alterações")) {
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

    int resposta = QMessageBox::question(
        this, "Confirmar exclusão",
        "Tem certeza que deseja excluir esta questão?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (resposta == QMessageBox::Yes) {
        repo->Excluir(id);
        krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
        recarregar();
    }
}

void QuestoesPage::mostrarDetalhes(QListWidgetItem* item) {
    if (!item) return;
    if (item->data(Qt::UserRole + 99).toBool()) return;
    detailsPanelGroup->setVisible(true);
    contentStack->setCurrentWidget(detailPage);
    
    int id = item->data(Qt::UserRole).toInt();
    const Questao q = repo->BuscarPorId(id);
    if (q.id == 0) return;

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
    btnCampoAdicional->setText(hasResposta ? "Remover campo adicional" : "Adicionar campo adicional");
}

void QuestoesPage::filtrarQuestoes(int index) {
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
    // tamanho ajustado ao final da paginação
}

void QuestoesPage::excluirTodasQuestoes() {
    const int resposta = QMessageBox::question(
        this,
        "Remover todas as questoes",
        "Tem certeza que deseja apagar todas as questoes? Esta acao nao pode ser desfeita.",
        QMessageBox::Yes | QMessageBox::No
    );
    if (resposta != QMessageBox::Yes) {
        return;
    }
    repo->ExcluirTodas();
    krepo::QuestaoRepoSQLite::InvalidarCacheBasico();
    recarregar();
    contentStack->setCurrentWidget(listPage);
}

void QuestoesPage::importarKeepJson() {
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Selecionar arquivos JSON do Google Keep",
        QString(),
        "JSON (*.json)"
    );
    if (files.isEmpty()) {
        return;
    }

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
    QMessageBox::information(
        this,
        "Importacao concluida",
        QString("Importadas: %1\nIgnoradas: %2").arg(imported).arg(skipped)
    );
}

void QuestoesPage::adicionarItemLista(const Questao& q) {
    auto* item = new QListWidgetItem(lista);

    item->setData(Qt::UserRole, q.id);
    item->setData(Qt::UserRole + 1, q.enunciado);
    item->setData(Qt::UserRole + 2, q.tags);
    item->setData(Qt::UserRole + 3, !q.resposta.trimmed().isEmpty());
    item->setData(Qt::UserRole + 99, false);

    const QString enunciadoPreview = q.enunciado;
    const QString respostaPreview = q.resposta.trimmed().isEmpty()
        ? "Sem resposta"
        : q.resposta;

    auto* card = new QWidget();
    card->setObjectName("listaCard");
    card->setAttribute(Qt::WA_StyledBackground, true);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 12, 14, 12);
    cardLayout->setSpacing(6);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* lblTitulo = new QLabel(enunciadoPreview);
    lblTitulo->setObjectName("cardTitle");
    lblTitulo->setTextFormat(Qt::PlainText);
    lblTitulo->setWordWrap(true);
    lblTitulo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lblTitulo->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblTitulo->setContentsMargins(0, 0, 0, 0);

    auto* lblRespostaPreview = new QLabel(respostaPreview);
    lblRespostaPreview->setObjectName("cardSubtitle");
    lblRespostaPreview->setTextFormat(Qt::PlainText);
    lblRespostaPreview->setWordWrap(true);
    lblRespostaPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lblRespostaPreview->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblRespostaPreview->setContentsMargins(0, 0, 0, 2);

    auto* topRow = new QVBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(6);
    topRow->addWidget(lblTitulo);
    topRow->addWidget(lblRespostaPreview);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 2, 0, 0);
    bottomRow->setSpacing(1);
    auto* lblTagsCard = new QLabel(buildTagsHtml(q.tags));
    lblTagsCard->setTextFormat(Qt::RichText);
    lblTagsCard->setWordWrap(true);
    lblTagsCard->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblTagsCard->setContentsMargins(0, 0, 0, 0);
    lblTagsCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* lblDataCard = new QLabel(q.criadaEm.toString("dd/MM/yyyy"));
    lblDataCard->setObjectName("lblData");

    bottomRow->addWidget(lblTagsCard, 1);
    bottomRow->addStretch();
    bottomRow->addWidget(lblDataCard, 0, Qt::AlignRight);

    cardLayout->addLayout(topRow);
    cardLayout->addLayout(bottomRow);

    cardLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
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
        if (item->data(Qt::UserRole + 99).toBool()) {
            continue;
        }
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

    loadingItem = new QListWidgetItem("Carregando...");
    loadingItem->setFlags(Qt::NoItemFlags);
    loadingItem->setData(Qt::UserRole + 99, true);
    lista->addItem(loadingItem);

    carregarMais();
}

void QuestoesPage::carregarMais() {
    if (renderQueue.isEmpty()) {
        return;
    }
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
            if (cur <= lastScrollValue) {
                return;
            }
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
        loadingItem = new QListWidgetItem("Carregando...");
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

bool QuestoesPage::abrirDialogQuestao(Questao& ioQuestao,
                                      const QString& titulo,
                                      const QString& header,
                                      const QString& actionLabel) {
    QDialog dialog(this);
    dialog.setWindowTitle(titulo);
    dialog.setModal(true);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);

    auto* headerLabel = new QLabel(header);
    headerLabel->setStyleSheet("font-size: 20px; font-weight: 600;");
    layout->addWidget(headerLabel);

    auto* formGroup = new QGroupBox();
    auto* formLayout = new QVBoxLayout(formGroup);
    formLayout->setSpacing(10);

    auto* lblEnunciado = new QLabel("Enunciado");
    auto* edtEnunciado = new ClipboardTextEdit();
    edtEnunciado->setPlaceholderText("Digite o enunciado...");
    edtEnunciado->setText(ioQuestao.enunciado);

    auto* lblEnunciadoImgs = new QLabel("Imagens do enunciado");
    auto* listaEnunciadoImgs = new QListWidget();
    configureImageList(listaEnunciadoImgs);

    auto* enunciadoImgButtons = new QHBoxLayout();
    auto* btnAddEnunciadoImg = new QPushButton("Adicionar imagem");
    auto* btnRemEnunciadoImg = new QPushButton("Remover selecionada");
    enunciadoImgButtons->addWidget(btnAddEnunciadoImg);
    enunciadoImgButtons->addWidget(btnRemEnunciadoImg);
    enunciadoImgButtons->addStretch();

    auto* btnCampoAdicionalDialog = new QPushButton("Adicionar campo adicional");
    btnCampoAdicionalDialog->setObjectName("btnCampoAdicionalDialog");
    btnCampoAdicionalDialog->setCursor(Qt::PointingHandCursor);

    auto* respostaContainer = new QWidget();
    auto* respostaContainerLayout = new QVBoxLayout(respostaContainer);
    respostaContainerLayout->setContentsMargins(0, 0, 0, 0);
    respostaContainerLayout->setSpacing(10);

    auto* lblResposta = new QLabel("Campo adicional");
    auto* edtResposta = new ClipboardTextEdit();
    edtResposta->setPlaceholderText("Digite o campo adicional...");
    edtResposta->setText(ioQuestao.resposta);

    auto* lblRespostaImgs = new QLabel("Imagens do campo adicional");
    auto* listaRespostaImgs = new QListWidget();
    configureImageList(listaRespostaImgs);

    auto* respostaImgButtons = new QHBoxLayout();
    auto* btnAddRespostaImg = new QPushButton("Adicionar imagem");
    auto* btnRemRespostaImg = new QPushButton("Remover selecionada");
    respostaImgButtons->addWidget(btnAddRespostaImg);
    respostaImgButtons->addWidget(btnRemRespostaImg);
    respostaImgButtons->addStretch();

    respostaContainerLayout->addWidget(lblResposta);
    respostaContainerLayout->addWidget(edtResposta);
    respostaContainerLayout->addWidget(lblRespostaImgs);
    respostaContainerLayout->addWidget(listaRespostaImgs);
    respostaContainerLayout->addLayout(respostaImgButtons);

    auto* lblTags = new QLabel("Tags");
    auto* edtTags = new QLineEdit();
    edtTags->setPlaceholderText("Ex.: algebra, funcoes, geometria");
    edtTags->setText(ioQuestao.tags.join(", "));

    formLayout->addWidget(lblEnunciado);
    formLayout->addWidget(edtEnunciado);
    formLayout->addWidget(lblEnunciadoImgs);
    formLayout->addWidget(listaEnunciadoImgs);
    formLayout->addLayout(enunciadoImgButtons);
    formLayout->addWidget(btnCampoAdicionalDialog);
    formLayout->addWidget(respostaContainer);
    formLayout->addWidget(lblTags);
    formLayout->addWidget(edtTags);

    layout->addWidget(formGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(actionLabel);
    buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    layout->addWidget(buttons);

    preencherListaImagens(listaEnunciadoImgs, ioQuestao.enunciadoImagens, true);
    preencherListaImagens(listaRespostaImgs, ioQuestao.respostaImagens, true);

    QSet<QString> newlyAdded;
    const bool hasRespostaInicial = !ioQuestao.resposta.trimmed().isEmpty() || !ioQuestao.respostaImagens.isEmpty();
    respostaContainer->setVisible(hasRespostaInicial);
    btnCampoAdicionalDialog->setText(hasRespostaInicial ? "Remover campo adicional" : "Adicionar campo adicional");

    auto addImagePath = [&](QListWidget* list, const QString& path) {
        if (path.isEmpty()) {
            return;
        }
        addImageItem(list, path, true);
        newlyAdded.insert(path);
    };

    auto addImageFromFile = [&](QListWidget* list, const QString& prefix) {
        const QStringList files = QFileDialog::getOpenFileNames(
            &dialog,
            "Selecionar imagens",
            QString(),
            "Imagens (*.png *.jpg *.jpeg *.bmp)"
        );
        for (const auto& file : files) {
            const QString saved = ImageStore::saveImageFromFile(file, prefix);
            if (saved.isEmpty()) {
                QMessageBox::warning(&dialog, "Imagem", "Nao foi possivel carregar a imagem selecionada.");
                continue;
            }
            addImagePath(list, saved);
        }
    };

    auto addImageFromClipboard = [&](QListWidget* list, const QImage& image, const QString& prefix) {
        const QString saved = ImageStore::saveImage(image, prefix);
        if (saved.isEmpty()) {
            QMessageBox::warning(&dialog, "Imagem", "Nao foi possivel colar a imagem do clipboard.");
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

    connect(btnAddEnunciadoImg, &QPushButton::clicked, [&]() {
        addImageFromFile(listaEnunciadoImgs, "enunciado");
    });
    connect(btnAddRespostaImg, &QPushButton::clicked, [&]() {
        addImageFromFile(listaRespostaImgs, "resposta");
    });
    connect(btnRemEnunciadoImg, &QPushButton::clicked, [&]() {
        removeSelectedImage(listaEnunciadoImgs);
    });
    connect(btnRemRespostaImg, &QPushButton::clicked, [&]() {
        removeSelectedImage(listaRespostaImgs);
    });
    connect(btnCampoAdicionalDialog, &QPushButton::clicked, [&]() {
        if (respostaContainer->isVisible()) {
            const int resposta = QMessageBox::question(
                &dialog,
                "Remover campo adicional",
                "Deseja remover o campo adicional e apagar o conteudo?",
                QMessageBox::Yes | QMessageBox::No
            );
            if (resposta != QMessageBox::Yes) {
                return;
            }
            edtResposta->clear();
            listaRespostaImgs->clear();
            respostaContainer->setVisible(false);
            btnCampoAdicionalDialog->setText("Adicionar campo adicional");
        } else {
            respostaContainer->setVisible(true);
            btnCampoAdicionalDialog->setText("Remover campo adicional");
            edtResposta->setFocus();
        }
    });
    connect(edtEnunciado, &ClipboardTextEdit::imagePasted, [&](const QImage& image) {
        addImageFromClipboard(listaEnunciadoImgs, image, "enunciado");
    });
    connect(edtResposta, &ClipboardTextEdit::imagePasted, [&](const QImage& image) {
        if (!respostaContainer->isVisible()) {
            respostaContainer->setVisible(true);
            btnCampoAdicionalDialog->setText("Remover campo adicional");
        }
        addImageFromClipboard(listaRespostaImgs, image, "resposta");
    });

    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        if (edtEnunciado->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validação", "O enunciado não pode ficar vazio.");
            return;
        }
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        const QStringList finalPaths = coletarPaths(listaEnunciadoImgs) + coletarPaths(listaRespostaImgs);
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

void QuestoesPage::carregarEstilo() {
    QFile file(":/styles/global.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    } else {
        this->setStyleSheet(R"(
            QWidget { background-color: #f5f5f5; font-family: Arial; }
            QPushButton { 
                background-color: #007bff; color: white; border-radius: 4px; 
                padding: 6px 12px; font-weight: bold; font-size: 12px;
            }
            QPushButton:hover { background-color: #0056b3; }
            QListWidget { 
                background-color: white; border: 1px solid #ddd; 
                border-radius: 4px; padding: 5px;
            }
            QListWidget::item:selected { background-color: #007bff; color: white; }
            QTextEdit, QLineEdit { 
                border: 1px solid #ddd; border-radius: 4px; padding: 6px;
            }
            QRadioButton { margin-right: 10px; }
            QLabel[font-weight="bold"] { font-weight: bold; }
        )");
    }
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
        renderImageListFullWidth(scrollImagensEnunciado, enunciadoImagesLayout, detalheEnunciadoPaths);
        renderImageListFullWidth(scrollImagensResposta, respostaImagesLayout, detalheRespostaPaths);
    }
    atualizarTamanhoItensLista();
}
