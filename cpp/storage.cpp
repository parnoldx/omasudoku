#include "storage.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QVariantList>

StorageManager::StorageManager(const QString &baseDir)
{
    if (!baseDir.isEmpty()) {
        m_baseDir = baseDir;
    } else {
        const QByteArray xdg = qgetenv("XDG_DATA_HOME");
        if (!xdg.isEmpty())
            m_baseDir = QString::fromLocal8Bit(xdg) + QStringLiteral("/omarchy-sudoku");
        else
            m_baseDir = QDir::homePath() + QStringLiteral("/.local/share/omarchy-sudoku");
    }
    QDir().mkpath(m_baseDir);
    m_saveFile = m_baseDir + QStringLiteral("/savegame.json");
    m_highscoreFile = m_baseDir + QStringLiteral("/highscores.json");
}

bool StorageManager::hasSavedGame() const
{
    if (!QFile::exists(m_saveFile))
        return false;
    QFile f(m_saveFile);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return false;
    const QJsonObject data = doc.object();
    const QJsonArray board = data.value(QStringLiteral("board")).toArray();
    const QJsonArray solution = data.value(QStringLiteral("solution")).toArray();
    if (board.size() != 81 || solution.size() != 81)
        return false;

    bool finished = true;
    for (int i = 0; i < 81; ++i) {
        const int b = board[i].toInt();
        const int s = solution[i].toInt();
        if (!(b == s && b != 0)) {
            finished = false;
            break;
        }
    }
    if (finished)
        return false;

    const QJsonArray initialClues = data.value(QStringLiteral("initial_clues")).toArray();
    const QJsonArray notes = data.value(QStringLiteral("notes")).toArray();
    const int timeSpent = data.value(QStringLiteral("time")).toInt(0);
    if (initialClues.size() == 81) {
        bool hasMoves = false;
        for (int i = 0; i < 81; ++i) {
            if (board[i].toInt() != 0 && !initialClues[i].toBool()) {
                hasMoves = true;
                break;
            }
        }
        bool hasNotes = false;
        for (const QJsonValue &n : notes) {
            if (n.toArray().size() > 0) {
                hasNotes = true;
                break;
            }
        }
        if (!(hasMoves || hasNotes || timeSpent > 10))
            return false;
    }
    return true;
}

void StorageManager::saveGame(const QJsonObject &state)
{
    QFile temp(m_saveFile + QStringLiteral(".tmp"));
    if (!temp.open(QIODevice::WriteOnly))
        return;
    temp.write(QJsonDocument(state).toJson(QJsonDocument::Indented));
    temp.close();
    QFile::remove(m_saveFile);
    QFile::rename(temp.fileName(), m_saveFile);
}

std::optional<QJsonObject> StorageManager::loadGame() const
{
    if (!QFile::exists(m_saveFile))
        return std::nullopt;
    QFile f(m_saveFile);
    if (!f.open(QIODevice::ReadOnly))
        return std::nullopt;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return std::nullopt;
    return doc.object();
}

void StorageManager::deleteSavedGame()
{
    if (QFile::exists(m_saveFile))
        QFile::remove(m_saveFile);
}

QVariantMap StorageManager::getAllHighscores() const
{
    QVariantMap scores{
        {QStringLiteral("simple"), 0},
        {QStringLiteral("easy"), 0},
        {QStringLiteral("intermediate"), 0},
        {QStringLiteral("expert"), 0},
    };
    if (!QFile::exists(m_highscoreFile))
        return scores;
    QFile f(m_highscoreFile);
    if (!f.open(QIODevice::ReadOnly))
        return scores;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return scores;
    const QJsonObject data = doc.object();
    for (auto it = scores.begin(); it != scores.end(); ++it)
        it.value() = data.value(it.key()).toInt(0);
    return scores;
}

int StorageManager::getHighscore(const QString &difficultyKey) const
{
    return getAllHighscores().value(difficultyKey, 0).toInt();
}

bool StorageManager::setHighscore(const QString &difficultyKey, int score)
{
    QVariantMap scores = getAllHighscores();
    const int current = scores.value(difficultyKey, 0).toInt();
    if (score <= current)
        return false;
    scores[difficultyKey] = score;
    QJsonObject obj;
    for (auto it = scores.constBegin(); it != scores.constEnd(); ++it)
        obj.insert(it.key(), it.value().toInt());
    QFile temp(m_highscoreFile + QStringLiteral(".tmp"));
    if (!temp.open(QIODevice::WriteOnly))
        return false;
    temp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    temp.close();
    QFile::remove(m_highscoreFile);
    QFile::rename(temp.fileName(), m_highscoreFile);
    return true;
}
