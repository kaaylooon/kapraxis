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

private slots:
    void adicionarQuestao();
    void editarQuestao();
    void excluirQuestao();
    void mostrarDetalhes(QListWidgetItem* item);
    void recarregar();
    void carregarEstilo();
    void alternarVisualizacao();
    void atualizarVisualizacao();
    void atualizarTamanhoGrid(int colunas);
    void filtrarQuestoes(int index);

private:
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
    
    QTextEdit* txtEnunciado;
    QTextEdit* txtResposta;

    QPushButton* btnToggleView;

    QRadioButton* rbtnListView;
    QRadioButton* rbtnGridView;
    QSpinBox* spinColumns;
    QComboBox* comboFilterTag;


    QFrame* rightFrame;
};

#endif