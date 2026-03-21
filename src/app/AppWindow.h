#ifndef KAPRAXIS_APP_APPWINDOW_H_
#define KAPRAXIS_APP_APPWINDOW_H_

#include <QGraphicsOpacityEffect>
#include <QMainWindow>
#include <QPropertyAnimation>
#include <QString>
#include <QTranslator>
#include <QVector>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class InicioPage;
class BlocosPage;
class SettingsPage;
class QuestoesPage;

class AppWindow : public QMainWindow {
    Q_OBJECT
   public:
    explicit AppWindow(QWidget* parent = nullptr);

   protected:
    void resizeEvent(QResizeEvent* event) override;

   private:
    void carregarEstiloGlobal();
    void aplicarTema(const QString& themeId);
    void atualizarSpacerSidebar();
    void animatePageTransition(int pageIndex);
    void applyStyleSheet(const QString& path);
    void recarregarEstilo();
    void carregarIdioma();
    void applyLanguage(const QString& languageId);
    void aplicarIdioma(const QString& languageId);

   private:
    QListWidget* sidebar = nullptr;
    QStackedWidget* pages = nullptr;
    InicioPage* inicioPage = nullptr;
    QuestoesPage* questoesPage = nullptr;
    SettingsPage* settingsPage = nullptr;
    BlocosPage* blocosPage = nullptr;
    QListWidgetItem* spacerItem = nullptr;
    QVector<QListWidgetItem*> pageItems;
    QGraphicsOpacityEffect* pagesOpacityEffect = nullptr;
    QString currentThemeId;
    QTranslator translator_;
    QString currentLanguageId = "en";
    bool pageTransitionInProgress = false;
};

#endif  // KAPRAXIS_APP_APPWINDOW_H_
    