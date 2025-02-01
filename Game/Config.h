#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config()
    {
        reload();
    }

    /**
     * Метод открывает файл настроек "settings.json", считывает его содержимое
     * в объект `json config` и затем закрывает файл. Это позволяет динамически
     * обновлять конфигурацию приложения при каждом вызове `reload()`.
     *
     * Определен один оператор круглые скобки который позволяет получать по "setting_dir" и "setting_name" нужную нам настройку.
     * Например, setting_dir это Bot а setting_name это Optimization. Как получить оптимизацию - нужно вызвать круглые скобки
     * от Bot, запятая и Optimization. Как сделано например в файле Game.h 35 строке -
     * const int Max_turns = config("Game", "MaxNumTurns");
     */

    void reload()
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};
