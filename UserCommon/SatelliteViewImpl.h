#pragma once

#include <vector>

#include "../common/SatelliteView.h"

namespace UserCommon_207177197_301251571
{
    class SatelliteViewImpl : public SatelliteView {
    public:
        explicit SatelliteViewImpl(const std::vector<std::vector<char>>& boardData);

        char getObjectAt(size_t x, size_t y) const override;

        size_t getRows() const { return chars_matrix_.size(); }
        size_t getCols() const { return chars_matrix_.empty() ? 0 : chars_matrix_[0].size(); }

    private:
        std::vector<std::vector<char>> chars_matrix_;
    };
}