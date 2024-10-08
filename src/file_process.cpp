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
#include <fstream>
#include <ranges>

Q_INVOKABLE void GameData::save()
{
    unsigned long long black{0}, white{0};
    for (int i : std::views::iota(0, 7))
    {
        for (int j : std::views::iota(0, 7))
        {
            if (m_cell_data[i][j].get_state() == CellState::black || m_cell_data[i][j].get_state() == CellState::black_selected)
                black |= 1ULL << (i << 3 | j);
            if (m_cell_data[i][j].get_state() == CellState::white || m_cell_data[i][j].get_state() == CellState::white_selected)
                white |= 1ULL << (i << 3 | j);
        }
    }
    std::ofstream out_file_stream("data.wzqtql", std::ios::out);
    out_file_stream << "black: " << black << std::endl
                    << "white: " << white << std::endl
                    << "steps: " << m_steps << std::endl
                    << "seconds: " << m_seconds << std::endl
                    << "game_state: " << static_cast<int>(m_game_state) << std::endl;
    out_file_stream.close();
}

Q_INVOKABLE void GameData::load()
{
    char tmp[16]{0};
    int game_state{0};

    unsigned long long black{0}, white{0};

    std::ifstream load_file_stream("data.wzqtql", std::ios::in);
    load_file_stream >> tmp >> black >>
        tmp >> white >>
        tmp >> m_steps >>
        tmp >> m_seconds >>
        tmp >> game_state;
    load_file_stream.close();

    m_game_state = static_cast<GameState>(game_state);

    unsigned long long temp_black{0}, temp_white{0};

    for (int i : std::views::iota(0, 7))
    {
        for (int j : std::views::iota(0, 7))
        {
            temp_black = 1ULL & (black >> (i << 3 | j));
            temp_white = 1ULL & (white >> (i << 3 | j));
            if (temp_black)
                m_cell_data[i][j].set_state(CellState::black);
            else if (temp_white)
                m_cell_data[i][j].set_state(CellState::white);
            else
                m_cell_data[i][j].set_state(CellState::empty);
        }
    }

    set_enable_timer(true);
}
