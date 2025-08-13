#pragma once

#include <vector>

#include "../common/SatelliteView.h"


class SatelliteViewImpl : public SatelliteView {
public:
    SatelliteViewImpl(const std::vector<std::vector<char>>& boardData);

    char getObjectAt(size_t x, size_t y) const override;

    size_t getRows() const { return board_.size(); }
    size_t getCols() const { return board_.empty() ? 0 : board_[0].size(); }

private:
    std::vector<std::vector<char>> board_;
};
