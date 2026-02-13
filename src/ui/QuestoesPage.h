#pragma once
#ifndef QUESTOESPAGE_H
#define QUESTOESPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QComboBox>
#include <QShortcut>
#include <QTimer>

class QListWidget;
class QLabel;
class QTextEdit;
class QPushButton;
class QuestaoRepoSQLite;
class QFrame;

class QuestoesPage : public QWidget {
    Q_OBJECT
public:
    explicit QuestoesPage(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void adicionarQuestao();
    void editarQuestao();
    void excluirQuestao();

    void mostrarDetalhes(QListWidgetItem* item);

    void carregarEstilo();
    void salvarResposta();
    void filtrarQuestoes(int index);
    void buscarQuestoes(const QString& texto);
    void focarBusca();

private:
    void recarregar();
    void setupShortcuts();
    void destacarItem(QListWidgetItem* item);

    QuestaoRepoSQLite* repo;

    QListWidget* lista;
    QPushButton* btnAdd;
    QPushButton* btnEdit;
    QPushButton* btnDelete;
    
    QLabel* lblEnunciado;
    QLabel* lblResposta;
    QLabel* lblTags;
    QLabel* lblData;
    QLabel* lblTitle;
    QLabel* lblInfo;
    
    QTextEdit* txtEnunciado;
    QTextEdit* txtResposta;

    QLineEdit* txtBusca;

    QComboBox* comboFilterTag;

    QFrame* rightFrame;

    QShortcut* atalhoNovaQuestao;
    QShortcut* atalhoSalvar;
    QShortcut* atalhoBusca;
};

#endif