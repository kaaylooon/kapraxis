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
#include "../ui/BlocosPage.h"

AppWindow::AppWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    sidebar = new QListWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFocusPolicy(Qt::NoFocus);
    sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebar->setIconSize(QSize(32, 32));

    QStringList labels = {"", "", "", ""};
    QStringList iconPaths = {
        ":/resources/icons/house.svg",
        ":/resources/icons/list-ul.svg",
        ":/resources/icons/view-list.svg",
        ":/resources/icons/stopwatch.svg"
    };

    for (int i = 0; i < labels.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(sidebar);
        item->setIcon(QIcon(iconPaths[i]));
        item->setText(labels[i]);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    
    pages = new QStackedWidget;
    
    pages->addWidget(new InicioPage());
    pages->addWidget(new QuestoesPage());
    pages->addWidget(new QLabel("em desenvolvimento)"));
    pages->addWidget(new BlocosPage());
    
    mainLayout->addWidget(sidebar, 1);
    mainLayout->addWidget(pages, 10); 
    
    setCentralWidget(central);
    setWindowTitle("Kapraxis");
    resize(1280, 720);
    
    connect(sidebar, &QListWidget::currentRowChanged,
            pages, &QStackedWidget::setCurrentIndex);
    
    sidebar->setCurrentRow(0);

    carregarEstiloGlobal();
}

void AppWindow::carregarEstiloGlobal() {
    QString style = R"(

        *{
            font-size: 15px;
            font-family: 'Arial', sans-serif;
        }

        QWidget {
            background-color: #151515; /* Very Dark Gray */
            color: #e0e0e0; /* Light Gray for text */
        }
        
        /* ===== SIDEBAR ===== */
        QListWidget#sidebar {
            background-color: #2a2a2a; /* Black */
            border: none;
            color: #cccccc; /* Lighter Gray for sidebar text */
            font-size: 15px;
            font-weight: 500;
            border-radius: 6px;
            margin: 5px;
            margin-right: 0px;
            border-right: 1px solid #2a2a2a; /* Dark Gray border */
        }

        QListWidget::icon {
            margin-right: 12px;
        }

        QListWidget::item:selected QIcon {
            color: white;
        }

        QListWidget::item:hover QIcon {
            transform: scale(1.1);
            transition: transform 0.2s;
        }
        
        QListWidget#sidebar::item {
            padding: 14px 20px;
            margin: 1px;
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
            show-decoration-selected: 0;
            alternate-background-color: rgba(0, 0, 0, 0.05);
            outline: 0;
        }

        QListWidget::item {
            margin: 0;
            border-bottom: 1px solid rgba(62, 62, 62, 0.1); /* Subtle border from #3e3e3e */
            border-radius: 8px;
            background-color: rgba(62, 62, 62, 0.1);
            outline: none;
            selection-background-color: transparent;
        }

        QListWidget::item:hover {
            background-color: rgba(62, 62, 62, 0.2); /* Subtle hover from #3e3e3e */
        }

        QListWidget#lista::item:selected {
            background-color: #3e3e3e; /* Medium Dark Gray for selection */
            color: white;
            border: none;
            outline: none;
        }

        QListWidget#lista::item:selected:!active {
            background-color: #3e3e3e;
            color: white;
            border: none;
        }

        /* Adicionar estas propriedades para controlar o comportamento de foco */
        QListWidget#lista:focus {
            outline: none;
            border: none;
        }

        QListWidget#lista::item:focus {
            outline: none;
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

        QLabel#lblTags{
            background-color: #fff; color: #000;
            border-radius: 8px;
            padding: 5px;
        }

        QLabel#lblData{
            color: #666; font-size: 12px;
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
