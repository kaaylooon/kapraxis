#include "AppWindow.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QFrame>
#include <QApplication>

#include "../ui/InicioPage.h"
#include "../ui/QuestoesPage.h"

AppWindow::AppWindow(QWidget* parent)
    : QMainWindow(parent)
{
    carregarEstiloGlobal();
    
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // === SIDEBAR SIMPLES E BONITA ===
    sidebar = new QListWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(170);  // Largura fixa
    sidebar->setFocusPolicy(Qt::NoFocus);
    sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Adicionar todos os itens
    QStringList items = {"Início", "Questões", "Revisão", "Blocos"};
    for (const QString& item : items) {
        QListWidgetItem* listItem = new QListWidgetItem(item, sidebar);
        listItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    
    // === ÁREA DE CONTEÚDO ===
    pages = new QStackedWidget;
    
    // Criar páginas
    pages->addWidget(new InicioPage);
    pages->addWidget(new QuestoesPage);
    pages->addWidget(new QLabel("Revisão (em desenvolvimento)"));
    pages->addWidget(new QLabel("Blocos (em desenvolvimento)"));
    
    // Adicionar à layout principal
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pages, 1);  // Expande para preencher
    
    setCentralWidget(central);
    setWindowTitle("Kasty");
    resize(1280, 720);
    
    connect(sidebar, &QListWidget::currentRowChanged,
            pages, &QStackedWidget::setCurrentIndex);
    
    sidebar->setCurrentRow(0);
}

void AppWindow::carregarEstiloGlobal() {
    QString style = R"(
        QWidget {
            background-color: #151515; /* Very Dark Gray */
            color: #e0e0e0; /* Light Gray for text */
            font-family: 'Segoe UI', 'Roboto', sans-serif;
            font-size: 15px;
        }
        
        /* ===== SIDEBAR ===== */
        QListWidget#sidebar {
            background-color: #2a2a2a; /* Black */
            border: none;
            color: #cccccc; /* Lighter Gray for sidebar text */
            font-size: 16px;
            font-weight: 500;
            border-radius: 6px;
            margin: 5px;
            margin-right: 0px;
            border-right: 1px solid #2a2a2a; /* Dark Gray border */
        }
        
        QListWidget#sidebar::item {
            padding: 14px 20px;
            border-bottom: 1px solid rgba(62, 62, 62, 0.3); /* Subtle border from #3e3e3e */
            background-color: transparent;
        }
        
        QListWidget#sidebar::item:selected {
            background-color: #3e3e3e; /* Medium Dark Gray for selection */
            color: white;
            border-left: 4px solid #535353; /* Medium Gray accent */
            font-weight: bold;
        }
        
        QListWidget#sidebar::item:hover:!selected {
            background-color: rgba(62, 62, 62, 0.2); /* Subtle hover from #3e3e3e */
            color: #e0e0e0;
        }
        
        /* ===== CONTENT AREA ===== */
        QStackedWidget {
            background-color: #2a2a2a; /* Dark Gray */
            border-radius: 6px;
            margin: 5px;
            border: 1px solid #3e3e3e; /* Medium Dark Gray border */
        }
        
        /* Generic Labels */
        QLabel {
            color: #e0e0e0;
            font-size: 14px;
            background-color: transparent;
        }
        
        /* ===== REMOVE SCROLLBARS ===== */
        QScrollBar:vertical,
        QScrollBar:horizontal {
            width: 0px;
            height: 0px;
            background: transparent;
        }
        
        /* ===== BUTTONS ===== */
        QPushButton {
            background-color: #3e3e3e; /* Medium Dark Gray */
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
            font-size: 13px;
            min-width: 80px;
        }
        
        QPushButton:hover {
            background-color: #535353; /* Medium Gray */
        }
        
        QPushButton:pressed {
            background-color: #2a2a2a; /* Dark Gray */
        }
        
        /* ===== INPUTS ===== */
        QLineEdit, QTextEdit {
            background-color: rgba(0, 0, 0, 0.2); /* Subtle black background */
            border: 1px solid #3e3e3e; /* Medium Dark Gray border */
            border-radius: 6px;
            padding: 8px;
            color: #e0e0e0;
            selection-background-color: #535353; /* Medium Gray for selection */
        }
        
        QLineEdit:focus, QTextEdit:focus {
            border: 1px solid #535353; /* Medium Gray focus border */
            background-color: rgba(0, 0, 0, 0.3);
        }
        
        QTextEdit {
            background-color: rgba(0, 0, 0, 0.1);
        }
        
        /* ===== GROUPBOX ===== */
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            border: 1px solid #3e3e3e; /* Medium Dark Gray border */
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 12px;
            background-color: #2a2a2a; /* Dark Gray */
            color: #cccccc;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px 0 8px;
            color: #cccccc;
        }
        
        /* ===== LISTS ===== */
        QListWidget {
            background-color: rgba(0, 0, 0, 0.1); /* Subtle black background */
            border: none;
            border-radius: 8px;
            padding: 7px;
            color: #ddd;
            alternate-background-color: rgba(0, 0, 0, 0.05);
        }
        
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid rgba(62, 62, 62, 0.1); /* Subtle border from #3e3e3e */
            border-radius: 4px;
            background-color: transparent;
        }
        
        QListWidget::item:hover {
            background-color: rgba(62, 62, 62, 0.1); /* Subtle hover from #3e3e3e */
        }
        
        QListWidget::item:selected {
            background-color: #3e3e3e; /* Medium Dark Gray for selection */
            color: white;
            border: none;
        }
        
        /* ===== FRAMES ===== */
        QFrame#shadowFrame {
            background-color: #2a2a2a; /* Dark Gray */
            border-radius: 8px;
            border: 1px solid #3e3e3e; /* Medium Dark Gray border */
        }
        
        /* ===== PLACEHOLDER TEXT ===== */
        QTextEdit::placeholder-text {
            color: rgba(224, 224, 224, 0.5);
        }
        
        /* ===== SPECIFIC BUTTONS (Neutralized for minimalism) ===== */
        QPushButton#btnAdd {
            background-color: #3e3e3e; /* Medium Dark Gray */
        }
        
        QPushButton#btnAdd:hover {
            background-color: #535353;
        }
        
        QPushButton#btnEdit {
            background-color: #3e3e3e; /* Medium Dark Gray */
        }
        
        QPushButton#btnEdit:hover {
            background-color: #535353;
        }
        
        QPushButton#btnDelete {
            background-color: #3e3e3e; /* Medium Dark Gray */
        }
        
        QPushButton#btnDelete:hover {
            background-color: #535353;
        }

        QWidget#buttonContainer{
            background-color: #2a2a2a;
        }
    )";
    
    qApp->setStyleSheet(style);
}
