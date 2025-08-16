#include "SatelliteViewImpl.h"

USERCOMMON_BEGIN

SatelliteViewImpl::SatelliteViewImpl(const std::vector<std::vector<char>>& boardData)
        : chars_matrix_(boardData) {}

char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const {
    if (y >= chars_matrix_.size() || x >= chars_matrix_[y].size()) {
        return '&'; // Out of bounds
    }
    return chars_matrix_[y][x];
}

USERCOMMON_END