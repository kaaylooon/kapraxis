#ifndef KAPRAXIS_DOMAIN_QUESTAO_H_
#define KAPRAXIS_DOMAIN_QUESTAO_H_

#include <QDateTime>
#include <QString>
#include <QStringList>

struct Questao {
    Questao() : id(0) {}
    int id;
    QString enunciado;
    QString resposta;
    QStringList tags;
    QStringList enunciadoImagens;
    QStringList respostaImagens;
    QDateTime criadaEm;
};

#endif  // KAPRAXIS_DOMAIN_QUESTAO_H_
