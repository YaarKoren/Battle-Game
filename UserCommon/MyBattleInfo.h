#pragma once

#include <utility>
#include <vector>
#include <tuple>

#include "../common/BattleInfo.h"
#include "../common/SatelliteView.h"
#include "Direction.h"
#include "UserCommonNamespace.h"

USERCOMMON_BEGIN

class MyBattleInfo : public BattleInfo {
public:
    MyBattleInfo(const SatelliteView& view, int playerIndex, size_t rows, size_t cols,
                 std::pair<size_t, size_t> selfPos);

    char getObjectAt(size_t x, size_t y) const;

    size_t getRows() const { return rows_; }
    size_t getCols() const { return cols_; }
    int getPlayerIndex() const { return playerIndex_; }
    std::pair<size_t, size_t> getSelfPosition() const { return selfPos_; }

    Direction inferSelfDirection() const;

private:
    std::vector<std::vector<char>> battlefield_;
    size_t rows_;
    size_t cols_;
    int playerIndex_;
    std::pair<size_t, size_t> selfPos_;
};

USERCOMMON_END