#ifndef KAPRAXIS_UI_QUESTOESPAGE_H_
#define KAPRAXIS_UI_QUESTOESPAGE_H_

#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QShortcut>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>
#include <QHBoxLayout>

#include "../domain/Questao.h"

namespace kapraxis {
namespace repo {
class QuestaoRepoSQLite;
}
}  // namespace kapraxis

class QListWidget;
class QLabel;
class QTextEdit;
class ClipboardTextEdit;
class QPushButton;
class QFrame;
class QGroupBox;
class QStackedWidget;
class QScrollArea;
class QVBoxLayout;
class QScrollBar;

class QuestoesPage : public QWidget {
    Q_OBJECT
   public:
    explicit QuestoesPage(QWidget* parent = nullptr);
    void excluirTodasQuestoes();
    void importarKeepJson();

   protected:
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private slots:
    void adicionarQuestao();
    void editarQuestao();
    void excluirQuestao();

    void mostrarDetalhes(QListWidgetItem* item);

    void salvarAlteracoes();
    void filtrarQuestoes(int index);
    void buscarQuestoes(const QString& texto);
    void focarBusca();

   private:
    void recarregar();
    void setupShortcuts();
    void setupDetailsPanel();
    void setupCampoAdicionalConnections();
    void setupImagePasteConnections();
    void destacarItem(QListWidgetItem* item);
    void adicionarItemLista(const Questao& q);
    void atualizarTamanhoItensLista();
    void iniciarPaginacao(const QList<Questao>& list, const QString& agrupamento);
    void carregarMais();
    void removerLoadingItem();
    bool abrirDialogQuestao(Questao& ioQuestao, const QString& titulo, const QString& header,
                            const QString& actionLabel);

    kapraxis::repo::QuestaoRepoSQLite* repo;

    // Componentes principais
    QListWidget* lista;
    QLineEdit* txtBusca;
    QComboBox* comboFilterTag;
    QComboBox* comboGroupBy;
    QStackedWidget* contentStack;
    QWidget* listPage;
    QWidget* detailPage;
    QPushButton* btnVoltarLista;

    // Componentes do painel de detalhes
    QGroupBox* detailsPanelGroup;
    QGroupBox* respostaGroup;
    QPushButton* btnCampoAdicional;
    QGroupBox* imagensEnunciadoGroup;
    QGroupBox* imagensRespostaGroup;
    QScrollArea* scrollImagensEnunciado;
    QScrollArea* scrollImagensResposta;
    QWidget* enunciadoImagesContainer;
    QWidget* respostaImagesContainer;
    QHBoxLayout* enunciadoImagesLayout;
    QHBoxLayout* respostaImagesLayout;
    QStringList detalheEnunciadoPaths;
    QStringList detalheRespostaPaths;

    // Componentes de texto
    ClipboardTextEdit* txtEnunciado;
    ClipboardTextEdit* txtResposta;
    QLabel* lblTags;
    QLabel* lblData;
    QLabel* lblInfo;

    // Atalhos de teclado
    QShortcut* atalhoNovaQuestao;
    QShortcut* atalhoSalvar;
    QShortcut* atalhoBusca;

    // Timer para auto-save
    QTimer* autoSaveTimer;

    // Paginação
    QList<Questao> renderQueue;
    int renderIndex = 0;
    QString renderGroupMode;
    QString renderCurrentGroup;
    QListWidgetItem* loadingItem = nullptr;
    int lastScrollValue = 0;
    bool forceNextLoad = false;

    static QString buildTagsHtml(const QStringList& tags);
    static QString groupKeyFor(const QDateTime& dt, const QString& mode);
};

#endif  // KAPRAXIS_UI_QUESTOESPAGE_H_
