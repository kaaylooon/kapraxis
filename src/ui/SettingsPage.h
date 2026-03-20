#ifndef KAPRAXIS_UI_SETTINGSPAGE_H_
#define KAPRAXIS_UI_SETTINGSPAGE_H_

#include <QString>
#include <QWidget>

class QButtonGroup;
class QLabel;
class QPushButton;
class QComboBox;

class SettingsPage : public QWidget {
    Q_OBJECT

   public:
    explicit SettingsPage(QWidget* parent = nullptr);

   signals:
    void themeChanged(const QString& themeId);
    void removeAllRequested();
    void importKeepRequested();
    void languageChanged(const QString& languageId);

   private:
    void loadCurrentTheme();
    void applyThemeSelection(const QString& themeId);
    void loadCurrentLanguage();
    void handleLanguageSelectionChanged(int index);

    QButtonGroup* themeGroup;
    QLabel* lblInfo;
    QComboBox* languageCombo = nullptr;
    QLabel* lblLanguageNote = nullptr;
    QString currentLanguageId = "en";
};

#endif  // KAPRAXIS_UI_SETTINGSPAGE_H_
