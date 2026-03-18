#include "SettingsPage.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto* title = new QLabel(tr("Settings"));
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    auto* subtitle = new QLabel(tr("Personalize the Kapraxis appearance"));
    subtitle->setObjectName("pageSubtitle");
    layout->addWidget(subtitle);

    auto* appearanceGroup = new QGroupBox(tr("Appearance"));
    auto* appearanceLayout = new QVBoxLayout(appearanceGroup);
    appearanceLayout->setSpacing(10);

    auto* lblTema = new QLabel(tr("Theme"));
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

    addThemeCard(tr("Dark"), tr("High contrast focus mode"), "dark",
                 {"#151515", "#2a2a2a", "#3e3e3e", "#e0e0e0"});
    addThemeCard(tr("Soft"), tr("Soft gray palette"), "soft",
                 {"#1b1c1e", "#26282b", "#35383d", "#d0d0d0"});
    addThemeCard(tr("Light"), tr("Clean and bright interface"), "light",
                 {"#f2f2f2", "#e6e6e6", "#d4d4d4", "#1f1f1f"});
    addThemeCard(tr("Palette"), tr("Palette-inspired appearance"), "palette",
                 {"#0D1017", "#10141C", "#E6C08A", "#39BAE6"});
    
    appearanceLayout->addSpacing(6);
    lblInfo = new QLabel(tr("Theme changes are applied instantly."));
    appearanceLayout->addWidget(lblInfo);
    lblInfo->setAccessibleName(tr("Immediate application notice"));

    layout->addWidget(appearanceGroup);

    auto* languageGroup = new QGroupBox(tr("Language"));
    auto* languageLayout = new QVBoxLayout(languageGroup);
    languageLayout->setSpacing(6);

    languageCombo = new QComboBox();
    languageCombo->addItem(tr("English"), "en");
    languageCombo->addItem(tr("Portuguese"), "pt_BR");
    languageCombo->setAccessibleName(tr("Language selector"));
    languageCombo->setCursor(Qt::PointingHandCursor);

    lblLanguageNote = new QLabel(tr("Language changes require restarting the app."));
    lblLanguageNote->setWordWrap(true);
    lblLanguageNote->setAccessibleName(tr("Language change notice"));

    languageLayout->addWidget(languageCombo);
    languageLayout->addWidget(lblLanguageNote);
    layout->addWidget(languageGroup);

    connect(languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPage::handleLanguageSelectionChanged);
    loadCurrentLanguage();

    auto* dangerGroup = new QGroupBox(tr("Danger zone"));
    auto* dangerLayout = new QVBoxLayout(dangerGroup);
    auto* dangerText = new QLabel(tr("Irreversible actions"));
    dangerLayout->addWidget(dangerText);

    auto* btnImportKeep = new QPushButton(tr("Import Keep (JSON)"));
    btnImportKeep->setObjectName("btnImportKeep");
    btnImportKeep->setCursor(Qt::PointingHandCursor);
    dangerLayout->addWidget(btnImportKeep);
    btnImportKeep->setAccessibleName(tr("Import Keep notes"));
    connect(btnImportKeep, &QPushButton::clicked, this, &SettingsPage::importKeepRequested);

    auto* btnRemoveAll = new QPushButton(tr("Delete all questions"));
    btnRemoveAll->setObjectName("btnRemoveAll");
    btnRemoveAll->setCursor(Qt::PointingHandCursor);
    dangerLayout->addWidget(btnRemoveAll);
    btnRemoveAll->setAccessibleName(tr("Delete all questions"));
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

void SettingsPage::loadCurrentLanguage() {
    if (!languageCombo) return;
    QSettings settings;
    const QString languageId = settings.value("ui/language", "en").toString();
    const int index = languageCombo->findData(languageId);
    if (index >= 0) {
        currentLanguageId = languageId;
        languageCombo->blockSignals(true);
        languageCombo->setCurrentIndex(index);
        languageCombo->blockSignals(false);
    }
}

void SettingsPage::handleLanguageSelectionChanged(int index) {
    if (!languageCombo) return;
    const QString languageId = languageCombo->itemData(index).toString();
    if (languageId.isEmpty() || languageId == currentLanguageId) {
        return;
    }
    currentLanguageId = languageId;
    QSettings settings;
    settings.setValue("ui/language", languageId);
    emit languageChanged(languageId);
    QMessageBox::information(this, tr("Restart required"),
                             tr("Restart Kapraxis to apply the selected language."));
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
