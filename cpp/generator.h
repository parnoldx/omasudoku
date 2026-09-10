#pragma once

#include "difficulty.h"

#include <QPair>
#include <QVector>

class SudokuEngine {
public:
    static int solveCount(QVector<int> &board, int limit = 2);
    static QVector<int> generateSolvedBoard();
    static QPair<QVector<int>, QVector<int>> generatePuzzle(const Difficulty &difficulty);

private:
    static int boxIndex(int r, int c) { return (r / 3) * 3 + (c / 3); }
};
