#pragma once

#include <QDate>
#include <QMap>
#include <QString>

class StudyStatsStore {
public:
    static QMap<QString, int> loadDailySeconds();
    static QMap<QString, QMap<QString, int>> loadDailySecondsByDiscipline();
    static void addSeconds(const QDate& date, const QString& disciplina, int seconds);

private:
    static QString filePath();
    static void saveDailySeconds(const QMap<QString, QMap<QString, int>>& map);
};
