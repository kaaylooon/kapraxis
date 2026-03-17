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
    
    inicioPage = new InicioPage();
    pages->addWidget(inicioPage);
    pages->addWidget(new QuestoesPage());
    pages->addWidget(new QLabel("em desenvolvimento)"));
    blocosPage = new BlocosPage();
    pages->addWidget(blocosPage);
    
    mainLayout->addWidget(sidebar, 1);
    mainLayout->addWidget(pages, 10); 
    
    setCentralWidget(central);
    setWindowTitle("Kapraxis");
    resize(1280, 720);
    
    connect(sidebar, &QListWidget::currentRowChanged,
            pages, &QStackedWidget::setCurrentIndex);
    
    sidebar->setCurrentRow(0);

    carregarEstiloGlobal();

    connect(blocosPage, &BlocosPage::studyStatsUpdated,
            inicioPage, &InicioPage::atualizarResumo);
}

void AppWindow::carregarEstiloGlobal() {
    QFile file(":/resources/styles/global.qss");
    if (file.open(QFile::ReadOnly)) {
        QString style = QLatin1String(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
    }
}
