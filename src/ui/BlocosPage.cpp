// BlocosPage.cpp
#include "BlocosPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QDateTime>

BlocosPage::BlocosPage(QWidget* parent)
    : QWidget(parent)
    , timer(new QTimer(this))
    , elapsedTimer(new QElapsedTimer())
    , tempoDecorrido(0)
    , blocoAtivo(false)
    , emPausa(false)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    
    // Timer
    auto* timerGroup = new QGroupBox("Temporizador");
    auto* timerLayout = new QVBoxLayout(timerGroup);
    
    lblTimer = new QLabel("00:00:00");
    lblTimer->setStyleSheet("font-size: 42px; display: flex; font-weight: bold; text-align: center;");
    
    lblStatus = new QLabel("Pronto para começar");
    lblStatus->setStyleSheet("text-align: center; color: #666;");
    
    auto* buttonLayout = new QHBoxLayout();
    btnIniciar = new QPushButton("▶  Iniciar");
    btnPausar = new QPushButton("  Pausar");
    btnFinalizar = new QPushButton("⏹  Finalizar");
    
    btnPausar->setVisible(false);
    btnFinalizar->setVisible(false);
    btnIniciar->setVisible(true);
    
    buttonLayout->addWidget(btnIniciar);
    buttonLayout->addWidget(btnPausar);
    buttonLayout->addWidget(btnFinalizar);
    
    timerLayout->addWidget(lblTimer);
    timerLayout->addWidget(lblStatus);
    timerLayout->addLayout(buttonLayout);
    
    auto* detailsGroup = new QGroupBox("Detalhes");
    auto* detailsLayout = new QFormLayout(detailsGroup);
    
    comboDisciplina = new QComboBox();
    comboDisciplina->addItems({"Matemática", "Física", "Química", "Português", "Inglês", "Outra"});
    
    txtTopico = new QLineEdit();
    txtTopico->setPlaceholderText("Tópico estudado...");
    
    detailsLayout->addRow("Disciplina:", comboDisciplina);
    detailsLayout->addRow("Comentário:", txtTopico);
    
    auto* historyGroup = new QGroupBox("Histórico");
    listaBlocos = new QListWidget();
    
    auto* historyLayout = new QVBoxLayout(historyGroup);
    historyLayout->addWidget(listaBlocos);
    
    mainLayout->addWidget(timerGroup);
    mainLayout->addWidget(detailsGroup);
    mainLayout->addWidget(historyGroup);
    
    connect(btnIniciar, &QPushButton::clicked, this, &BlocosPage::iniciarBloco);
    connect(btnPausar, &QPushButton::clicked, this, &BlocosPage::pausarContinuarBloco);
    connect(btnFinalizar, &QPushButton::clicked, this, &BlocosPage::finalizarBloco);
    connect(timer, &QTimer::timeout, this, &BlocosPage::atualizarTimer);
    timer->setInterval(1000);
}

void BlocosPage::iniciarBloco()
{
    if (blocoAtivo) return;
    
    QString disciplina = comboDisciplina->currentText().trimmed();
    if (disciplina.isEmpty()) return;
    
    blocoAtivo = true;
    emPausa = false;
    tempoDecorrido = 0;
    
    blocoAtual = BlocoEstudo();
    blocoAtual.disciplina = disciplina;
    blocoAtual.topico = txtTopico->text().trimmed();
    blocoAtual.inicio = QDateTime::currentDateTime();
    
    lblStatus->setText("Estudando " + disciplina + "...");
    btnPausar->setVisible(true);
    btnFinalizar->setVisible(true);
    btnIniciar->setVisible(false);
    
    elapsedTimer->restart();
    timer->start();
}

void BlocosPage::pausarContinuarBloco()
{
    if (!blocoAtivo) return;
    
    if (!emPausa) {
        emPausa = true;
        timer->stop();
        lblStatus->setText("  Pausado");
        btnPausar->setText("▶  Continuar");
    } else {
        emPausa = false;
        lblStatus->setText("Estudando " + blocoAtual.disciplina + "...");
        btnPausar->setText("  Pausar");
        elapsedTimer->restart();
        timer->start();
    }
}

void BlocosPage::finalizarBloco()
{
    if (!blocoAtivo) return;
    
    timer->stop();
    
    blocoAtual.fim = QDateTime::currentDateTime();
    blocoAtual.duracaoSegundos = tempoDecorrido;
    
    historico.append(blocoAtual);
    
    QString itemText = QString("%1 // %2 (%3)")
        .arg(blocoAtual.inicio.toString("HH:mm"))
        .arg(blocoAtual.disciplina)
        .arg(formatarTempo(tempoDecorrido));
    
    if (!blocoAtual.topico.isEmpty()) {
        itemText += "\n  " + blocoAtual.topico;
    }
    
    listaBlocos->insertItem(0, itemText);
    
    // Reset
    blocoAtivo = false;
    emPausa = false;
    tempoDecorrido = 0;
    
    lblTimer->setText("00:00:00");
    lblStatus->setText("Pronto para começar");
    btnIniciar->setVisible(true);
    btnPausar->setVisible(false);
    btnPausar->setText("⏸ Pausar");
    btnFinalizar->setVisible(false);
}

void BlocosPage::atualizarTimer()
{
    tempoDecorrido = elapsedTimer->elapsed() / 1000;
    lblTimer->setText(formatarTempo(tempoDecorrido));
}

QString BlocosPage::formatarTempo(int segundos)
{
    int horas = segundos / 3600;
    int minutos = (segundos % 3600) / 60;
    int segs = segundos % 60;
    return QString("%1:%2:%3")
        .arg(horas, 2, 10, QLatin1Char('0'))
        .arg(minutos, 2, 10, QLatin1Char('0'))
        .arg(segs, 2, 10, QLatin1Char('0'));
}

int BlocosPage::calcularTempoEfetivo(const BlocoEstudo& bloco)
{
    return bloco.duracaoSegundos;
}

void BlocosPage::carregarHistorico()
{
    
}