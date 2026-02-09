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

#include "../repo/QuestaoRepoSQLite.h"

QuestoesPage::QuestoesPage(QWidget* parent)
    : QWidget(parent)
{
    repo = new QuestaoRepoSQLite;
    
    //carregarEstilo();

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);
    
    // Painel esquerdo
    auto* leftPanel = new QVBoxLayout();
    leftPanel->setContentsMargins(0, 0, 0, 0);
    leftPanel->setSpacing(15);

    // Layout de filtros (sem QGroupBox)
    auto* filterLayout = new QHBoxLayout();
    
    // Rótulo
    auto* filterLabel = new QLabel("Filtros:");
    filterLabel->setStyleSheet("font-weight: bold;");
    filterLayout->addWidget(filterLabel);
    
    // Botões de rádio para visualização
    rbtnListView = new QRadioButton("Lista");
    rbtnGridView = new QRadioButton("Grade");
    rbtnGridView->setChecked(true);
    filterLayout->addWidget(rbtnListView);
    filterLayout->addWidget(rbtnGridView);
    
    // Controle de colunas
    filterLayout->addWidget(new QLabel("Colunas:"));
    spinColumns = new QSpinBox();
    spinColumns->setRange(1, 6);
    spinColumns->setValue(3);
    spinColumns->setEnabled(rbtnGridView->isChecked());
    spinColumns->setFixedWidth(50);
    filterLayout->addWidget(spinColumns);
    
    // Combo box de filtro
    filterLayout->addWidget(new QLabel("Status:"));
    comboFilterTag = new QComboBox();
    comboFilterTag->addItem("Todas", "");
    comboFilterTag->addItem("Sem resposta", "sem_resposta");
    comboFilterTag->addItem("Com resposta", "com_resposta");
    comboFilterTag->setFixedWidth(120);
    filterLayout->addWidget(comboFilterTag);
    
    // Espaçador
    filterLayout->addStretch();
    
    // Botão de alternar visualização
    btnToggleView = new QPushButton("Alternar");
    btnToggleView->setObjectName("btnToggle");
    btnToggleView->setCursor(Qt::PointingHandCursor);
    btnToggleView->setFixedWidth(80);
    filterLayout->addWidget(btnToggleView);
    
    leftPanel->addLayout(filterLayout);

    // Lista/Grid de questões
    lista = new QListWidget();
    lista->setSelectionMode(QAbstractItemView::SingleSelection);
    lista->setViewMode(QListView::IconMode);
    lista->setResizeMode(QListView::Adjust);
    lista->setGridSize(QSize(200, 150));
    lista->setSpacing(10);
    lista->setWordWrap(true);
    lista->setUniformItemSizes(true);

    leftPanel->addWidget(lista, 1);
    
    // Painel direito
    rightFrame = new QFrame();
    rightFrame->setObjectName("shadowFrame");
    auto* rightPanel = new QVBoxLayout();
    rightPanel->setContentsMargins(10, 10, 10, 10);
    rightPanel->setSpacing(15);
    rightPanel->setAlignment(Qt::AlignCenter);

    auto* detailsGroup = new QGroupBox("Detalhes da Questão"); 
    detailsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");

    auto* formLayout = new QFormLayout(detailsGroup);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setVerticalSpacing(10);
    formLayout->setHorizontalSpacing(15);
    
    lblEnunciado = new QLabel("Enunciado");
    lblEnunciado->setStyleSheet("font-weight: bold; font-size: 16px;");

    txtEnunciado = new QTextEdit();
    txtEnunciado->setWordWrapMode(QTextOption::WordWrap);
    txtEnunciado->setMinimumWidth(300);
    txtEnunciado->setMaximumHeight(100);

    lblResposta = new QLabel("Resposta");
    lblResposta->setStyleSheet("font-weight: bold; font-size: 16px;");

    txtResposta = new QTextEdit();
    txtResposta->setPlaceholderText("Digite a resposta...");
    txtResposta->setMinimumHeight(150);
    
    lblTags = new QLabel();
    lblTags->setStyleSheet("border-radius: 6px; border: 1px solid #ddd; padding: 4px;");

    lblData = new QLabel();
    lblData->setStyleSheet("color: #666; font-size: 12px;");
        
    formLayout->addRow(lblEnunciado);
    formLayout->addRow(txtEnunciado);
    formLayout->addRow(lblResposta);
    formLayout->addRow(txtResposta);
    formLayout->addRow(new QLabel("Tags:"), lblTags);
    formLayout->addRow(new QLabel("Criada em:"), lblData);

    rightPanel->addWidget(detailsGroup);
    
    // Botões
    auto* buttonContainer = new QWidget();
    buttonContainer->setObjectName("buttonContainer");
    auto* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 20, 0, 0);
    buttonLayout->setSpacing(8);

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

    rightPanel->addWidget(buttonContainer);
    
    // Layout principal
    mainLayout->addLayout(leftPanel, 2);
    mainLayout->addLayout(rightPanel, 1);
    
    // Conectar sinais
    connect(btnAdd, &QPushButton::clicked,
            this, &QuestoesPage::adicionarQuestao);
    connect(btnEdit, &QPushButton::clicked,
            this, &QuestoesPage::editarQuestao);
    connect(btnDelete, &QPushButton::clicked,
            this, &QuestoesPage::excluirQuestao);
    connect(btnToggleView, &QPushButton::clicked,
            this, &QuestoesPage::alternarVisualizacao);
    connect(lista, &QListWidget::itemClicked,
            this, &QuestoesPage::mostrarDetalhes);
    connect(rbtnListView, &QRadioButton::toggled,
            this, &QuestoesPage::atualizarVisualizacao);
    connect(rbtnGridView, &QRadioButton::toggled,
            this, &QuestoesPage::atualizarVisualizacao);
    connect(spinColumns, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &QuestoesPage::atualizarTamanhoGrid);
    connect(comboFilterTag, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestoesPage::filtrarQuestoes);
    connect(txtResposta, &QTextEdit::textChanged, [this]() {
        QListWidgetItem* item = lista->currentItem();
        if (item) {
            int id = item->data(Qt::UserRole).toInt();
            // Aqui você implementaria a atualização no banco
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
            
            // Formatar tags
            QString tagsHtml;
            for (const auto& tag : q.tags) {
                tagsHtml += QString("<span style='background-color: #e0e0e0; padding: 2px 6px; border-radius: 10px; margin: 2px; display: inline-block;'>%1</span> ").arg(tag);
            }
            lblTags->setText(tagsHtml);
            
            lblData->setText(q.criadaEm.toString("dd/MM/yyyy hh:mm"));
            break;
        }
    }
}

void QuestoesPage::alternarVisualizacao() {
    if (lista->viewMode() == QListView::IconMode) {
        lista->setViewMode(QListView::ListMode);
        spinColumns->setEnabled(false);
    } else {
        lista->setViewMode(QListView::IconMode);
        spinColumns->setEnabled(true);
    }
    recarregar();
}

void QuestoesPage::atualizarVisualizacao() {
    if (rbtnListView->isChecked()) {
        lista->setViewMode(QListView::ListMode);
        lista->setGridSize(QSize());
        spinColumns->setEnabled(false);
    } else {
        lista->setViewMode(QListView::IconMode);
        atualizarTamanhoGrid(spinColumns->value());
        spinColumns->setEnabled(true);
    }
    recarregar();
}

void QuestoesPage::atualizarTamanhoGrid(int colunas) {
    int largura = lista->width() / colunas - 20;
    if (largura < 100) largura = 100; // Largura mínima
    lista->setGridSize(QSize(largura, 150));
    lista->update();
}

void QuestoesPage::filtrarQuestoes(int index) {
    recarregar();
}

void QuestoesPage::recarregar() {
    lista->clear();
    
    // Aplicar filtros
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
    
    // Adicionar questões
    for (auto& q : questoesFiltradas) {
        QString itemText;
        if (lista->viewMode() == QListView::IconMode) {
            // Modo grade - texto compacto
            itemText = QString("#%1\n%2")
                .arg(q.id)
                .arg(q.enunciado.left(50) + (q.enunciado.length() > 50 ? "..." : ""));
        } else {
            // Modo lista - texto completo
            itemText = QString("#%1 - %2\nTags: %3")
                .arg(q.id)
                .arg(q.enunciado.left(80))
                .arg(q.tags.join(", "));
        }
        
        QListWidgetItem* item = new QListWidgetItem(itemText, lista);
        item->setData(Qt::UserRole, q.id);
        
        // Tooltip
        item->setToolTip(QString("ID: %1\nEnunciado: %2\nTags: %3\nCriada: %4\nResposta: %5")
            .arg(q.id)
            .arg(q.enunciado)
            .arg(q.tags.join(", "))
            .arg(q.criadaEm.toString("dd/MM/yyyy hh:mm"))
            .arg(q.resposta.isEmpty() ? "Não respondida" : q.resposta.left(100) + "..."));
        
        /*
        if (!q.resposta.trimmed().isEmpty()) {
            item->setForeground(Qt::darkGreen);
            item->setBackground(QColor(240, 255, 240));
        } else {
            item->setForeground(Qt::darkRed);
            item->setBackground(QColor(255, 240, 240));
        }*/
        
        item->setTextAlignment(Qt::AlignCenter);
        
        // Definir tamanho para itens no modo grid
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