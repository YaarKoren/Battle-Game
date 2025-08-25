#include "SatelliteViewImpl.h"

namespace UserCommon_207177197_301251571
{
    SatelliteViewImpl::SatelliteViewImpl(const std::vector<std::vector<char>>& boardData)
        : board_(boardData) {}

    char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const {
        if (y >= board_.size() || x >= board_[y].size()) {
            return '&'; // Out of bounds
        }
        return board_[y][x];
    }

}