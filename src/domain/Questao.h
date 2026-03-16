#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>

struct Questao {
    Questao() : id(0) {}
    int id;
    QString enunciado;
    QString resposta;
    QStringList tags;
    QDateTime criadaEm;
};
