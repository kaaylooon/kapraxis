// QuestoesPage.cpp
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

#include "../repo/QuestaoRepoSQLite.h"

static QString buildTagsHtml(const QStringList& tags) {
    if (tags.isEmpty()) {
        return "<span style='padding: 4px 8px; border-radius: 999px; background-color: #3e3e3e; border: 1px solid #535353; color: #e0e0e0;'>Sem tags</span>";
    }

    QString html;
    for (const auto& tag : tags) {
        html += QString("<span style='padding: 4px 8px; border-radius: 999px; background-color: #3e3e3e; border: 1px solid #535353; color: #e0e0e0; margin-right: 6px;'>%1</span>")
            .arg(tag.toHtmlEscaped());
    }
    return html;
}

QuestoesPage::QuestoesPage(QWidget* parent)
    : QWidget(parent)
{
    repo = new QuestaoRepoSQLite;
    
    setupShortcuts();

    //carregarEstilo();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10); 

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    autoSaveTimer->setInterval(700);

    
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
    comboFilterTag->setEditable(false);
    
    filterLayout->addWidget(txtBusca, 1);
    filterLayout->addStretch();
    filterLayout->addWidget(comboFilterTag);
    
    auto* contentGroup = new QGroupBox();
    contentGroup->setFlat(true);
    auto* contentLayout = new QHBoxLayout(contentGroup);
    contentLayout->setSpacing(12);
    
    auto* listPanelGroup = new QGroupBox();
    listPanelGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* listPanelLayout = new QVBoxLayout(listPanelGroup);
    
    lista = new QListWidget();
    lista->setObjectName("lista");
    lista->setSelectionMode(QAbstractItemView::SingleSelection);
    lista->setViewMode(QListView::ListMode);
    lista->setResizeMode(QListView::Adjust);
    //lista->setGridSize(QSize(200, 150));
    lista->setSpacing(6);
    lista->setWordWrap(true);
    lista->setUniformItemSizes(false);
    lista->setDropIndicatorShown(false);
    
    listPanelLayout->addWidget(lista);
    
    detailsPanelGroup = new QGroupBox();
    detailsPanelGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* detailsPanelLayout = new QVBoxLayout(detailsPanelGroup);
    detailsPanelLayout->setSpacing(15);

    auto* enunciadoGroup = new QGroupBox();
    auto* enunciadoLayout = new QVBoxLayout(enunciadoGroup);
    txtEnunciado = new QTextEdit();
    txtEnunciado->setPlaceholderText("Digite o enunciado...");
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    enunciadoLayout->addWidget(txtEnunciado);

    auto* respostaGroup = new QGroupBox();
    auto* respostaLayout = new QVBoxLayout(respostaGroup);
    txtResposta = new QTextEdit();
    txtResposta->setPlaceholderText("Digite a resposta...");
    txtResposta->setWordWrapMode(QTextOption::WordWrap);
    respostaLayout->addWidget(txtResposta);

    auto* infoGroup = new QGroupBox();
    auto* infoLayout = new QHBoxLayout(infoGroup);
    lblTags = new QLabel("Sem tags");
    lblTags->setTextFormat(Qt::RichText);
    lblData = new QLabel("--/--/----");
    lblData->setObjectName("lblData");
    
    lblInfo = new QLabel(""); 
    
    infoLayout->addWidget(lblTags);
    infoLayout->addStretch();
    infoLayout->addWidget(lblData);
    
    detailsPanelLayout->addWidget(enunciadoGroup);
    detailsPanelLayout->addWidget(respostaGroup);
    detailsPanelLayout->addStretch();
    detailsPanelLayout->addWidget(lblInfo);
    detailsPanelLayout->addWidget(infoGroup);
    
    contentLayout->addWidget(listPanelGroup, 3);
    contentLayout->addWidget(detailsPanelGroup, 2);
    
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

    mainLayout->addWidget(filterGroup);
    mainLayout->addWidget(contentGroup, 1);
    mainLayout->addWidget(buttonGroup);
    

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
            return;
        }
        detailsPanelGroup->setVisible(true);
        mostrarDetalhes(current);
    });
    connect(comboFilterTag, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestoesPage::filtrarQuestoes);
    connect(txtResposta, &QTextEdit::textChanged, [this]() {
        if (!lista->currentItem()) return;
        autoSaveTimer->start();
    });

    connect(autoSaveTimer, &QTimer::timeout, this, &QuestoesPage::salvarResposta);

    detailsPanelGroup->setVisible(false);
    recarregar();
}

void QuestoesPage::adicionarQuestao() {
    Questao q;
    if (!abrirDialogQuestao(q, "Nova questão", "Adicionar questão", "Salvar")) {
        return;
    }
    
    repo->salvar(q);
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
    
    int id = item->data(Qt::UserRole).toInt();
    Questao q = repo->buscarPorId(id);
    
    if (q.id != 0) {
        q.resposta = txtResposta->toPlainText();
        repo->atualizar(q);
        
        // Atualiza indicador visual na lista
        bool temResposta = !q.resposta.isEmpty();
        item->setData(Qt::UserRole + 2, temResposta);
        
        // Feedback sutil
        lblInfo->setText("✓ Salvo em " + QTime::currentTime().toString("hh:mm:ss"));
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
    
    lista->clear();
    auto todasQuestoes = repo->listar();
    
    for (const auto& q : todasQuestoes) {
        if (q.enunciado.contains(texto, Qt::CaseInsensitive) ||
            q.resposta.contains(texto, Qt::CaseInsensitive) ||
            q.tags.join(" ").contains(texto, Qt::CaseInsensitive)) {
            
            QString status = q.resposta.isEmpty() ? "⏳" : "✅";
            QString preview = q.enunciado.left(100);
            if (q.enunciado.length() > 100) preview += "...";
            
            QString cardText = QString("%1 %2").arg(status).arg(preview);
            auto* item = new QListWidgetItem(cardText, lista);
            item->setData(Qt::UserRole, q.id);
            item->setData(Qt::UserRole + 1, q.enunciado);
            item->setData(Qt::UserRole + 2, !q.resposta.isEmpty());
            item->setTextAlignment(Qt::AlignCenter);
        }
    }
}


void QuestoesPage::editarQuestao() {
    QListWidgetItem* item = lista->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    
    Questao questao = repo->buscarPorId(id);

    if (questao.id == 0) return; 

    if (!abrirDialogQuestao(questao, "Editar questão", "Editar questão", "Salvar alterações")) {
        return;
    }

    repo->atualizar(questao);
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
        repo->excluir(id);
        recarregar();
    }
}

void QuestoesPage::mostrarDetalhes(QListWidgetItem* item) {
    if (!item) return;
    detailsPanelGroup->setVisible(true);
    
    int id = item->data(Qt::UserRole).toInt();
    auto todasQuestoes = repo->listar();
    
    for (const auto& q : todasQuestoes) {
        if (q.id == id) {
            txtEnunciado->setText(q.enunciado);
            txtResposta->setText(q.resposta);
            
            lblTags->setText(buildTagsHtml(q.tags));
            
            lblData->setText(q.criadaEm.toString("dd/MM/yyyy hh:mm"));
            break;
        }
    }
}

void QuestoesPage::filtrarQuestoes(int index) {
    recarregar();
}

void QuestoesPage::recarregar() {
    lista->clear();
    
    auto todasQuestoes = repo->listar();
    QList<Questao> questoesFiltradas;
    
    QString filtro = comboFilterTag->currentData().toString();
    
    for (const auto& q : todasQuestoes) {
        bool incluir = true;
        
        if (filtro == "sem_resposta" && !q.resposta.trimmed().isEmpty()) {
            incluir = false;
        } else if (filtro == "com_resposta" && q.resposta.trimmed().isEmpty()) {
            incluir = false;
        }
        
        if (incluir) {
            questoesFiltradas.append(q);
        }
    }
    
    for (auto& q : questoesFiltradas) {
        auto* item = new QListWidgetItem(lista);

        item->setData(Qt::UserRole, q.id);
        item->setData(Qt::UserRole + 1, q.enunciado);
        item->setData(Qt::UserRole + 2, q.tags);
        item->setData(Qt::UserRole + 3, !q.resposta.trimmed().isEmpty());
        
        const QString enunciadoPreview = q.enunciado.left(180) + (q.enunciado.length() > 180 ? "..." : "");
        const QString respostaPreview = q.resposta.trimmed().isEmpty()
            ? "Sem resposta"
            : q.resposta.left(140) + (q.resposta.length() > 140 ? "..." : "");

        auto* card = new QWidget();
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 10);
        cardLayout->setSpacing(6);

        auto* lblTitulo = new QLabel(enunciadoPreview);
        lblTitulo->setObjectName("cardTitle");
        lblTitulo->setWordWrap(true);

        auto* lblRespostaPreview = new QLabel(respostaPreview);
        lblRespostaPreview->setObjectName("cardSubtitle");
        lblRespostaPreview->setWordWrap(true);

        auto* bottomRow = new QHBoxLayout();
        auto* lblTagsCard = new QLabel(buildTagsHtml(q.tags));
        lblTagsCard->setTextFormat(Qt::RichText);
        lblTagsCard->setWordWrap(true);

        auto* lblDataCard = new QLabel(q.criadaEm.toString("dd/MM/yyyy"));
        lblDataCard->setObjectName("lblData");

        bottomRow->addWidget(lblTagsCard, 1);
        bottomRow->addStretch();
        bottomRow->addWidget(lblDataCard, 0, Qt::AlignRight);

        cardLayout->addWidget(lblTitulo);
        cardLayout->addWidget(lblRespostaPreview);
        cardLayout->addLayout(bottomRow);

        item->setSizeHint(card->sizeHint());
        lista->addItem(item);
        lista->setItemWidget(item, card);
    }

    if (lista->count() == 0) {
        detailsPanelGroup->setVisible(false);
    }
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
    auto* edtEnunciado = new QTextEdit();
    edtEnunciado->setPlaceholderText("Digite o enunciado...");
    edtEnunciado->setText(ioQuestao.enunciado);

    auto* lblResposta = new QLabel("Resposta (opcional)");
    auto* edtResposta = new QTextEdit();
    edtResposta->setPlaceholderText("Digite a resposta...");
    edtResposta->setText(ioQuestao.resposta);

    auto* lblTags = new QLabel("Tags");
    auto* edtTags = new QLineEdit();
    edtTags->setPlaceholderText("Ex.: álgebra, funções, geometria");
    edtTags->setText(ioQuestao.tags.join(", "));

    formLayout->addWidget(lblEnunciado);
    formLayout->addWidget(edtEnunciado);
    formLayout->addWidget(lblResposta);
    formLayout->addWidget(edtResposta);
    formLayout->addWidget(lblTags);
    formLayout->addWidget(edtTags);

    layout->addWidget(formGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(actionLabel);
    buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        if (edtEnunciado->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validação", "O enunciado não pode ficar vazio.");
            return;
        }
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    const QString tagsRaw = edtTags->text();
    QStringList tags = tagsRaw.split(",", Qt::SkipEmptyParts);
    for (QString& t : tags) {
        t = t.trimmed();
    }
    tags.removeAll(QString());

    ioQuestao.enunciado = edtEnunciado->toPlainText().trimmed();
    ioQuestao.resposta = edtResposta->toPlainText().trimmed();
    ioQuestao.tags = tags;
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
