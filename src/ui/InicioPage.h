#ifndef KAPRAXIS_UI_INICIOPAGE_H_
#define KAPRAXIS_UI_INICIOPAGE_H_

#include <QLabel>
#include <QWidget>

class QFrame;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QShowEvent;
namespace kapraxis {
namespace repo {
class QuestaoRepoSQLite;
}
}  // namespace kapraxis

class InicioPage : public QWidget {
    Q_OBJECT
   public:
    explicit InicioPage(QWidget* parent = nullptr);

   protected:
    void showEvent(QShowEvent* event) override;

   public slots:
    void atualizarResumo();

   private:
    kapraxis::repo::QuestaoRepoSQLite* repo;
    QLabel* lblTotalQuestoes;
    QLabel* lblHorasEstudo;
    QWidget* chartWidget;
};

#endif  // KAPRAXIS_UI_INICIOPAGE_H_
