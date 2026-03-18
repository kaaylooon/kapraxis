#ifndef KAPRAXIS_APP_APPWINDOW_H_
#define KAPRAXIS_APP_APPWINDOW_H_

#include <QMainWindow>
#include <QString>
#include <QVector>

class QListWidget;
class QStackedWidget;
class InicioPage;
class BlocosPage;
class SettingsPage;
class QListWidgetItem;
class QuestoesPage;
class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void carregarEstiloGlobal();
    void aplicarTema(const QString& themeId);
    void atualizarSpacerSidebar();
    
private:
    QListWidget* sidebar;
    QStackedWidget* pages;
    InicioPage* inicioPage;
    QuestoesPage* questoesPage;
    SettingsPage* settingsPage;
    BlocosPage* blocosPage;
    QListWidgetItem* spacerItem;
    QVector<QListWidgetItem*> pageItems;
};

#endif  // KAPRAXIS_APP_APPWINDOW_H_
