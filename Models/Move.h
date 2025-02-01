#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // Определяем POS_T как 8-битный целочисленный тип (экономия памяти)

// Структура, представляющая ход в игре
struct move_pos
{
    POS_T x, y;             // Начальная позиция (координаты "откуда")
    POS_T x2, y2;           // Конечная позиция (координаты "куда")
    POS_T xb = -1, yb = -1; // Координаты сбитой фигуры (по умолчанию -1, если сбития не было)

    // Конструктор для обычного хода (без взятия фигуры)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2)
    {
    }
    // Конструктор для хода со взятием фигуры (указываются координаты побитой фигуры)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    // Оператор сравнения (равенство ходов)
    bool operator==(const move_pos &other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }
    // Оператор сравнения (неравенство ходов)
    bool operator!=(const move_pos &other) const
    {
        return !(*this == other);
    }
};
