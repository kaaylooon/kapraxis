#ifndef KAPRAXIS_UI_SETTINGSPAGE_H_
#define KAPRAXIS_UI_SETTINGSPAGE_H_

#include <QString>
#include <QWidget>

class QButtonGroup;
class QLabel;
class QPushButton;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

signals:
    void themeChanged(const QString& themeId);
    void removeAllRequested();
    void importKeepRequested();

private:
    void loadCurrentTheme();
    void applyThemeSelection(const QString& themeId);

    QButtonGroup* themeGroup;
    QLabel* lblInfo;
};

#endif  // KAPRAXIS_UI_SETTINGSPAGE_H_
