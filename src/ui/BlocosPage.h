#ifndef BLOCOSPAGE_H
#define BLOCOSPAGE_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

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

class BlocosPage : public QWidget
{
    Q_OBJECT

public:
    explicit BlocosPage(QWidget* parent = nullptr);
    
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
};

#endif // BLOCOSPAGE_H