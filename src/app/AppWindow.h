#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>

class QListWidget;
class QStackedWidget;

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget* parent = nullptr);
    
private:
    void carregarEstiloGlobal();
    
private:
    QListWidget* sidebar;
    QStackedWidget* pages;
};

#endif