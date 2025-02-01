#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
      // Конструктор класса Logic
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        // Инициализация генератора случайных чисел
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");  // Установка режима подсчета очков
        optimization = (*config)("Bot", "Optimization");    // Установка режима оптимизации
    }

    // Вектор для хранения возможных ходов
    vector<move_pos> find_best_turns(const bool color);
    
    /*
    {
        next_best_state.clear();
        next_move.clear();

        find_first_best_turn(board->get_board(), color, -1, -1, 0);

        int cur_state = 0;
        vector<move_pos> res;
        do
        {
            res.push_back(next_move[cur_state]);
            cur_state = next_best_state[cur_state];
        } while (cur_state != -1 && next_move[cur_state].x != -1);
        return res;
    }
    */


    /*
    * Применяет указанный ход к текущему состоянию доски, обновляя ее.
    * Возвращает новое состояние доски после выполнения хода. 
    */
private:
    // Функция, применяющая ход к текущему состоянию доски
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const
    {
        // Удаляем взятую фигуру, если она есть
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0;
        // Если шашка достигла противоположного края, повышаем ее до дамки
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;
        // Перемещаем фигуру на новую позицию
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        mtx[turn.x][turn.y] = 0; // Убираем фигуру с предыдущей позиции
        return mtx; // Возвращаем обновленное состояние доски
    }


    /*
    * Вычисляет оценку текущего состояния доски для заданного цвета, учитывая количество и положение фигур на доске.
    * Возвращает значение, которое может использоваться для оценки преимущества одного из игроков.
    */
    // Функция, вычисляющая оценку текущего состояния доски для заданного цвета
    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0;  // Счетчики для белых и черных фигур
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                // Увеличиваем счетчики в зависимости от типа фигур
                w += (mtx[i][j] == 1);      // Обычная белая шашка
                wq += (mtx[i][j] == 3);     // Дамка белая
                b += (mtx[i][j] == 2);      // Обычная черная шашка
                bq += (mtx[i][j] == 4);     // Дамка черная
                // Если включен режим "NumberAndPotential", учитываем положение фигур
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i); // Учитываем позицию белых шашек
                    b += 0.05 * (mtx[i][j] == 2) * (i);     // Учитываем позицию черных шашек
                }
            }
        }
        // Меняем местами счетчики, если цвет противника
        if (!first_bot_color)
        {
            swap(b, w);
            swap(bq, wq);
        }
        // Если нет белых фигур, возвращаем бесконечность
        if (w + wq == 0)
            return INF;
        // Если нет черных фигур, возвращаем 0
        if (b + bq == 0)
            return 0;
        int q_coef = 4; // Коэффициент для дамок
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5; // Изменяем коэффициент для режима "NumberAndPotential"
        }
        return (b + bq * q_coef) / (w + wq * q_coef);
    }

    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1);
    /*
    {
        next_best_state.push_back(-1);
        next_move.emplace_back(-1, -1, -1, -1);
        double best_score = -1;
        if (state != 0)
            find_turns(x, y, mtx);
        auto turns_now = turns;
        bool have_beats_now = have_beats;

        if (!have_beats_now && state != 0)
        {
            return find_best_turns_rec(mtx, 1 - color, 0, alpha);
        }

        vector<move_pos> best_moves;
        vector<int> best_states;

        for (auto turn : turns_now)
        {
            size_t next_state = next_move.size();
            double score;
            if (have_beats_now)
            {
                score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, next_state, best_score);
            }
            else
            {
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, 0, best_score);
            }
            if (score > best_score)
            {
                best_score = score;
                next_best_state[state] = (have_beats_now ? int(next_state) : -1);
                next_move[state] = turn;
            }
        }
        return best_score;
    }
    */

    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1);
    /*
    {
        if (depth == Max_depth)
        {
            return calc_score(mtx, (depth % 2 == color));
        }
        if (x != -1)
        {
            find_turns(x, y, mtx);
        }
        else
            find_turns(color, mtx);
        auto turns_now = turns;
        bool have_beats_now = have_beats;

        if (!have_beats_now && x != -1)
        {
            return find_best_turns_rec(mtx, 1 - color, depth + 1, alpha, beta);
        }

        if (turns.empty())
            return (depth % 2 ? 0 : INF);

        double min_score = INF + 1;
        double max_score = -1;
        for (auto turn : turns_now)
        {
            double score = 0.0;
            if (!have_beats_now && x == -1)
            {
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta);
            }
            else
            {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
            }
            min_score = min(min_score, score);
            max_score = max(max_score, score);
            // alpha-beta pruning
            if (depth % 2)
                alpha = max(alpha, max_score);
            else
                beta = min(beta, min_score);
            if (optimization != "O0" && alpha >= beta)
                return (depth % 2 ? max_score + 1 : min_score - 1);
        }
        return (depth % 2 ? max_score : min_score);
    }
    */


/*
* find_turns(const bool color)
* 
* Вызывает find_turns(color, board->get_board()) для поиска ходов всех фигур указанного цвета.
* find_turns(const POS_T x, const POS_T y)
*
* Вызывает find_turns(x, y, board->get_board()) для поиска ходов конкретной фигуры.
* find_turns(const bool color, const vector<vector<POS_T>>& mtx)
*
* Перебирает всю доску, находит возможные ходы для фигур указанного цвета.
* Если есть взятия, оставляет только их.
* Список ходов перемешивается для случайности.
* find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>>& mtx)
*
* Проверяет, может ли фигура на (x, y) совершить взятие (обязательный ход).
* Если взятий нет, ищет обычные ходы.
* Если фигура — дамка, проверяет диагональные движения.
* Эти перегруженные функции используются для определения возможных ходов в зависимости от того, что именно требуется проверить: весь цвет, конкретную фигуру или конкретное положение на доске.
*/
public:
    // Перегруженная функция: ищет возможные ходы для указанного цвета
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board());
    }

    // Перегруженная функция: ищет возможные ходы для конкретной фигуры на позиции (x, y)
    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board());
    }

private:
    // Перегруженная функция: ищет возможные ходы для всех фигур заданного цвета на переданной доске
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx)
    {
        vector<move_pos> res_turns;
        bool have_beats_before = false;  // Флаг, обозначающий, есть ли обязательные взятия
        // Перебираем всю доску
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                // Если на текущей клетке есть фигура и она принадлежит указанному цвету
                if (mtx[i][j] && mtx[i][j] % 2 != color)
                {
                    // Ищем возможные ходы для этой фигуры
                    find_turns(i, j, mtx);
                    // Если обнаружено хотя бы одно взятие, очищаем список ходов (они становятся обязательными)
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    // Если взятия обязательны или их ещё нет, добавляем ходы в список
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        // Обновляем список ходов и перемешиваем их для случайности
        turns = res_turns;
        shuffle(turns.begin(), turns.end(), rand_eng);
        have_beats = have_beats_before;  // Обновляем флаг наличия обязательных взятий
    }

    // Перегруженная функция: ищет возможные ходы для фигуры на позиции (x, y) на переданной доске
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        turns.clear();
        have_beats = false;  // Флаг, обозначающий наличие обязательных взятий
        POS_T type = mtx[x][y];  // Тип фигуры (обычная шашка или дамка)
        // Проверяем возможность взятий (обязательные ходы)
        switch (type)
        {
        case 1: // Белая шашка
        case 2: // Чёрная шашка
            // check pieces
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default: // Дамка
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // Если были найдены взятия, остальные ходы не проверяем
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        // Проверяем обычные (не бьющие) ходы
        switch (type)
        {
        case 1: // Белая шашка
        case 2: // Чёрная шашка
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default: // Дамка
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
      // Вектор для хранения возможных ходов
    vector<move_pos> turns; // Хранит список возможных ходов
    bool have_beats;        // Флаг, указывающий, есть ли обязательные взятия
    int Max_depth;          // Максимальная глубина поиска

  private:
    default_random_engine rand_eng; // Генератор случайных чисел
    string scoring_mode;            // Режим подсчета очков ("NumberAndPotential" и другие)
    string optimization;            // Режим оптимизации
    vector<move_pos> next_move;     // Хранит следующий выбранный ход
    vector<int> next_best_state;    // Хранит состояние для следующего лучшего хода
    Board *board;                   // Указатель на объект доски
    Config *config;                 // Указатель на объект конфигурации
};
