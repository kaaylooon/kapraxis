#ifndef KAPRAXIS_REPO_QUESTAO_REPO_SQLITE_H_
#define KAPRAXIS_REPO_QUESTAO_REPO_SQLITE_H_

#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include "../domain/Questao.h"

namespace kapraxis {
namespace repo {

class QuestaoRepoSQLite {
public:
    QuestaoRepoSQLite();

    void Salvar(const Questao& qst);
    void Atualizar(const Questao& qst);
    void Excluir(int id);
    void Excluir(const Questao& qst);
    void ExcluirTodas();
    int Contar();

    Questao BuscarPorId(int id);
    QList<Questao> Listar();
    QList<Questao> ListarBasico();
    QList<Questao> ListarBasicoCached();
    static void InvalidarCacheBasico();
    //QList<Questao> buscarPorTag(const QString& tag);

private:
    void Init();
    QStringList ListarImagens(int questaoId, const QString& tipo);
    void SalvarImagens(int questaoId, const QStringList& paths, const QString& tipo);
    void SincronizarImagens(int questaoId, const QStringList& paths, const QString& tipo);
    void ExcluirImagens(int questaoId);
    
private:
    QSqlDatabase db_;  // Declare aqui
};

}  // namespace repo
}  // namespace kapraxis

#endif  // KAPRAXIS_REPO_QUESTAO_REPO_SQLITE_H_
