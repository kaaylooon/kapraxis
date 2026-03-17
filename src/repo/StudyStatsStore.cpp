#include "StudyStatsStore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

QString StudyStatsStore::filePath()
{
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("study_stats.json");
}

QMap<QString, int> StudyStatsStore::loadDailySeconds()
{
    QMap<QString, int> totals;
    const QMap<QString, QMap<QString, int>> byDisc = loadDailySecondsByDiscipline();
    for (auto it = byDisc.begin(); it != byDisc.end(); ++it) {
        int sum = 0;
        const QMap<QString, int>& perDisc = it.value();
        for (auto dit = perDisc.begin(); dit != perDisc.end(); ++dit) {
            sum += dit.value();
        }
        totals.insert(it.key(), sum);
    }
    return totals;
}

QMap<QString, QMap<QString, int>> StudyStatsStore::loadDailySecondsByDiscipline()
{
    QMap<QString, QMap<QString, int>> map;
    QFile file(filePath());
    if (!file.exists()) {
        return map;
    }
    if (!file.open(QFile::ReadOnly)) {
        return map;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return map;
    }
    const QJsonObject root = doc.object();

    if (root.contains("daily") && root.value("daily").isObject()) {
        const QJsonObject daily = root.value("daily").toObject();
        for (auto it = daily.begin(); it != daily.end(); ++it) {
            QMap<QString, int> perDisc;
            if (it.value().isObject()) {
                const QJsonObject discObj = it.value().toObject();
                for (auto dit = discObj.begin(); dit != discObj.end(); ++dit) {
                    perDisc.insert(dit.key(), dit.value().toInt());
                }
            }
            map.insert(it.key(), perDisc);
        }
        return map;
    }

    // Legacy format: { "yyyy-MM-dd": seconds }
    for (auto it = root.begin(); it != root.end(); ++it) {
        QMap<QString, int> perDisc;
        perDisc.insert("Outra", it.value().toInt());
        map.insert(it.key(), perDisc);
    }
    return map;
}

void StudyStatsStore::saveDailySeconds(const QMap<QString, QMap<QString, int>>& map)
{
    QJsonObject daily;
    for (auto it = map.begin(); it != map.end(); ++it) {
        QJsonObject perDisc;
        const QMap<QString, int>& discMap = it.value();
        for (auto dit = discMap.begin(); dit != discMap.end(); ++dit) {
            perDisc.insert(dit.key(), dit.value());
        }
        daily.insert(it.key(), perDisc);
    }
    QJsonObject root;
    root.insert("daily", daily);
    const QJsonDocument doc(root);

    QFile file(filePath());
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
}

void StudyStatsStore::addSeconds(const QDate& date, const QString& disciplina, int seconds)
{
    if (seconds <= 0) return;
    const QString key = date.toString("yyyy-MM-dd");
    QMap<QString, QMap<QString, int>> map = loadDailySecondsByDiscipline();
    QMap<QString, int> perDisc = map.value(key);
    const QString disc = disciplina.trimmed().isEmpty() ? QString("Outra") : disciplina.trimmed();
    perDisc.insert(disc, perDisc.value(disc, 0) + seconds);
    map.insert(key, perDisc);
    saveDailySeconds(map);
}
