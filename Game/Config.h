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
     * ����� ��������� ���� �������� "settings.json", ��������� ��� ����������
     * � ������ `json config` � ����� ��������� ����. ��� ��������� �����������
     * ��������� ������������ ���������� ��� ������ ������ `reload()`.
     *
     * ��������� ���� �������� ������� ������ ������� ��������� �������� �� "setting_dir" � "setting_name" ������ ��� ���������.
     * ��������, setting_dir ��� Bot � setting_name ��� Optimization. ��� �������� ����������� - ����� ������� ������� ������
     * �� Bot, ������� � Optimization. ��� ������� �������� � ����� Game.h 35 ������ -
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
