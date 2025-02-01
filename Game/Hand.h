#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// methods for hands
// Класс Hand отвечает за обработку ввода от пользователя (мышь, закрытие окна и т. д.)
class Hand
{
  public:
      // Конструктор принимает указатель на объект Board для взаимодействия с игровой доской
    Hand(Board *board) : board(board)
    {
    }
    // Метод ожидает ввода от пользователя и возвращает кортеж с:
    // - типом действия (Response)
    // - координатами (xc, yc), если выбрана клетка
    tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;     // Координаты клика в пикселях
        int xc = -1, yc = -1;   // Координаты клика в логической сетке доски
        while (true)
        {
            if (SDL_PollEvent(&windowEvent)) // Обрабатываем события
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT: // Если пользователь закрыл окно, отправляем команду выхода
                    resp = Response::QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN: // Обработка клика мышью
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    // Преобразуем пиксельные координаты в координаты клеток доски
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    // Если клик был вне игрового поля и есть история ходов — кнопка "назад"
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK;
                    }
                    // Если клик в определенную область — кнопка "перезапуск игры"
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY;
                    }
                    // Если клик внутри игровой доски (8×8 клеток)
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8)
                    {
                        resp = Response::CELL;
                    }
                    else
                    {
                        // Если клик был в невалидной зоне, сбрасываем координаты
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    // Обработка изменения размера окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size();
                        break;
                    }
                }
                // Если событие обработано (не OK), выходим из цикла
                if (resp != Response::OK)
                    break;
            }
        }
        return {resp, xc, yc};
    }

    // Метод ожидает действий от пользователя (используется, например, в конце игры)
    Response wait() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent)) // Ждем события
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size(); // Адаптируем размеры окна
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    // Получаем координаты клика
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    // Определяем, был ли клик в зоне кнопки "Перезапуск"
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8)
                        resp = Response::REPLAY; // Игрок нажал "перезапуск"
                }
                break;
                }
                // Если событие обработано, выходим из цикла
                if (resp != Response::OK)
                    break;
            }
        }
        return resp;
    }

  private:
    Board *board; // Указатель на игровую доску для взаимодействия
};
