#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>

struct Questao {
    int id;
    QString enunciado;
    QString resposta;
    QStringList tags;
    QDateTime criadaEm;
};
