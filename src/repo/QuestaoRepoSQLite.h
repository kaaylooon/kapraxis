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
    void excluirTodas();
    int contar();

    Questao buscarPorId(int id);
    QList<Questao> listar();
    QList<Questao> listarBasico();
    QList<Questao> listarBasicoCached();
    static void invalidarCacheBasico();
    //QList<Questao> buscarPorTag(const QString& tag);

private:
    void init();
    QStringList listarImagens(int questaoId, const QString& tipo);
    void salvarImagens(int questaoId, const QStringList& paths, const QString& tipo);
    void sincronizarImagens(int questaoId, const QStringList& paths, const QString& tipo);
    void excluirImagens(int questaoId);
    
private:
    QSqlDatabase db;  // Declare aqui
};

#endif
