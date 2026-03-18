// InicioPage.cpp
#include "InicioPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGroupBox>
#include <QDate>
#include <QPainter>
#include <QStyleOption>

#include "../repo/QuestaoRepoSQLite.h"
#include "../repo/StudyStatsStore.h"

namespace {
class StudyChartWidget : public QWidget {
public:
    explicit StudyChartWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(160);
    }

    void setData(const QVector<int>& seconds, const QStringList& labels) {
        dataSeconds = seconds;
        dataLabels = labels;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QStyleOption opt;
        opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

        const QRect r = rect().adjusted(16, 12, -16, -28);

        if (dataSeconds.isEmpty()) {
            p.setPen(QColor(180, 180, 180));
            p.drawText(r, Qt::AlignCenter, "Sem dados de estudo");
            return;
        }

        int maxValue = 1;
        for (int v : dataSeconds) {
            if (v > maxValue) maxValue = v;
        }

        const int count = dataSeconds.size();
        const int gap = 10;
        const int barWidth = (r.width() - (gap * (count - 1))) / count;

        p.setPen(Qt::NoPen);

        for (int i = 0; i < count; ++i) {
            const int v = dataSeconds[i];
            const double ratio = static_cast<double>(v) / maxValue;
            const int h = static_cast<int>(ratio * r.height());
            const int x = r.left() + i * (barWidth + gap);
            const int y = r.bottom() - h;

            QRect barRect(x, y, barWidth, h);

            QLinearGradient grad(barRect.topLeft(), barRect.bottomLeft());
            grad.setColorAt(0.0, QColor(80, 170, 255));
            grad.setColorAt(1.0, QColor(40, 110, 190));
            p.setBrush(grad);
            p.drawRoundedRect(barRect, 6, 6);

            p.setPen(QColor(170, 170, 170));
            QRect labelRect(x, r.bottom() + 6, barWidth, 18);
            p.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, dataLabels.value(i));
            p.setPen(Qt::NoPen);
        }
    }

private:
    QVector<int> dataSeconds;
    QStringList dataLabels;
};

static QString formatDuration(int seconds) {
    if (seconds < 0) seconds = 0;
    const int horas = seconds / 3600;
    const int minutos = (seconds % 3600) / 60;
    const int segs = seconds % 60;
    if (horas > 0) {
        return QString("%1h %2m").arg(horas).arg(minutos, 2, 10, QLatin1Char('0'));
    }
    if (minutos > 0) {
        return QString("%1m").arg(minutos);
    }
    return QString("%1s").arg(segs);
}

} // namespace

InicioPage::InicioPage(QWidget* parent)
    : QWidget(parent)
{
    repo = new QuestaoRepoSQLite;

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 24);
    rootLayout->setSpacing(16);

    auto* title = new QLabel("Kapraxis");
    title->setObjectName("pageTitle");

    auto* subtitle = new QLabel("Visao geral dos estudos");
    subtitle->setObjectName("pageSubtitle");

    rootLayout->addWidget(title);
    rootLayout->addWidget(subtitle);

    auto* metricsRow = new QHBoxLayout();
    metricsRow->setSpacing(12);

    auto* cardQuestoes = new QFrame();
    cardQuestoes->setObjectName("metricCard");
    auto* cardQuestoesLayout = new QVBoxLayout(cardQuestoes);
    cardQuestoesLayout->setSpacing(6);

    lblTotalQuestoes = new QLabel("0");
    lblTotalQuestoes->setObjectName("metricValue");
    auto* lblTotalCaption = new QLabel("Total de questoes");
    lblTotalCaption->setObjectName("metricLabel");

    cardQuestoesLayout->addWidget(lblTotalCaption);
    cardQuestoesLayout->addWidget(lblTotalQuestoes);

    auto* cardHoras = new QFrame();
    cardHoras->setObjectName("metricCard");
    auto* cardHorasLayout = new QVBoxLayout(cardHoras);
    cardHorasLayout->setSpacing(6);

    lblHorasEstudo = new QLabel("0m");
    lblHorasEstudo->setObjectName("metricValue");
    auto* lblHorasCaption = new QLabel("Tempo total de estudo");
    lblHorasCaption->setObjectName("metricLabel");

    cardHorasLayout->addWidget(lblHorasCaption);
    cardHorasLayout->addWidget(lblHorasEstudo);

    metricsRow->addWidget(cardQuestoes, 1);
    metricsRow->addWidget(cardHoras, 1);

    rootLayout->addLayout(metricsRow);

    auto* chartFrame = new QFrame();
    chartFrame->setObjectName("chartCard");
    auto* chartLayout = new QVBoxLayout(chartFrame);
    chartLayout->setContentsMargins(16, 14, 16, 16);
    chartLayout->setSpacing(10);

    //auto* chartTitle = new QLabel("");
    //chartTitle->setObjectName("sectionTitle");

    chartWidget = new StudyChartWidget();
    chartWidget->setObjectName("studyChart");
    chartWidget->setAttribute(Qt::WA_StyledBackground, true);

    //chartLayout->addWidget(chartTitle);
    chartLayout->addWidget(chartWidget);

    rootLayout->addWidget(chartFrame, 1);

    rootLayout->addStretch();
}

void InicioPage::showEvent(QShowEvent* event)
{
    atualizarResumo();
    QWidget::showEvent(event);
}

void InicioPage::atualizarResumo()
{
    const int totalQuestoes = repo->contar();
    lblTotalQuestoes->setText(QString::number(totalQuestoes));

    const QMap<QString, QMap<QString, int>> mapa = StudyStatsStore::loadDailySecondsByDiscipline();

    QVector<int> seconds;
    QStringList labels;
    int total7Dias = 0;
    int totalGeral = 0;

    const QDate hoje = QDate::currentDate();
    for (int i = 6; i >= 0; --i) {
        const QDate d = hoje.addDays(-i);
        const QString key = d.toString("yyyy-MM-dd");
        const QMap<QString, int> perDisc = mapa.value(key);
        int diaTotal = 0;
        for (auto it = perDisc.begin(); it != perDisc.end(); ++it) {
            diaTotal += it.value();
        }
        seconds.append(diaTotal);
        labels.append(d.toString("dd/MM"));
        total7Dias += diaTotal;
    }

    for (auto it = mapa.begin(); it != mapa.end(); ++it) {
        const QMap<QString, int>& perDisc = it.value();
        for (auto dit = perDisc.begin(); dit != perDisc.end(); ++dit) {
            totalGeral += dit.value();
        }
    }

    lblHorasEstudo->setText(formatDuration(totalGeral));
    static_cast<StudyChartWidget*>(chartWidget)->setData(seconds, labels);
}
