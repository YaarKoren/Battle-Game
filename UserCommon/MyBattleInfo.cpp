#include "MyBattleInfo.h"

USERCOMMON_BEGIN

MyBattleInfo::MyBattleInfo(const SatelliteView& view, int playerIndex, size_t rows,
                           size_t cols, std::pair<size_t, size_t> selfPos)
        : battlefield_(rows, std::vector<char>(cols, ' ')),
          rows_(rows), cols_(cols),
          playerIndex_(playerIndex),
          selfPos_(selfPos) {

    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            char obj = view.getObjectAt(x, y);
            battlefield_[y][x] = (x == selfPos.first && y == selfPos.second) ? '%' : obj;
        }
    }
}

char MyBattleInfo::getObjectAt(size_t x, size_t y) const {
    if (x >= cols_ || y >= rows_) return '&';
    return battlefield_[y][x];
}

Direction MyBattleInfo::inferSelfDirection() const {
    int x = static_cast<int>(selfPos_.first);
    int y = static_cast<int>(selfPos_.second);

    const std::vector<std::tuple<int, int, Direction>> directions = {
        {1, 0, Direction::Right},
        {-1, 0, Direction::Left},
        {0, 1, Direction::Down},
        {0, -1, Direction::Up}
    };

    for (const auto& [dx, dy, dir] : directions) {
        int nx = x + dx;
        int ny = y + dy;
        if (nx >= 0 && ny >= 0 && nx < static_cast<int>(cols_) && ny < static_cast<int>(rows_)) {
            if (battlefield_[ny][nx] == '%') {
                return dir;
            }
        }
    }

    return (playerIndex_ == 1) ? Direction::Right : Direction::Left;
}

USERCOMMON_END