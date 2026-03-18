#include "AppWindow.h"
#include <QAbstractAnimation>
#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QEasingCurve>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QSettings>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "../ui/InicioPage.h"
#include "../ui/QuestoesPage.h"
#include "../ui/BlocosPage.h"
#include "../ui/SettingsPage.h"

AppWindow::AppWindow(QWidget* parent)
    : QMainWindow(parent)
{
    carregarIdioma();

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
    sidebar->setAccessibleName(tr("Main navigation"));
    sidebar->setAccessibleDescription(tr("Switches between the Kapraxis sections."));

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
    pagesOpacityEffect = new QGraphicsOpacityEffect(pages);
    pagesOpacityEffect->setOpacity(1.0);
    pages->setGraphicsEffect(pagesOpacityEffect);
    pages->setAccessibleName(tr("Main content"));
    pages->setAccessibleDescription(tr("Shows the overview, questions, blocks, and settings."));
    
    inicioPage = new InicioPage();
    pages->addWidget(inicioPage);
    questoesPage = new QuestoesPage();
    pages->addWidget(questoesPage);
    blocosPage = new BlocosPage();
    pages->addWidget(blocosPage);
    settingsPage = new SettingsPage();
    pages->addWidget(settingsPage);

    inicioPage->setAccessibleName(tr("Home page"));
    questoesPage->setAccessibleName(tr("Questions page"));
    blocosPage->setAccessibleName(tr("Blocks page"));
    settingsPage->setAccessibleName(tr("Settings page"));
    
    mainLayout->addWidget(sidebar, 1);
    mainLayout->addWidget(pages, 10); 
    
    setCentralWidget(central);
    setWindowTitle(tr("Kapraxis"));
    resize(1280, 720);
    
    connect(sidebar, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem*) {
        if (!current) return;
        const int pageIndex = current->data(Qt::UserRole).toInt();
        animatePageTransition(pageIndex);
    });

    {
        QSignalBlocker blocker(sidebar);
        sidebar->setCurrentItem(pageItems.isEmpty() ? nullptr : pageItems.first());
    }
    atualizarSpacerSidebar();

    carregarEstiloGlobal();

    connect(blocosPage, &BlocosPage::studyStatsUpdated,
            inicioPage, &InicioPage::atualizarResumo);

    connect(settingsPage, &SettingsPage::themeChanged, this, &AppWindow::aplicarTema);
    connect(settingsPage, &SettingsPage::removeAllRequested, questoesPage, &QuestoesPage::excluirTodasQuestoes);
    connect(settingsPage, &SettingsPage::importKeepRequested, questoesPage, &QuestoesPage::importarKeepJson);
    connect(settingsPage, &SettingsPage::languageChanged, this, &AppWindow::aplicarIdioma);
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
    QString normalizedTheme = themeId;
    if (normalizedTheme.isEmpty()) {
        normalizedTheme = "dark";
    }
    if (normalizedTheme == currentThemeId) {
        return;
    }

    QString path = ":/resources/styles/global.qss";
    if (normalizedTheme == "soft") {
        path = ":/resources/styles/theme-soft.qss";
    } else if (normalizedTheme == "light") {
        path = ":/resources/styles/theme-light.qss";
    } else if (normalizedTheme == "palette") {
        path = ":/resources/styles/theme-palette.qss";
    }

    currentThemeId = normalizedTheme;

    if (!isVisible()) {
        applyStyleSheet(path);
        return;
    }

    auto* fadeOut = new QPropertyAnimation(this, "windowOpacity", this);
    fadeOut->setDuration(180);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.65);
    fadeOut->setEasingCurve(QEasingCurve::InOutQuad);
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, path]() {
        applyStyleSheet(path);
        auto* fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
        fadeIn->setDuration(220);
        fadeIn->setStartValue(0.65);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void AppWindow::applyStyleSheet(const QString& path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }
    const QString style = QString::fromUtf8(file.readAll());
    qApp->setStyleSheet(style);
}

void AppWindow::animatePageTransition(int pageIndex) {
    if (!pages || pageIndex < 0 || pageIndex >= pages->count()) {
        return;
    }
    if (pageIndex == pages->currentIndex() || pageTransitionInProgress) {
        return;
    }

    if (!pagesOpacityEffect) {
        pagesOpacityEffect = new QGraphicsOpacityEffect(pages);
        pages->setGraphicsEffect(pagesOpacityEffect);
    }

    pageTransitionInProgress = true;
    auto* fadeOut = new QPropertyAnimation(pagesOpacityEffect, "opacity", this);
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.35);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, pageIndex]() {
        pages->setCurrentIndex(pageIndex);
        auto* fadeIn = new QPropertyAnimation(pagesOpacityEffect, "opacity", this);
        fadeIn->setDuration(240);
        fadeIn->setStartValue(0.35);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
            pageTransitionInProgress = false;
        });
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void AppWindow::carregarIdioma() {
    QSettings settings;
    const QString languageId = settings.value("ui/language", "en").toString();
    applyLanguage(languageId);
}

void AppWindow::applyLanguage(const QString& languageId) {
    if (languageId == currentLanguageId) {
        return;
    }
    qApp->removeTranslator(&translator_);
    if (languageId == "pt_BR") {
        if (translator_.load(":/translations/kapraxis_pt_BR.qm")) {
            qApp->installTranslator(&translator_);
        }
    }
    currentLanguageId = languageId;
}

void AppWindow::aplicarIdioma(const QString& languageId) {
    if (languageId == currentLanguageId) {
        return;
    }
    QSettings settings;
    settings.setValue("ui/language", languageId);
    applyLanguage(languageId);
    QMessageBox::information(this, tr("Restart required"),
                             tr("Restart Kapraxis to apply the selected language."));
}
