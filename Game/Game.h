#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // to start checkers
    int play()
    {
        // Засекаем время начала игры
        auto start = chrono::steady_clock::now();
        // Если идет повтор игры, сбрасываем логику, перезагружаем настройки и перерисовываем доску
        if (is_replay)
        {
            logic = Logic(&board, &config);
            config.reload();
            board.redraw();
        }
        // Иначе рисуем начальное состояние доски
        else
        {
            board.start_draw();
        }
        is_replay = false;

        int turn_num = -1; // Номер хода
        bool is_quit = false;  // Флаг выхода из игры
        const int Max_turns = config("Game", "MaxNumTurns");  // Максимальное количество ходов
        while (++turn_num < Max_turns)
        {
            beat_series = 0;  // Сброс серии ударов
            logic.find_turns(turn_num % 2);  // Поиск доступных ходов для текущего игрока
            // Если ходов нет — выход из игры
            if (logic.turns.empty())
                break;
            // Устанавливаем глубину поиска для бота на основе настроек
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));
            // Проверяем, является ли текущий игрок ботом
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot")))
            {
                // Обрабатываем ход игрока
                auto resp = player_turn(turn_num % 2);
                // Если игрок выбрал выход, завершаем игру
                if (resp == Response::QUIT)
                {
                    is_quit = true;
                    break;
                }
                // Если игрок выбрал повтор игры, запускаем заново
                else if (resp == Response::REPLAY)
                {
                    is_replay = true;
                    break;
                }
                // Если игрок выбрал "назад", отменяем ходы
                else if (resp == Response::BACK)
                {
                    // Если соперник — бот, ход не был ударом, и в истории есть хотя бы 3 состояния, откатываем ходы
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback();
                        --turn_num;
                    }
                    // Откатываем ход игрока
                    if (!beat_series)
                        --turn_num;

                    board.rollback();
                    --turn_num;
                    beat_series = 0;
                }
            }
            else
                // Если текущий игрок — бот, совершаем ход автоматически
                bot_turn(turn_num % 2);
        }
        // Засекаем время окончания игры и записываем в лог
        auto end = chrono::steady_clock::now();
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();

        // Если выбран повтор игры, перезапускаем функцию
        if (is_replay)
            return play();
        // Если игрок выбрал выход — завершаем игру
        if (is_quit)
            return 0;
        // Определяем результат игры:
        // 0 - ничья (по достижению максимального числа ходов)
        // 1 - победа белых
        // 2 - победа черных
        int res = 2;
        if (turn_num == Max_turns)
        {
            res = 0;
        }
        else if (turn_num % 2)
        {
            res = 1;
        }
        // Показываем финальный экран с результатом игры
        board.show_final(res);
        // Ожидаем действия игрока после окончания игры
        auto resp = hand.wait();
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        // Возвращаем результат игры
        return res;
    }

  private:
    void bot_turn(const bool color)
    {
        // Запоминаем время начала хода бота
        auto start = chrono::steady_clock::now();

        // Получаем задержку перед каждым ходом бота из конфигурации
        auto delay_ms = config("Bot", "BotDelayMS");
        // Создаём новый поток, который обеспечит равномерную задержку перед каждым ходом
        thread th(SDL_Delay, delay_ms);
        // Находим лучшие ходы для бота в зависимости от его цвета
        auto turns = logic.find_best_turns(color);
        // Дожидаемся завершения потока с задержкой
        th.join();
        bool is_first = true;
        // Выполняем найденные ходы
        for (auto turn : turns)
        {
            // Для второго и последующих ходов добавляем задержку
            if (!is_first)
            {
                SDL_Delay(delay_ms);
            }
            is_first = false;
            // Увеличиваем счётчик серии ударов, если была взята шашка
            beat_series += (turn.xb != -1);
            // Делаем ход на игровой доске
            board.move_piece(turn, beat_series);
        }

        // Запоминаем время окончания хода бота
        auto end = chrono::steady_clock::now();
        // Логируем время выполнения хода бота в миллисекундах
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();
    }

    Response player_turn(const bool color)
    {
        // return 1 if quit
        // Возвращает Response::QUIT, если игрок хочет выйти из игры
        vector<pair<POS_T, POS_T>> cells;
        // Собираем все возможные ходы игрока
        for (auto turn : logic.turns)
        {
            cells.emplace_back(turn.x, turn.y);
        }
        // Подсвечиваем возможные ходы на доске
        board.highlight_cells(cells);
        move_pos pos = {-1, -1, -1, -1}; // Переменная для хранения выбранного хода
        POS_T x = -1, y = -1;           // Координаты первого выбора клетки игроком
        // trying to make first move
        // Попытка сделать первый ход
        while (true)
        {
            auto resp = hand.get_cell(); // Получаем клетку, выбранную игроком
            // Если игрок выбрал выход, возврат или перезапуск игры
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp);
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)}; // Получаем координаты клетки

            bool is_correct = false; // Проверяем, является ли выбор игрока корректным
            // Проверяем, является ли клетка допустимым ходом
            for (auto turn : logic.turns)
            {
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true;
                    break;
                }
                // Проверяем, является ли это продолжением предыдущего хода
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn;
                    break;
                }
            }
            // Если ход завершен, выходим из цикла
            if (pos.x != -1)
                break;
            // Если выбор клетки некорректен
            if (!is_correct)
            {
                // Сбрасываем предыдущие подсветки и активные клетки
                if (x != -1)
                {
                    board.clear_active();
                    board.clear_highlight();
                    board.highlight_cells(cells);
                }
                x = -1;
                y = -1;
                continue;
            }
            // Запоминаем координаты выбранной клетки
            x = cell.first;
            y = cell.second;
            // Обновляем подсветку и выделяем активную клетку
            board.clear_highlight();
            board.set_active(x, y);
            // Собираем возможные вторые клетки для данного хода
            vector<pair<POS_T, POS_T>> cells2;
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2);
                }
            }
            // Подсвечиваем возможные варианты продолжения хода
            board.highlight_cells(cells2);
        }
        // Завершаем выделение и выполняем ход
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1);
        // Если это не ударная серия ходов, возвращаем OK
        if (pos.xb == -1)
            return Response::OK;
        // continue beating while can
        // Продолжаем серию ударов, если это возможно
        beat_series = 1;
        while (true)
        {
            // Ищем возможные продолжения удара
            logic.find_turns(pos.x2, pos.y2);
            // Если больше бить некого, прерываем серию
            if (!logic.have_beats)
                break;

            // Подсвечиваем возможные клетки для продолжения удара
            vector<pair<POS_T, POS_T>> cells;
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells);
            board.set_active(pos.x2, pos.y2);
            // trying to make move
            // Попытка продолжить серию ударов
            while (true)
            {
                auto resp = hand.get_cell(); // Ждем выбора игрока
                // Если игрок выбрал выход, возврат или перезапуск  
                if (get<0>(resp) != Response::CELL)
                    return get<0>(resp);
                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

                bool is_correct = false;
                // Проверяем, есть ли этот ход среди возможных продолжений
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second)
                    {
                        is_correct = true;
                        pos = turn;
                        break;
                    }
                }
                // Если ход некорректен, ждем другой выбор
                if (!is_correct)
                    continue;

                // Завершаем текущий ход и продолжаем серию ударов
                board.clear_highlight();
                board.clear_active();
                beat_series += 1;
                board.move_piece(pos, beat_series);
                break;
            }
        }

        return Response::OK; // Завершаем ход
    }

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};
