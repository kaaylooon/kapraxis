#include "SettingsPage.h"

#include <QButtonGroup>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto* title = new QLabel("Configuracoes");
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    auto* subtitle = new QLabel("Personalize a aparencia do Kapraxis");
    subtitle->setObjectName("pageSubtitle");
    layout->addWidget(subtitle);

    auto* appearanceGroup = new QGroupBox("Aparencia");
    auto* appearanceLayout = new QVBoxLayout(appearanceGroup);
    appearanceLayout->setSpacing(10);

    auto* lblTema = new QLabel("Tema");
    appearanceLayout->addWidget(lblTema);

    themeGroup = new QButtonGroup(this);
    themeGroup->setExclusive(true);

    auto addThemeCard = [&](const QString& title,
                            const QString& subtitleText,
                            const QString& themeId,
                            const QStringList& swatches) {
        auto* card = new QPushButton();
        card->setCheckable(true);
        card->setObjectName("themeCard");
        card->setProperty("themeId", themeId);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 12, 14, 12);
        cardLayout->setSpacing(8);

        auto* titleLabel = new QLabel(title);
        titleLabel->setObjectName("themeCardTitle");
        titleLabel->setWordWrap(true);
        auto* subLabel = new QLabel(subtitleText);
        subLabel->setObjectName("themeCardSubtitle");
        subLabel->setWordWrap(true);

        auto* swatchRow = new QHBoxLayout();
        swatchRow->setSpacing(6);
        for (const auto& color : swatches) {
            auto* sw = new QFrame();
            sw->setObjectName("themeSwatch");
            sw->setStyleSheet(QString("background-color:%1;").arg(color));
            sw->setFixedSize(16, 16);
            swatchRow->addWidget(sw);
        }
        swatchRow->addStretch();

        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(subLabel);
        cardLayout->addLayout(swatchRow);

        themeGroup->addButton(card);
        appearanceLayout->addWidget(card);

        connect(card, &QPushButton::clicked, [this, themeId]() {
            applyThemeSelection(themeId);
        });
    };

    addThemeCard("Dark", "Contraste alto, foco total", "dark",
                 {"#151515", "#2a2a2a", "#3e3e3e", "#e0e0e0"});
    addThemeCard("Soft", "Cinzas suaves e discretos", "soft",
                 {"#1b1c1e", "#26282b", "#35383d", "#d0d0d0"});
    addThemeCard("Light", "Visual claro e leve", "light",
                 {"#f2f2f2", "#e6e6e6", "#d4d4d4", "#1f1f1f"});
    addThemeCard("Palette", "Tema inspirado na paleta ayu", "palette",
                 {"#0D1017", "#10141C", "#E6C08A", "#39BAE6"});
    
    appearanceLayout->addSpacing(6);
    lblInfo = new QLabel("As mudancas sao aplicadas imediatamente.");
    appearanceLayout->addWidget(lblInfo);

    layout->addWidget(appearanceGroup);

    auto* dangerGroup = new QGroupBox("Zona de risco");
    auto* dangerLayout = new QVBoxLayout(dangerGroup);
    auto* dangerText = new QLabel("Acoes irreversiveis");
    dangerLayout->addWidget(dangerText);

    auto* btnImportKeep = new QPushButton("Importar Keep (JSON)");
    btnImportKeep->setObjectName("btnImportKeep");
    btnImportKeep->setCursor(Qt::PointingHandCursor);
    dangerLayout->addWidget(btnImportKeep);
    connect(btnImportKeep, &QPushButton::clicked, this, &SettingsPage::importKeepRequested);

    auto* btnRemoveAll = new QPushButton("Apagar todas as questoes");
    btnRemoveAll->setObjectName("btnRemoveAll");
    btnRemoveAll->setCursor(Qt::PointingHandCursor);
    dangerLayout->addWidget(btnRemoveAll);
    connect(btnRemoveAll, &QPushButton::clicked, this, &SettingsPage::removeAllRequested);

    layout->addWidget(dangerGroup);

    layout->addStretch();

    loadCurrentTheme();
}

void SettingsPage::loadCurrentTheme() {
    QSettings settings;
    const QString themeId = settings.value("ui/theme", "dark").toString();
    applyThemeSelection(themeId);
}

void SettingsPage::applyThemeSelection(const QString& themeId) {
    const auto buttons = themeGroup->buttons();
    for (auto* btn : buttons) {
        if (btn->property("themeId").toString() == themeId) {
            btn->setChecked(true);
        }
    }
    QSettings settings;
    settings.setValue("ui/theme", themeId);
    emit themeChanged(themeId);
}
