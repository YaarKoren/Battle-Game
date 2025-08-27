#include "SatelliteViewImpl.h"

namespace UserCommon_207177197_301251571
{
    SatelliteViewImpl::SatelliteViewImpl(const std::vector<std::vector<char>>& boardData)
        : board_(boardData) {}

    char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const {
        // First check y, before accessing board_[y]
        if (y >= board_.size()) {
            return '&';
        }
        if (x >= board_[y].size()) {
            return '&';
        }
        return board_[y][x];
    }

}