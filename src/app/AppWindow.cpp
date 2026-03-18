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
#include <QSettings>

#include "../ui/InicioPage.h"
#include "../ui/QuestoesPage.h"
#include "../ui/BlocosPage.h"
#include "../ui/SettingsPage.h"

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

    QStringList labelsTop = {"", "", ""};
    QStringList labelsBottom = {""};
    QStringList topIcons = {
        ":/resources/icons/house.svg",
        ":/resources/icons/list-ul.svg",
        ":/resources/icons/stopwatch.svg"
    };
    QStringList bottomIcons = {
        ":/resources/icons/bookmark.svg"
    };

    const int itemHeight = 54;
    auto addPageItem = [&](const QString& text, const QString& iconPath, int pageIndex) {
        auto* item = new QListWidgetItem(sidebar);
        item->setIcon(QIcon(iconPath));
        item->setText(text);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setData(Qt::UserRole, pageIndex);
        item->setSizeHint(QSize(1, itemHeight));
        pageItems.append(item);
    };

    for (int i = 0; i < labelsTop.size(); ++i) {
        addPageItem(labelsTop[i], topIcons[i], i);
    }

    spacerItem = new QListWidgetItem(sidebar);
    spacerItem->setFlags(Qt::NoItemFlags);
    spacerItem->setData(Qt::UserRole, -1);

    for (int i = 0; i < labelsBottom.size(); ++i) {
        const int pageIndex = labelsTop.size() + i;
        addPageItem(labelsBottom[i], bottomIcons[i], pageIndex);
    }
    
    pages = new QStackedWidget;
    
    inicioPage = new InicioPage();
    pages->addWidget(inicioPage);
    questoesPage = new QuestoesPage();
    pages->addWidget(questoesPage);
    blocosPage = new BlocosPage();
    pages->addWidget(blocosPage);
    settingsPage = new SettingsPage();
    pages->addWidget(settingsPage);
    
    mainLayout->addWidget(sidebar, 1);
    mainLayout->addWidget(pages, 10); 
    
    setCentralWidget(central);
    setWindowTitle("Kapraxis");
    resize(1280, 720);
    
    connect(sidebar, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem*) {
        if (!current) return;
        const int pageIndex = current->data(Qt::UserRole).toInt();
        if (pageIndex < 0 || pageIndex >= pages->count()) return;
        pages->setCurrentIndex(pageIndex);
    });
    
    sidebar->setCurrentItem(pageItems.isEmpty() ? nullptr : pageItems.first());
    atualizarSpacerSidebar();

    carregarEstiloGlobal();

    connect(blocosPage, &BlocosPage::studyStatsUpdated,
            inicioPage, &InicioPage::atualizarResumo);

    connect(settingsPage, &SettingsPage::themeChanged, this, &AppWindow::aplicarTema);
    connect(settingsPage, &SettingsPage::removeAllRequested, questoesPage, &QuestoesPage::excluirTodasQuestoes);
    connect(settingsPage, &SettingsPage::importKeepRequested, questoesPage, &QuestoesPage::importarKeepJson);
}

void AppWindow::carregarEstiloGlobal() {
    QSettings settings;
    const QString themeId = settings.value("ui/theme", "dark").toString();
    aplicarTema(themeId);
}

void AppWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    atualizarSpacerSidebar();
}

void AppWindow::atualizarSpacerSidebar() {
    if (!spacerItem || pageItems.isEmpty()) return;
    const int itemHeight = 54;
    const int splitIndex = 3;
    const int topHeight = itemHeight * splitIndex;
    const int bottomHeight = itemHeight * (pageItems.size() - splitIndex);
    const int available = sidebar->viewport()->height();
    const int spacerHeight = qMax(0, available - topHeight - bottomHeight - 8);
    spacerItem->setSizeHint(QSize(1, spacerHeight));
}

void AppWindow::aplicarTema(const QString& themeId) {
    QString path = ":/resources/styles/global.qss";
    if (themeId == "soft") {
        path = ":/resources/styles/theme-soft.qss";
    } else if (themeId == "light") {
        path = ":/resources/styles/theme-light.qss";
    } else if (themeId == "palette") {
        path = ":/resources/styles/theme-palette.qss";
    }

    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QString style = QLatin1String(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
    }
}
