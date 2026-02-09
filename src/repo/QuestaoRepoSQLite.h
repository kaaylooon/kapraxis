#ifndef QUESTAOREPSQLITE_H
#define QUESTAOREPSQLITE_H

#include <QList>
#include <QSqlDatabase>
#include "../domain/Questao.h"


class QuestaoRepoSQLite {
public:
    QuestaoRepoSQLite();

    void salvar(const Questao& qst);
    void atualizar(const Questao& qst);
    void excluir(int id);
    void excluir(const Questao& qst);

    Questao buscarPorId(int id);
    QList<Questao> listar();
    //QList<Questao> buscarPorTag(const QString& tag);

private:
    void init();
    
private:
    QSqlDatabase db;  // Declare aqui
};

#endif