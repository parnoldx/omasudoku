#pragma once

#include <QString>
#include <array>

struct Difficulty {
    QString key;
    QString label;
    int factor = 28;
    int targetClues = 40;

    static const Difficulty &easy();
    static const Difficulty &medium();
    static const Difficulty &hard();
    static const Difficulty &master();
    static const std::array<Difficulty, 4> &all();
    static Difficulty fromKey(const QString &key);
};
