// InicioPage.h
#ifndef INICIOPAGE_H
#define INICIOPAGE_H

#include <QWidget>
#include <QLabel>

class QFrame;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QShowEvent;
class QuestaoRepoSQLite;

class InicioPage : public QWidget
{
    Q_OBJECT
public:
    explicit InicioPage(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;

public slots:
    void atualizarResumo();

private:
    QuestaoRepoSQLite* repo;
    QLabel* lblTotalQuestoes;
    QLabel* lblHorasEstudo;
    QWidget* chartWidget;
};

#endif
