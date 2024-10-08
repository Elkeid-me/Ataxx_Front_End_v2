// Copyright (C) 2021-2024 Elkeid-me
//
// This file is part of Ataxx Frontend.
//
// Ataxx Frontend is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Ataxx Frontend is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Ataxx Frontend.  If not, see <http://www.gnu.org/licenses/>.

#include "GameData.h"
#include <algorithm>
#include <cmath>
#include <ranges>
#include <utility>

static int Chebyshev_distance(std::pair<int, int> point_1, std::pair<int, int> point_2)
{
    int dis_row{std::abs(point_1.first - point_2.first)}, dis_col{std::abs(point_1.second - point_2.second)};

    return std::max(dis_row, dis_col);
}

static bool is_point_legal(std::pair<int, int> point)
{
    return point.first >= 0 && point.first <= 6 &&
           point.second >= 0 && point.second <= 6;
}

Q_INVOKABLE void GameData::new_game()
{
    set_game_state(GameState::black_moving);
    m_start_row = -1;
    m_start_column = -1;
    m_seconds = 0;
    m_steps = 0;
    m_enable_timer = true;

    for (int i : std::views::iota(0, 7))
    {
        for (int j : std::views::iota(0, 7))
            set_cell_state(i, j, CellState::empty);
    }

    set_cell_state(0, 0, CellState::black);
    set_cell_state(6, 6, CellState::black);

    set_cell_state(0, 6, CellState::white);
    set_cell_state(6, 0, CellState::white);
}

Q_INVOKABLE void GameData::clean()
{
    set_game_state(GameState::black_moving);
    m_start_row = -1;
    m_start_column = -1;
    m_seconds = 0;
    m_steps = 0;
    m_enable_timer = false;

    for (int i : std::views::iota(0, 7))
    {
        for (int j : std::views::iota(0, 7))
            set_cell_state(i, j, CellState::empty);
    }
}

void GameData::set_start(int row, int column)
{
    m_start_row = row;
    m_start_column = column;

    CellState new_state{m_game_state == GameState::white_moving ? CellState::white_selected : CellState::black_selected};

    set_cell_state(row, column, new_state);

    for (int i : std::views::iota(row - 2, row + 3))
    {
        for (int j : std::views::iota(column - 2, column + 3))
        {
            if (is_point_legal({i, j}) && m_cell_data[i][j].get_state() == CellState::empty)
            {
                switch (Chebyshev_distance({i, j}, {row, column}))
                {
                case 1:
                    set_cell_state(i, j, CellState::distance_1);
                    break;
                case 2:
                    set_cell_state(i, j, CellState::distance_2);
                    break;
                }
            }
        }
    }
}

void GameData::clean_start()
{
    CellState new_state{m_cell_data[m_start_row][m_start_column].get_state() == CellState::white_selected ? CellState::white : CellState::black};

    set_cell_state(m_start_row, m_start_column, new_state);

    for (int i : std::views::iota(m_start_row - 2, m_start_row + 3))
    {
        for (int j : std::views::iota(m_start_column - 2, m_start_column + 3))
        {
            if (is_point_legal({i, j}) && (m_cell_data[i][j].get_state() == CellState::distance_1 || m_cell_data[i][j].get_state() == CellState::distance_2))
                set_cell_state(i, j, CellState::empty);
        }
    }

    m_start_row = -1;
    m_start_column = -1;
}

Q_INVOKABLE void GameData::click(int row, int column)
{
    if (m_game_state == GameState::white_win || m_game_state == GameState::black_win)
        return;

    if (m_start_row == -1 && m_start_column == -1) // 如果没有确定起点
    {
        // 如果点击的位置是一个合法的起点（即，当前玩家为白方，且点击了白棋；或当前为黑方，且点击了黑棋）
        if ((m_game_state == GameState::white_moving && m_cell_data[row][column].get_state() == CellState::white) ||
            (m_game_state == GameState::black_moving && m_cell_data[row][column].get_state() == CellState::black))
            set_start(row, column); // 那么设定起点
    }
    else // 如果已经确定了起点
    {
        // 如果点击的位置是一个合法的起点（即，当前玩家为白方，且点击了白棋；或当前为黑方，且点击了黑棋）
        if ((m_game_state == GameState::white_moving && m_cell_data[row][column].get_state() == CellState::white) ||
            (m_game_state == GameState::black_moving && m_cell_data[row][column].get_state() == CellState::black))
        {
            clean_start();
            set_start(row, column); // 那么重设起点
        }

        // 如果点击的位置是一个空白块，且这个空白块与起点的切比雪夫距离为 1 或者 2
        else if (m_cell_data[row][column].get_state() == CellState::distance_1)
        {
            int start_row{m_start_row}, start_column{m_start_column};
            clean_start();                                 // 清除起点
            move(start_row, start_column, row, column, 1); // 走一步
        }

        else if (m_cell_data[row][column].get_state() == CellState::distance_2)
        {
            int start_row{m_start_row}, start_column{m_start_column};
            clean_start();                                 // 清除起点
            move(start_row, start_column, row, column, 2); // 走一步
        }
    }
}

void GameData::move(int start_row, int start_column, int destination_row, int destination_column, int Chebyshev_distance)
{
    CellState current_cell_state{m_game_state == GameState::white_moving ? CellState::white : CellState::black};

    CellState enemy_cell_state{current_cell_state == CellState::white ? CellState::black : CellState::white};
    for (int i : std::views::iota(destination_row - 1, destination_row + 2))
    {
        for (int j : std::views::iota(destination_column - 1, destination_column + 2))
        {
            if (is_point_legal({i, j}) && m_cell_data[i][j].get_state() == enemy_cell_state)
                set_cell_state(i, j, current_cell_state);
        }
    }

    set_cell_state(destination_row, destination_column, current_cell_state);

    if (Chebyshev_distance == 2)
        set_cell_state(start_row, start_column, CellState::empty);

    m_steps++;

    GameState new_game_state{m_game_state == GameState::black_moving ? GameState::white_moving : GameState::black_moving};

    set_game_state(new_game_state);
    check_win();
    emit steps_changed();
}

bool GameData::can_move(int row, int column) const
{
    for (int i : std::views::iota(row - 2, row + 3))
    {
        for (int j : std::views::iota(column - 2, column + 3))
        {
            if (is_point_legal({i, j}) && m_cell_data[i][j].get_state() == CellState::empty)
                return true;
        }
    }

    return false;
}

void GameData::check_win()
{
    int n_white{0}, n_black{0};
    bool white_can_move{false}, black_can_move{false};
    for (int i : std::views::iota(0, 7))
    {
        for (int j : std::views::iota(0, 7))
        {
            if (m_cell_data[i][j].get_state() == CellState::white)
            {
                white_can_move = white_can_move || can_move(i, j);
                n_white++;
            }
            else if (m_cell_data[i][j].get_state() == CellState::black)
            {
                black_can_move = black_can_move || can_move(i, j);
                n_black++;
            }
        }
    }

    if (!white_can_move || !black_can_move)
    {
        if (n_white > n_black)
            set_game_state(GameState::white_win);
        else if (n_white < n_black)
            set_game_state(GameState::black_win);
        else
            set_game_state(GameState::draw);

        set_enable_timer(false);
    }
}
