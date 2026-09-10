#include "game.h"
#include <algorithm>
#include "generator.h"

#include <QJsonArray>
#include <QJsonObject>

SudokuGame::SudokuGame(StorageManager *storage, QObject *parent)
    : QObject(parent)
    , m_difficulty(Difficulty::easy())
{
    if (storage) {
        m_storage = storage;
        m_ownsStorage = false;
    } else {
        m_storage = new StorageManager();
        m_ownsStorage = true;
    }

    m_board.fill(0, 81);
    m_solution.fill(0, 81);
    m_initialClues.fill(false, 81);
    m_notes.resize(81);

    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &SudokuGame::onSecondTick);
}

SudokuGame::~SudokuGame()
{
    if (m_ownsStorage)
        delete m_storage;
}

void SudokuGame::onSecondTick()
{
    if (m_isPaused || !m_inGame)
        return;
    ++m_time;
    emit timeChanged();

    if (m_time % TIME_FACTOR_REDUCE == 0 && m_factor > 1) {
        --m_factor;
        emit factorChanged();
    }
}

void SudokuGame::startNewGame(const QString &difficultyKey)
{
    m_difficulty = Difficulty::fromKey(difficultyKey);
    const auto generated = SudokuEngine::generatePuzzle(m_difficulty);
    m_board = generated.first;
    m_solution = generated.second;
    m_initialClues.resize(81);
    for (int i = 0; i < 81; ++i)
        m_initialClues[i] = m_board[i] != 0;
    m_notes = QVector<QSet<int>>(81);
    m_history.clear();

    m_factor = m_difficulty.factor;
    m_points = 0;
    m_time = 0;
    m_fails = 0;
    m_isPaused = false;
    m_inGame = true;
    m_notesMode = false;

    m_selectedRow = 0;
    m_selectedCol = 0;
    updateHighlightNum();

    m_timer.start();

    emit boardChanged();
    emit scoreChanged();
    emit factorChanged();
    emit failsChanged();
    emit timeChanged();
    emit selectionChanged();
    emit notesModeChanged();
    emit gameStateChanged();
    m_storage->deleteSavedGame();
    emit canResumeChanged();
}

void SudokuGame::resumeGame()
{
    auto dataOpt = m_storage->loadGame();
    if (!dataOpt)
        return;
    const QJsonObject data = *dataOpt;

    m_difficulty = Difficulty::fromKey(data.value(QStringLiteral("difficulty")).toString(QStringLiteral("simple")));

    m_board.fill(0, 81);
    m_solution.fill(0, 81);
    m_initialClues.fill(false, 81);
    const QJsonArray boardArr = data.value(QStringLiteral("board")).toArray();
    const QJsonArray solArr = data.value(QStringLiteral("solution")).toArray();
    const QJsonArray cluesArr = data.value(QStringLiteral("initial_clues")).toArray();
    for (int i = 0; i < 81 && i < boardArr.size(); ++i)
        m_board[i] = boardArr[i].toInt();
    for (int i = 0; i < 81 && i < solArr.size(); ++i)
        m_solution[i] = solArr[i].toInt();
    for (int i = 0; i < 81 && i < cluesArr.size(); ++i)
        m_initialClues[i] = cluesArr[i].toBool();

    m_notes = QVector<QSet<int>>(81);
    const QJsonArray notesArr = data.value(QStringLiteral("notes")).toArray();
    for (int i = 0; i < 81 && i < notesArr.size(); ++i) {
        for (const QJsonValue &v : notesArr[i].toArray())
            m_notes[i].insert(v.toInt());
    }

    m_factor = data.value(QStringLiteral("factor")).toInt(m_difficulty.factor);
    m_points = data.value(QStringLiteral("points")).toInt(0);
    m_time = data.value(QStringLiteral("time")).toInt(0);
    m_fails = data.value(QStringLiteral("fails")).toInt(0);

    m_selectedRow = 0;
    m_selectedCol = 0;
    updateHighlightNum();

    m_isPaused = false;
    m_inGame = true;
    m_notesMode = false;
    m_history.clear();

    m_timer.start();

    emit boardChanged();
    emit scoreChanged();
    emit factorChanged();
    emit failsChanged();
    emit timeChanged();
    emit selectionChanged();
    emit notesModeChanged();
    emit gameStateChanged();
}

void SudokuGame::pauseGame()
{
    m_isPaused = true;
    m_timer.stop();
    emit gameStateChanged();
}

void SudokuGame::resumeTimer()
{
    if (m_inGame && m_isPaused) {
        m_isPaused = false;
        m_timer.start();
        emit gameStateChanged();
    }
}

void SudokuGame::togglePause()
{
    if (m_isPaused)
        resumeTimer();
    else
        pauseGame();
}

void SudokuGame::returnToMenu()
{
    if (m_inGame && !is_finished()) {
        pauseGame();
        save_current_state();
    }
}

void SudokuGame::save_current_state()
{
    if (!m_inGame || is_finished())
        return;

    bool hasMoves = false;
    for (int i = 0; i < 81; ++i) {
        if (m_board[i] != 0 && !m_initialClues[i]) {
            hasMoves = true;
            break;
        }
    }
    bool hasNotes = false;
    for (const auto &n : m_notes) {
        if (!n.isEmpty()) {
            hasNotes = true;
            break;
        }
    }
    if (!(hasMoves || hasNotes || m_time > 10))
        return;

    QJsonObject state;
    state.insert(QStringLiteral("difficulty"), m_difficulty.key);
    QJsonArray boardArr;
    QJsonArray solArr;
    QJsonArray cluesArr;
    QJsonArray notesArr;
    for (int i = 0; i < 81; ++i) {
        boardArr.append(m_board[i]);
        solArr.append(m_solution[i]);
        cluesArr.append(m_initialClues[i]);
        QJsonArray cellNotes;
        QList<int> sorted = m_notes[i].values();
        std::sort(sorted.begin(), sorted.end());
        for (int n : sorted)
            cellNotes.append(n);
        notesArr.append(cellNotes);
    }
    state.insert(QStringLiteral("board"), boardArr);
    state.insert(QStringLiteral("solution"), solArr);
    state.insert(QStringLiteral("initial_clues"), cluesArr);
    state.insert(QStringLiteral("notes"), notesArr);
    state.insert(QStringLiteral("factor"), m_factor);
    state.insert(QStringLiteral("points"), m_points);
    state.insert(QStringLiteral("time"), m_time);
    state.insert(QStringLiteral("fails"), m_fails);

    m_storage->saveGame(state);
    emit canResumeChanged();
}

void SudokuGame::selectCell(int row, int col)
{
    if (row >= 0 && row < 9 && col >= 0 && col < 9) {
        m_selectedRow = row;
        m_selectedCol = col;
        updateHighlightNum();
        emit selectionChanged();
    }
}

void SudokuGame::moveSelection(int dRow, int dCol)
{
    if (m_selectedRow == -1 || m_selectedCol == -1) {
        selectCell(0, 0);
        return;
    }
    const int newRow = qBound(0, m_selectedRow + dRow, 8);
    const int newCol = qBound(0, m_selectedCol + dCol, 8);
    selectCell(newRow, newCol);
}

void SudokuGame::updateHighlightNum()
{
    if (m_selectedRow >= 0 && m_selectedRow < 9 && m_selectedCol >= 0 && m_selectedCol < 9) {
        const int idx = m_selectedRow * 9 + m_selectedCol;
        m_highlightNum = m_board[idx];
    } else {
        m_highlightNum = 0;
    }
}

void SudokuGame::enterNumber(int num)
{
    if (!m_inGame || m_isPaused)
        return;
    if (m_selectedRow < 0 || m_selectedCol < 0)
        return;

    if (num >= 1 && num <= 9 && m_highlightNum != num) {
        m_highlightNum = num;
        emit selectionChanged();
    }

    const int idx = m_selectedRow * 9 + m_selectedCol;
    if (m_initialClues[idx])
        return;

    if (m_notesMode) {
        if (num >= 1 && num <= 9) {
            if (m_notes[idx].contains(num))
                m_notes[idx].remove(num);
            else
                m_notes[idx].insert(num);
            emit boardChanged();
        }
        return;
    }

    if (m_board[idx] != 0)
        return;

    if (num >= 1 && num <= 9) {
        const int expected = m_solution[idx];
        if (num == expected) {
            HistoryEntry entry;
            entry.idx = idx;
            entry.prevVal = 0;
            entry.prevNotes = m_notes[idx];
            entry.pointsAdded = num * m_factor;
            m_history.append(entry);

            m_board[idx] = num;
            m_notes[idx].clear();
            m_points += num * m_factor;
            updateHighlightNum();

            emit cellFlash(m_selectedRow, m_selectedCol, true);
            emit boardChanged();
            emit scoreChanged();
            emit selectionChanged();

            clearPeerNotes(m_selectedRow, m_selectedCol, num);
            checkCompletions(m_selectedRow, m_selectedCol);

            if (is_finished())
                onWon();
            else
                save_current_state();
        } else {
            ++m_fails;
            emit failsChanged();
            emit cellFlash(m_selectedRow, m_selectedCol, false);
        }
    }
}

void SudokuGame::clearPeerNotes(int row, int col, int num)
{
    bool changed = false;
    for (int i = 0; i < 9; ++i) {
        if (m_notes[row * 9 + i].remove(num))
            changed = true;
        if (m_notes[i * 9 + col].remove(num))
            changed = true;
    }
    const int br = (row / 3) * 3;
    const int bc = (col / 3) * 3;
    for (int r = br; r < br + 3; ++r) {
        for (int c = bc; c < bc + 3; ++c) {
            if (m_notes[r * 9 + c].remove(num))
                changed = true;
        }
    }
    if (changed)
        emit boardChanged();
}

void SudokuGame::checkCompletions(int row, int col)
{
    const int bonus = static_cast<int>(POINTS_BLOCK_ROW * m_factor);

    bool colFull = true;
    for (int r = 0; r < 9; ++r) {
        if (m_board[r * 9 + col] == 0) {
            colFull = false;
            break;
        }
    }
    if (colFull) {
        m_points += bonus;
        emit colCompleted(col);
    }

    bool rowFull = true;
    for (int c = 0; c < 9; ++c) {
        if (m_board[row * 9 + c] == 0) {
            rowFull = false;
            break;
        }
    }
    if (rowFull) {
        m_points += bonus;
        emit rowCompleted(row);
    }

    const int br = (row / 3) * 3;
    const int bc = (col / 3) * 3;
    const int boxIdx = (row / 3) * 3 + (col / 3);
    bool boxFull = true;
    for (int r = br; r < br + 3; ++r) {
        for (int c = bc; c < bc + 3; ++c) {
            if (m_board[r * 9 + c] == 0) {
                boxFull = false;
                break;
            }
        }
        if (!boxFull)
            break;
    }
    if (boxFull) {
        m_points += bonus;
        emit boxCompleted(boxIdx);
    }

    emit scoreChanged();
}

void SudokuGame::clearSelected()
{
    if (!m_inGame || m_isPaused)
        return;
    if (m_selectedRow < 0 || m_selectedCol < 0)
        return;
    const int idx = m_selectedRow * 9 + m_selectedCol;
    if (m_initialClues[idx])
        return;
    if (!m_notes[idx].isEmpty()) {
        m_notes[idx].clear();
        emit boardChanged();
    }
}

void SudokuGame::toggleNotesMode()
{
    m_notesMode = !m_notesMode;
    emit notesModeChanged();
}

void SudokuGame::undo()
{
    if (m_history.isEmpty())
        return;
    const HistoryEntry last = m_history.takeLast();
    m_board[last.idx] = last.prevVal;
    m_notes[last.idx] = last.prevNotes;
    m_points = qMax(0, m_points - last.pointsAdded);

    m_selectedRow = last.idx / 9;
    m_selectedCol = last.idx % 9;
    updateHighlightNum();

    emit boardChanged();
    emit scoreChanged();
    emit selectionChanged();
    save_current_state();
}

bool SudokuGame::is_finished() const
{
    for (int i = 0; i < 81; ++i) {
        if (!(m_board[i] == m_solution[i] && m_board[i] != 0))
            return false;
    }
    return true;
}

bool SudokuGame::isFinished() const
{
    return is_finished();
}

void SudokuGame::onWon()
{
    m_timer.stop();
    m_inGame = false;
    m_storage->deleteSavedGame();
    emit canResumeChanged();

    bool isNewRecord = false;
    if (m_fails <= 3)
        isNewRecord = m_storage->setHighscore(m_difficulty.key, m_points);

    const int currentHs = m_storage->getHighscore(m_difficulty.key);
    emit gameWon(m_points, m_fails, currentHs, isNewRecord);
    emit gameStateChanged();
}

QVariantList SudokuGame::board() const
{
    QVariantList list;
    list.reserve(81);
    for (int v : m_board)
        list.append(v);
    return list;
}

QVariantList SudokuGame::initialClues() const
{
    QVariantList list;
    list.reserve(81);
    for (bool v : m_initialClues)
        list.append(v);
    return list;
}

QVariantList SudokuGame::notes() const
{
    QVariantList list;
    list.reserve(81);
    for (const auto &n : m_notes) {
        QVariantList cell;
        QList<int> sorted = n.values();
        std::sort(sorted.begin(), sorted.end());
        for (int v : sorted)
            cell.append(v);
        list.append(QVariant(cell));
    }
    return list;
}

QString SudokuGame::formattedTime() const
{
    const int minutes = m_time / 60;
    const int seconds = m_time % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

bool SudokuGame::canResume() const
{
    return m_storage->hasSavedGame();
}

int SudokuGame::highscore() const
{
    return m_storage->getHighscore(m_difficulty.key);
}

int SudokuGame::getHighscoreFor(const QString &diffKey) const
{
    return m_storage->getHighscore(diffKey);
}
