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

#include "util/StorageProxy.h"

#include <list>
#include <queue>
#include <string>
#include <map>

#include <ostream>
//to be moved
struct Score {
    float points;
    std::string name;

    friend bool operator==(const Score & s1, const Score & s2) {
        return ((s1.points - s2.points) < 0.01f
            && s1.name == s2.name);
    }

    friend std::ostream & operator<<(std::ostream & o, const Score & s) {
        return o << s.name << "|" << s.points;
    }

};

class ScoreStorageProxy : public StorageProxy<Score> {
    public:
        ScoreStorageProxy();

        std::string getValue(const std::string& columnName);

        void setValue(const std::string& columnName, const std::string& value);
};
