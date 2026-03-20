#ifndef KAPRAXIS_UI_BLOCOSPAGE_H_
#define KAPRAXIS_UI_BLOCOSPAGE_H_

#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QPair>
#include <QString>
#include <QTimer>
#include <QWidget>

// Forward declarations
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class QListWidget;
class QGroupBox;

struct BlocoEstudo {
    int id;
    QString disciplina;
    QString topico;
    QDateTime inicio;
    QDateTime fim;
    int duracaoSegundos;
    bool pausado;
    QList<QPair<QDateTime, QDateTime>> pausas;

    BlocoEstudo() : id(0), duracaoSegundos(0), pausado(false) {}
};

class BlocosPage : public QWidget {
    Q_OBJECT

   public:
    explicit BlocosPage(QWidget* parent = nullptr);

   signals:
    void studyStatsUpdated();

   private slots:
    void iniciarBloco();
    void pausarContinuarBloco();
    void finalizarBloco();
    void atualizarTimer();
    void carregarHistorico();

   private:
    QTimer* timer;
    QElapsedTimer* elapsedTimer;
    int tempoDecorrido;
    bool blocoAtivo;
    bool emPausa;

    // UI Components
    QLabel* lblTimer;
    QLabel* lblStatus;
    QPushButton* btnIniciar;
    QPushButton* btnPausar;
    QPushButton* btnFinalizar;

    QComboBox* comboDisciplina;
    QLineEdit* txtTopico;
    QListWidget* listaBlocos;

    QList<BlocoEstudo> historico;
    BlocoEstudo blocoAtual;

    QString formatarTempo(int segundos);
    int calcularTempoEfetivo(const BlocoEstudo& bloco);
    void registrarTempoEstudo(const QDateTime& fim, int segundos);
};

#endif  // KAPRAXIS_UI_BLOCOSPAGE_H_
