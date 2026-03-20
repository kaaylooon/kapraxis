#include "questao_repo_sqlite.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace kapraxis {
namespace repo {

namespace {
QList<Questao> s_cacheBasico;
bool s_cacheBasicoValid = false;
}  // namespace

QuestaoRepoSQLite::QuestaoRepoSQLite() {
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataDir.isEmpty()) {
        QDir().mkpath(appDataDir);
    }

    const QString appDataDbPath =
        appDataDir.isEmpty() ? QString("questoes.db") : QDir(appDataDir).filePath("questoes.db");

    if (appDataDbPath != "questoes.db" && !QFile::exists(appDataDbPath) &&
        QFile::exists("questoes.db")) {
        QFile::copy("questoes.db", appDataDbPath);
    }

    if (QSqlDatabase::contains("kapraxis_connection")) {
        db_ = QSqlDatabase::database("kapraxis_connection");

        if (!db_.isOpen()) {
            if (!db_.open()) {
                qDebug() << "Error reopening database:" << db_.lastError().text();
                return;
            }
        }

    } else {
        db_ = QSqlDatabase::addDatabase("QSQLITE", "kapraxis_connection");
        db_.setDatabaseName(appDataDbPath);

        if (!db_.open()) {
            qDebug() << "Error opening database:" << db_.lastError().text();
            return;
        }

        QSqlQuery pragma(db_);
        pragma.exec("PRAGMA foreign_keys = ON");

        Init();
    }
}
void QuestaoRepoSQLite::Init() {
    QSqlQuery q(db_);
    q.exec(
        "CREATE TABLE IF NOT EXISTS questoes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "enunciado TEXT,"
        "resposta TEXT,"
        "tags TEXT,"
        "criada_em TEXT)");

    q.exec(
        "CREATE TABLE IF NOT EXISTS questao_imagens ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "questao_id INTEGER NOT NULL,"
        "tipo TEXT NOT NULL,"
        "path TEXT NOT NULL,"
        "ordem INTEGER,"
        "criada_em TEXT,"
        "FOREIGN KEY(questao_id) REFERENCES questoes(id) ON DELETE CASCADE)");
}

void QuestaoRepoSQLite::Atualizar(const Questao& qst) {
    if (qst.id <= 0) {
        qDebug() << "Invalid ID for update";
        return;
    }

    QSqlQuery q(db_);
    q.prepare(
        "UPDATE questoes SET "
        "enunciado = :enunciado, "
        "resposta = :resposta, "
        "tags = :tags, "
        "criada_em = :criada_em "
        "WHERE id = :id");

    q.bindValue(":id", qst.id);
    q.bindValue(":enunciado", qst.enunciado);
    q.bindValue(":resposta", qst.resposta);
    q.bindValue(":tags", qst.tags.join(","));
    q.bindValue(":criada_em", qst.criadaEm.toString(Qt::ISODate));

    if (!q.exec()) {
        qDebug() << "Error updating question:" << q.lastError().text();
        return;
    }

    SincronizarImagens(qst.id, qst.enunciadoImagens, "enunciado");
    SincronizarImagens(qst.id, qst.respostaImagens, "resposta");
}

void QuestaoRepoSQLite::Salvar(const Questao& qst) {
    QSqlQuery q(db_);
    q.prepare("INSERT INTO questoes (enunciado, resposta, tags, criada_em) VALUES (?, ?, ?, ?)");
    q.addBindValue(qst.enunciado);
    q.addBindValue(qst.resposta);
    q.addBindValue(qst.tags.join(","));
    q.addBindValue(qst.criadaEm.toString(Qt::ISODate));
    if (!q.exec()) {
        qDebug() << "Error saving question:" << q.lastError().text();
        return;
    }

    const QVariant newIdVar = q.lastInsertId();
    if (newIdVar.isValid()) {
        const int newId = newIdVar.toInt();
        SalvarImagens(newId, qst.enunciadoImagens, "enunciado");
        SalvarImagens(newId, qst.respostaImagens, "resposta");
    }
}

Questao QuestaoRepoSQLite::BuscarPorId(int id) {
    QSqlQuery q(db_);
    q.prepare("SELECT * FROM questoes WHERE id = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qDebug() << "Error fetching question:" << q.lastError().text();
        return Questao();
    }

    if (q.next()) {
        Questao qs;
        qs.id = q.value("id").toInt();
        qs.enunciado = q.value("enunciado").toString();
        qs.resposta = q.value("resposta").toString();
        qs.tags = q.value("tags").toString().split(",", Qt::SkipEmptyParts);
        qs.criadaEm = QDateTime::fromString(q.value("criada_em").toString(), Qt::ISODate);
        qs.enunciadoImagens = ListarImagens(qs.id, "enunciado");
        qs.respostaImagens = ListarImagens(qs.id, "resposta");
        return qs;
    }

    return Questao();
}

QList<Questao> QuestaoRepoSQLite::Listar() {
    QList<Questao> list;
    QSqlQuery q("SELECT * FROM questoes ORDER BY id DESC", db_);

    while (q.next()) {
        Questao qs;
        qs.id = q.value("id").toInt();
        qs.enunciado = q.value("enunciado").toString();
        qs.resposta = q.value("resposta").toString();
        qs.tags = q.value("tags").toString().split(",", Qt::SkipEmptyParts);
        qs.criadaEm = QDateTime::fromString(q.value("criada_em").toString(), Qt::ISODate);
        qs.enunciadoImagens = ListarImagens(qs.id, "enunciado");
        qs.respostaImagens = ListarImagens(qs.id, "resposta");
        list.append(qs);
    }
    return list;
}

QList<Questao> QuestaoRepoSQLite::ListarBasico() {
    QList<Questao> list;
    QSqlQuery q("SELECT id, enunciado, resposta, tags, criada_em FROM questoes ORDER BY id DESC",
                db_);

    while (q.next()) {
        Questao qs;
        qs.id = q.value(0).toInt();
        qs.enunciado = q.value(1).toString();
        qs.resposta = q.value(2).toString();
        qs.tags = q.value(3).toString().split(",", Qt::SkipEmptyParts);
        qs.criadaEm = QDateTime::fromString(q.value(4).toString(), Qt::ISODate);
        list.append(qs);
    }
    return list;
}

QList<Questao> QuestaoRepoSQLite::ListarBasicoCached() {
    if (s_cacheBasicoValid) {
        return s_cacheBasico;
    }
    s_cacheBasico = ListarBasico();
    s_cacheBasicoValid = true;
    return s_cacheBasico;
}

void QuestaoRepoSQLite::Excluir(int id) {
    ExcluirImagens(id);
    QSqlQuery q(db_);
    q.prepare("DELETE FROM questoes WHERE id = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qDebug() << "Error deleting question:" << q.lastError().text();
    }
}

void QuestaoRepoSQLite::Excluir(const Questao& qst) {
    Excluir(qst.id);
}

void QuestaoRepoSQLite::ExcluirTodas() {
    QSqlQuery qIds("SELECT id FROM questoes", db_);
    while (qIds.next()) {
        const int id = qIds.value(0).toInt();
        ExcluirImagens(id);
    }

    QSqlQuery q(db_);
    if (!q.exec("DELETE FROM questao_imagens")) {
        qDebug() << "Error deleting images:" << q.lastError().text();
    }
    if (!q.exec("DELETE FROM questoes")) {
        qDebug() << "Error deleting questions:" << q.lastError().text();
    }
}

void QuestaoRepoSQLite::InvalidarCacheBasico() {
    s_cacheBasicoValid = false;
    s_cacheBasico.clear();
}

int QuestaoRepoSQLite::Contar() {
    QSqlQuery q("SELECT COUNT(*) FROM questoes", db_);
    if (q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}

QStringList QuestaoRepoSQLite::ListarImagens(int questaoId, const QString& tipo) {
    QStringList paths;
    QSqlQuery q(db_);
    q.prepare(
        "SELECT path FROM questao_imagens WHERE questao_id = :id AND tipo = :tipo ORDER BY ordem "
        "ASC, id ASC");
    q.bindValue(":id", questaoId);
    q.bindValue(":tipo", tipo);
    if (!q.exec()) {
        qDebug() << "Error listing images:" << q.lastError().text();
        return paths;
    }
    while (q.next()) {
        paths.append(q.value(0).toString());
    }
    return paths;
}

void QuestaoRepoSQLite::SalvarImagens(int questaoId, const QStringList& paths,
                                      const QString& tipo) {
    if (questaoId <= 0 || paths.isEmpty()) {
        return;
    }
    QSqlQuery q(db_);
    q.prepare(
        "INSERT INTO questao_imagens (questao_id, tipo, path, ordem, criada_em) "
        "VALUES (:id, :tipo, :path, :ordem, :criada_em)");
    int ordem = 0;
    for (const auto& path : paths) {
        q.bindValue(":id", questaoId);
        q.bindValue(":tipo", tipo);
        q.bindValue(":path", path);
        q.bindValue(":ordem", ordem++);
        q.bindValue(":criada_em", QDateTime::currentDateTime().toString(Qt::ISODate));
        if (!q.exec()) {
            qDebug() << "Error saving image:" << q.lastError().text();
        }
    }
}

void QuestaoRepoSQLite::SincronizarImagens(int questaoId, const QStringList& paths,
                                           const QString& tipo) {
    const QStringList atuais = ListarImagens(questaoId, tipo);
    for (const auto& path : atuais) {
        if (!paths.contains(path)) {
            QFile::remove(path);
        }
    }

    QSqlQuery del(db_);
    del.prepare("DELETE FROM questao_imagens WHERE questao_id = :id AND tipo = :tipo");
    del.bindValue(":id", questaoId);
    del.bindValue(":tipo", tipo);
    if (!del.exec()) {
        qDebug() << "Error clearing images:" << del.lastError().text();
        return;
    }

    SalvarImagens(questaoId, paths, tipo);
}

void QuestaoRepoSQLite::ExcluirImagens(int questaoId) {
    const QStringList enunciado = ListarImagens(questaoId, "enunciado");
    const QStringList resposta = ListarImagens(questaoId, "resposta");
    for (const auto& path : enunciado) {
        QFile::remove(path);
    }
    for (const auto& path : resposta) {
        QFile::remove(path);
    }

    QSqlQuery del(db_);
    del.prepare("DELETE FROM questao_imagens WHERE questao_id = :id");
    del.bindValue(":id", questaoId);
    if (!del.exec()) {
        qDebug() << "Error deleting images:" << del.lastError().text();
    }
}

}  // namespace repo
}  // namespace kapraxis
