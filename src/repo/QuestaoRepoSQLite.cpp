// QuestaoRepoSQLite.cpp (versão corrigida completa)
#include "QuestaoRepoSQLite.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QuestaoRepoSQLite::QuestaoRepoSQLite() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("questoes.db");
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados:" << db.lastError().text();
        return;
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
    }
}

void QuestaoRepoSQLite::salvar(const Questao& qst) {
    QSqlQuery q(db);  // CORREÇÃO: passe db
    q.prepare("INSERT INTO questoes (enunciado, resposta, tags, criada_em) VALUES (?, ?, ?, ?)");
    q.addBindValue(qst.enunciado);
    q.addBindValue(qst.resposta);
    q.addBindValue(qst.tags.join(","));
    q.addBindValue(qst.criadaEm.toString(Qt::ISODate));
    q.exec();
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
        list.append(qs);
    }
    return list;
}

void QuestaoRepoSQLite::excluir(int id) {
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