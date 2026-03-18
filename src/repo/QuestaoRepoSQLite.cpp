// QuestaoRepoSQLite.cpp (versão corrigida completa)
#include "QuestaoRepoSQLite.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

namespace {
QList<Questao> s_cacheBasico;
bool s_cacheBasicoValid = false;
}

QuestaoRepoSQLite::QuestaoRepoSQLite() {
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataDir.isEmpty()) {
        QDir().mkpath(appDataDir);
    }

    const QString appDataDbPath = appDataDir.isEmpty()
        ? QString("questoes.db")
        : QDir(appDataDir).filePath("questoes.db");

    // Migrar DB antigo no diretório de execução, se existir
    if (appDataDbPath != "questoes.db" && !QFile::exists(appDataDbPath) && QFile::exists("questoes.db")) {
        QFile::copy("questoes.db", appDataDbPath);
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(appDataDbPath);
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados:" << db.lastError().text();
        return;
    }

    {
        QSqlQuery pragma(db);
        pragma.exec("PRAGMA foreign_keys = ON");
    }

    init();
}

void QuestaoRepoSQLite::init() {
    QSqlQuery q(db);  // CORREÇÃO: passe db
    q.exec(
        "CREATE TABLE IF NOT EXISTS questoes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "enunciado TEXT,"
        "resposta TEXT,"
        "tags TEXT,"
        "criada_em TEXT)"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS questao_imagens ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "questao_id INTEGER NOT NULL,"
        "tipo TEXT NOT NULL,"
        "path TEXT NOT NULL,"
        "ordem INTEGER,"
        "criada_em TEXT,"
        "FOREIGN KEY(questao_id) REFERENCES questoes(id) ON DELETE CASCADE)"
    );
}

void QuestaoRepoSQLite::atualizar(const Questao& qst) {
    if (qst.id <= 0) {
        qDebug() << "ID inválido para atualização";
        return;
    }
    
    QSqlQuery q(db);
    q.prepare(
        "UPDATE questoes SET "
        "enunciado = :enunciado, "
        "resposta = :resposta, "
        "tags = :tags, "
        "criada_em = :criada_em "
        "WHERE id = :id"
    );
    
    q.bindValue(":id", qst.id);
    q.bindValue(":enunciado", qst.enunciado);
    q.bindValue(":resposta", qst.resposta);
    q.bindValue(":tags", qst.tags.join(","));
    q.bindValue(":criada_em", qst.criadaEm.toString(Qt::ISODate));
    
    if (!q.exec()) {
        qDebug() << "Erro ao atualizar questão:" << q.lastError().text();
        return;
    }

    sincronizarImagens(qst.id, qst.enunciadoImagens, "enunciado");
    sincronizarImagens(qst.id, qst.respostaImagens, "resposta");
}

void QuestaoRepoSQLite::salvar(const Questao& qst) {
    QSqlQuery q(db);  // CORREÇÃO: passe db
    q.prepare("INSERT INTO questoes (enunciado, resposta, tags, criada_em) VALUES (?, ?, ?, ?)");
    q.addBindValue(qst.enunciado);
    q.addBindValue(qst.resposta);
    q.addBindValue(qst.tags.join(","));
    q.addBindValue(qst.criadaEm.toString(Qt::ISODate));
    if (!q.exec()) {
        qDebug() << "Erro ao salvar questão:" << q.lastError().text();
        return;
    }

    const QVariant newIdVar = q.lastInsertId();
    if (newIdVar.isValid()) {
        const int newId = newIdVar.toInt();
        salvarImagens(newId, qst.enunciadoImagens, "enunciado");
        salvarImagens(newId, qst.respostaImagens, "resposta");
    }
}

Questao QuestaoRepoSQLite::buscarPorId(int id) {
    QSqlQuery q(db);
    q.prepare("SELECT * FROM questoes WHERE id = :id");
    q.bindValue(":id", id);
    
    if (!q.exec()) {
        qDebug() << "Erro ao buscar questão:" << q.lastError().text();
        return Questao();  // Retorna Questao vazia
    }
    
    if (q.next()) {
        Questao qs;
        qs.id = q.value("id").toInt();
        qs.enunciado = q.value("enunciado").toString();
        qs.resposta = q.value("resposta").toString();
        qs.tags = q.value("tags").toString().split(",", Qt::SkipEmptyParts);
        qs.criadaEm = QDateTime::fromString(q.value("criada_em").toString(), Qt::ISODate);
        qs.enunciadoImagens = listarImagens(qs.id, "enunciado");
        qs.respostaImagens = listarImagens(qs.id, "resposta");
        return qs;
    }
    
    return Questao();  // Retorna Questao vazia se não encontrou
}

QList<Questao> QuestaoRepoSQLite::listar() {
    QList<Questao> list;
    QSqlQuery q("SELECT * FROM questoes ORDER BY id DESC", db);  // CORREÇÃO: passe db

    while (q.next()) {
        Questao qs;
        qs.id = q.value("id").toInt();
        qs.enunciado = q.value("enunciado").toString();
        qs.resposta = q.value("resposta").toString();
        qs.tags = q.value("tags").toString().split(",", Qt::SkipEmptyParts);
        qs.criadaEm = QDateTime::fromString(q.value("criada_em").toString(), Qt::ISODate);
        qs.enunciadoImagens = listarImagens(qs.id, "enunciado");
        qs.respostaImagens = listarImagens(qs.id, "resposta");
        list.append(qs);
    }
    return list;
}

QList<Questao> QuestaoRepoSQLite::listarBasico() {
    QList<Questao> list;
    QSqlQuery q("SELECT id, enunciado, resposta, tags, criada_em FROM questoes ORDER BY id DESC", db);

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

QList<Questao> QuestaoRepoSQLite::listarBasicoCached() {
    if (s_cacheBasicoValid) {
        return s_cacheBasico;
    }
    s_cacheBasico = listarBasico();
    s_cacheBasicoValid = true;
    return s_cacheBasico;
}

void QuestaoRepoSQLite::excluir(int id) {
    excluirImagens(id);
    QSqlQuery q(db);
    q.prepare("DELETE FROM questoes WHERE id = :id");
    q.bindValue(":id", id);
    
    if (!q.exec()) {
        qDebug() << "Erro ao excluir questão:" << q.lastError().text();
    }
}

void QuestaoRepoSQLite::excluir(const Questao& qst) {
    excluir(qst.id);
}

void QuestaoRepoSQLite::excluirTodas() {
    QSqlQuery qIds("SELECT id FROM questoes", db);
    while (qIds.next()) {
        const int id = qIds.value(0).toInt();
        excluirImagens(id);
    }

    QSqlQuery q(db);
    if (!q.exec("DELETE FROM questao_imagens")) {
        qDebug() << "Erro ao excluir imagens:" << q.lastError().text();
    }
    if (!q.exec("DELETE FROM questoes")) {
        qDebug() << "Erro ao excluir questoes:" << q.lastError().text();
    }
}

void QuestaoRepoSQLite::invalidarCacheBasico() {
    s_cacheBasicoValid = false;
    s_cacheBasico.clear();
}

int QuestaoRepoSQLite::contar() {
    QSqlQuery q("SELECT COUNT(*) FROM questoes", db);
    if (q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}

QStringList QuestaoRepoSQLite::listarImagens(int questaoId, const QString& tipo) {
    QStringList paths;
    QSqlQuery q(db);
    q.prepare("SELECT path FROM questao_imagens WHERE questao_id = :id AND tipo = :tipo ORDER BY ordem ASC, id ASC");
    q.bindValue(":id", questaoId);
    q.bindValue(":tipo", tipo);
    if (!q.exec()) {
        qDebug() << "Erro ao listar imagens:" << q.lastError().text();
        return paths;
    }
    while (q.next()) {
        paths.append(q.value(0).toString());
    }
    return paths;
}

void QuestaoRepoSQLite::salvarImagens(int questaoId, const QStringList& paths, const QString& tipo) {
    if (questaoId <= 0 || paths.isEmpty()) {
        return;
    }
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO questao_imagens (questao_id, tipo, path, ordem, criada_em) "
        "VALUES (:id, :tipo, :path, :ordem, :criada_em)"
    );
    int ordem = 0;
    for (const auto& path : paths) {
        q.bindValue(":id", questaoId);
        q.bindValue(":tipo", tipo);
        q.bindValue(":path", path);
        q.bindValue(":ordem", ordem++);
        q.bindValue(":criada_em", QDateTime::currentDateTime().toString(Qt::ISODate));
        if (!q.exec()) {
            qDebug() << "Erro ao salvar imagem:" << q.lastError().text();
        }
    }
}

void QuestaoRepoSQLite::sincronizarImagens(int questaoId, const QStringList& paths, const QString& tipo) {
    const QStringList atuais = listarImagens(questaoId, tipo);
    for (const auto& path : atuais) {
        if (!paths.contains(path)) {
            QFile::remove(path);
        }
    }

    QSqlQuery del(db);
    del.prepare("DELETE FROM questao_imagens WHERE questao_id = :id AND tipo = :tipo");
    del.bindValue(":id", questaoId);
    del.bindValue(":tipo", tipo);
    if (!del.exec()) {
        qDebug() << "Erro ao limpar imagens:" << del.lastError().text();
        return;
    }

    salvarImagens(questaoId, paths, tipo);
}

void QuestaoRepoSQLite::excluirImagens(int questaoId) {
    const QStringList enunciado = listarImagens(questaoId, "enunciado");
    const QStringList resposta = listarImagens(questaoId, "resposta");
    for (const auto& path : enunciado) {
        QFile::remove(path);
    }
    for (const auto& path : resposta) {
        QFile::remove(path);
    }

    QSqlQuery del(db);
    del.prepare("DELETE FROM questao_imagens WHERE questao_id = :id");
    del.bindValue(":id", questaoId);
    if (!del.exec()) {
        qDebug() << "Erro ao excluir imagens:" << del.lastError().text();
    }
}
