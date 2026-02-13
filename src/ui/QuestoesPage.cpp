// QuestoesPage.cpp
#include "QuestoesPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QDateTime>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QFrame>
#include <QFile>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QPainter>
#include <QShortcut>
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>

#include "../repo/QuestaoRepoSQLite.h"

QuestoesPage::QuestoesPage(QWidget* parent)
    : QWidget(parent)
{
    repo = new QuestaoRepoSQLite;
    
    setupShortcuts();

    //carregarEstilo();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10); 
    
    
    auto* filterGroup = new QGroupBox();
    auto* filterLayout = new QHBoxLayout(filterGroup);
    
    txtBusca = new QLineEdit();
    txtBusca->setObjectName("txtBusca");
    txtBusca->setPlaceholderText("Buscar...");
    txtBusca->setClearButtonEnabled(false);
    txtBusca->setFixedHeight(46);


    comboFilterTag = new QComboBox();
    comboFilterTag->addItem("Todas", "");
    comboFilterTag->addItem("Sem resposta", "sem_resposta");
    comboFilterTag->addItem("Com resposta", "com_resposta");
    comboFilterTag->setFixedWidth(180);
    comboFilterTag->setEditable(false);
    
    filterLayout->addWidget(txtBusca, 1);
    filterLayout->addStretch();
    filterLayout->addWidget(comboFilterTag);
    
    auto* contentGroup = new QGroupBox();
    contentGroup->setFlat(true);
    auto* contentLayout = new QHBoxLayout(contentGroup);
    
    auto* listPanelGroup = new QGroupBox();
    listPanelGroup->setMinimumWidth(400);
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
    
    auto* detailsPanelGroup = new QGroupBox();
    auto* detailsPanelLayout = new QVBoxLayout(detailsPanelGroup);
    detailsPanelLayout->setSpacing(15);

    auto* enunciadoGroup = new QGroupBox();
    auto* enunciadoLayout = new QVBoxLayout(enunciadoGroup);
    txtEnunciado = new QTextEdit();
    txtEnunciado->setPlaceholderText("Digite o enunciado...");
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    txtEnunciado->setMinimumHeight(200);
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    enunciadoLayout->addWidget(txtEnunciado);
    
    auto* respostaGroup = new QGroupBox();
    auto* respostaLayout = new QVBoxLayout(respostaGroup);
    txtResposta = new QTextEdit();
    txtResposta->setPlaceholderText("Digite a resposta...");
    txtResposta->setMinimumHeight(200);
    txtResposta->setWordWrapMode(QTextOption::WordWrap);
    respostaLayout->addWidget(txtResposta);
    
    auto* infoGroup = new QGroupBox();
    auto* infoLayout = new QHBoxLayout(infoGroup);
    lblTags = new QLabel("Sem tags");
    lblTags->setStyleSheet("color: #666;");
    lblData = new QLabel("--/--/----");
    lblData->setStyleSheet("color: #666;");
    
    lblInfo = new QLabel(""); 
    
    infoLayout->addWidget(lblTags);
    infoLayout->addStretch();
    infoLayout->addWidget(lblData);
    
    detailsPanelLayout->addWidget(txtEnunciado);
    detailsPanelLayout->addWidget(txtResposta);
    detailsPanelLayout->addStretch();
    detailsPanelLayout->addWidget(infoGroup);
    detailsPanelLayout->addWidget(lblInfo);
    
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
    connect(comboFilterTag, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestoesPage::filtrarQuestoes);
    connect(txtResposta, &QTextEdit::textChanged, [this]() {
        QListWidgetItem* item = lista->currentItem();
        if (item) {
            int id = item->data(Qt::UserRole).toInt();
            // implementaria a atualização no banco
        }
    });
    
    recarregar();
}

void QuestoesPage::adicionarQuestao() {
    bool ok;
    QString enunciado = QInputDialog::getMultiLineText(
        this, "Nova questão", "Enunciado:", "", &ok);
    
    if (!ok || enunciado.trimmed().isEmpty()) return;
    
    QString tagsStr = QInputDialog::getText(
        this, "Tags", "Tags (separadas por vírgula):", QLineEdit::Normal, "", &ok);
    
    Questao q;
    q.enunciado = enunciado;
    q.resposta = "";
    q.tags = tagsStr.split(",", Qt::SkipEmptyParts);
    q.criadaEm = QDateTime::currentDateTime();
    
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
    
    bool ok;
    QString novoEnunciado = QInputDialog::getMultiLineText(
        this, "Editar questão", "Enunciado:", questao.enunciado, &ok);
    
    if (ok && !novoEnunciado.trimmed().isEmpty()) {
        questao.enunciado = novoEnunciado;
        repo->atualizar(questao);
        recarregar();
    }
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
    
    int id = item->data(Qt::UserRole).toInt();
    auto todasQuestoes = repo->listar();
    
    for (const auto& q : todasQuestoes) {
        if (q.id == id) {
            txtEnunciado->setText(q.enunciado);
            txtResposta->setText(q.resposta);
            
            QString tagsHtml;
            for (const auto& tag : q.tags) {
                tagsHtml += QString("<span style='padding: 10px; border-radius: 10px; margin: 3px;'>%1</span> ").arg(tag);
            }
            lblTags->setText(tagsHtml);
            
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
        QString itemText;
        if (lista->viewMode() == QListView::IconMode) {

            // Modo grade
            itemText = QString("%1\n%2")
                .arg(q.enunciado.left(100) + (q.enunciado.length() > 100 ? "..." : ""))
                .arg(q.tags.join(", "));
        } else {
            // Modo lista
            itemText = QString("%1\n\n%2")
                .arg(q.enunciado.left(300) + (q.enunciado.length() > 300 ? "..." : ""))
                .arg(q.tags.join(", "));
        }
        
        QListWidgetItem* item = new QListWidgetItem(itemText, lista);

        item->setData(Qt::UserRole, q.id);
        item->setData(Qt::UserRole + 1, q.enunciado);
        item->setData(Qt::UserRole + 2, q.tags);
        item->setData(Qt::UserRole + 3, !q.resposta.trimmed().isEmpty());
        
        QString respostaPreview = q.resposta.trimmed().isEmpty() ? "Não respondida" : q.resposta.left(100) + "...";

        item->setToolTip(QString("%1\n%2\n\n%3\n%4")
            .arg(q.enunciado)
            .arg(respostaPreview)
            .arg(q.tags.join(", "))
            .arg(q.criadaEm.toString("dd/MM/yyyy - hh:mm")));
            
        
        /*
        if (!q.resposta.trimmed().isEmpty()) {
            item->setForeground(Qt::darkGreen);
            item->setBackground(QColor(240, 255, 240));
        } else {
            item->setForeground(Qt::darkRed);
            item->setBackground(QColor(255, 240, 240));
        }*/
        
        item->setTextAlignment(Qt::AlignCenter);
        
        // tamanho para itens no modo grid
        if (lista->viewMode() == QListView::IconMode) {
            item->setSizeHint(lista->gridSize());
        }
    }
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