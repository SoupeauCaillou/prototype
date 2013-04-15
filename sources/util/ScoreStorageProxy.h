/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <list>
#include <queue>
#include <string>
#include <map>

//to be moved
struct Score {
    int points;
    int coins;
    std::string name;
};

class ScoreStorageProxy {
    public:
        ScoreStorageProxy();

        std::string getValue(const std::string& columnName);

        void setValue(const std::string& columnName, const std::string& value);

    public:
        std::queue<Score> _queue;
        std::string _tableName;
        std::map<std::string, std::string> _columnsNameAndType;
};