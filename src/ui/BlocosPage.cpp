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

#include "../repo/StudyStatsStore.h"

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
    auto* timerGroup = new QGroupBox(tr("Timer"));
    auto* timerLayout = new QVBoxLayout(timerGroup);
    
    lblTimer = new QLabel("00:00:00");
    lblTimer->setStyleSheet("font-size: 42px; display: flex; font-weight: bold; text-align: center;");
    lblTimer->setAccessibleName(tr("Main timer"));

    lblStatus = new QLabel(tr("Ready to begin"));
    lblStatus->setStyleSheet("text-align: center; color: #666;");
    lblStatus->setAccessibleName(tr("Timer status"));
    
    auto* buttonLayout = new QHBoxLayout();
    btnIniciar = new QPushButton(tr("▶  Start"));
    btnPausar = new QPushButton(tr("  Pause"));
    btnFinalizar = new QPushButton(tr("⏹  Finish"));
    btnIniciar->setAccessibleName(tr("Start timer"));
    btnPausar->setAccessibleName(tr("Pause or resume block"));
    btnFinalizar->setAccessibleName(tr("Finish block"));
    
    btnPausar->setVisible(false);
    btnFinalizar->setVisible(false);
    btnIniciar->setVisible(true);
    
    buttonLayout->addWidget(btnIniciar);
    buttonLayout->addWidget(btnPausar);
    buttonLayout->addWidget(btnFinalizar);
    
    timerLayout->addWidget(lblTimer);
    timerLayout->addWidget(lblStatus);
    timerLayout->addLayout(buttonLayout);
    
    auto* detailsGroup = new QGroupBox(tr("Details"));
    auto* detailsLayout = new QFormLayout(detailsGroup);
    
    comboDisciplina = new QComboBox();
    comboDisciplina->addItems({
        tr("Mathematics"), tr("Physics"), tr("Chemistry"),
        tr("Portuguese"), tr("English"), tr("Other")
    });
    comboDisciplina->setAccessibleName(tr("Select discipline"));
    
    txtTopico = new QLineEdit();
    txtTopico->setPlaceholderText(tr("Study topic..."));
    txtTopico->setAccessibleName(tr("Block comment field"));
    
    detailsLayout->addRow(tr("Discipline:"), comboDisciplina);
    detailsLayout->addRow(tr("Comment:"), txtTopico);
    
    auto* historyGroup = new QGroupBox(tr("History"));
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
    
    lblStatus->setText(tr("Studying %1...").arg(disciplina));
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
        tempoDecorrido += elapsedTimer->elapsed() / 1000;
        lblStatus->setText(tr("  Paused"));
        btnPausar->setText(tr("▶  Resume"));
    } else {
        emPausa = false;
        lblStatus->setText(tr("Studying %1...").arg(blocoAtual.disciplina));
        btnPausar->setText(tr("  Pause"));
        elapsedTimer->restart();
        timer->start();
    }
}

void BlocosPage::finalizarBloco()
{
    if (!blocoAtivo) return;
    
    timer->stop();

    if (!emPausa) {
        tempoDecorrido += elapsedTimer->elapsed() / 1000;
    }
    
    blocoAtual.fim = QDateTime::currentDateTime();
    blocoAtual.duracaoSegundos = tempoDecorrido;

    if (blocoAtual.duracaoSegundos > 0) {
        registrarTempoEstudo(blocoAtual.fim, blocoAtual.duracaoSegundos);
    }
    
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
    lblStatus->setText(tr("Ready to begin"));
    btnIniciar->setVisible(true);
    btnPausar->setVisible(false);
    btnPausar->setText(tr("⏸ Pause"));
    btnFinalizar->setVisible(false);
}

void BlocosPage::atualizarTimer()
{
    int total = tempoDecorrido + (elapsedTimer->elapsed() / 1000);
    lblTimer->setText(formatarTempo(total));
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

void BlocosPage::registrarTempoEstudo(const QDateTime& fim, int segundos)
{
    if (segundos <= 0) return;
    StudyStatsStore::addSeconds(fim.date(), blocoAtual.disciplina, segundos);
    emit studyStatsUpdated();
}

void BlocosPage::carregarHistorico()
{
    
}
