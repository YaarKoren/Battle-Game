#include <climits>
#include <queue>
#include <algorithm>
#include <cmath>

#include "../common/TankAlgorithmRegistration.h"
#include "MyTankAlgorithm.h"

namespace Algorithm_207177197_301251571 {
using namespace std;
using namespace UserCommon_207177197_301251571;

MyTankAlgorithm::MyTankAlgorithm(int playerIndex, int tankIndex)
         : playerIndex_(playerIndex),
           tankIndex_(tankIndex)
        {
        //TODO understand if it's right (it's from the ctor of Hunter Algo)
        currentDirection_ = Direction::Up;
        turnsSinceLastUpdate_ = 0;
        isDirectionInitialized_ = false;
           }

void MyTankAlgorithm::updateBattleInfo(BattleInfo& info) {
    auto* myInfoPtr = dynamic_cast<MyBattleInfo*>(&info);
    if (!myInfoPtr) {
        currentInfo_.reset();
        return;
    }
    currentInfo_ = *myInfoPtr;
    turnsSinceLastUpdate_ = 0;

    // Initialize direction if not set
    if (!isDirectionInitialized_) {
        currentDirection_ = myInfoPtr->inferSelfDirection();
        isDirectionInitialized_ = true;
    }
}

ActionRequest MyTankAlgorithm::getAction() {
    turnsSinceLastUpdate_++;

    if (!currentInfo_.has_value() || !currentInfo_->isValid() ||
        turnsSinceLastUpdate_ > UPDATE_INTERVAL) {
        return ActionRequest::GetBattleInfo;
    }

    const MyBattleInfo& info = *currentInfo_;

    // Build grid
    vector<vector<char>> grid(info.getRows(), vector<char>(info.getCols(), ' '));
    Position myPos;
    vector<Position> enemies;
    char enemySymbol = (tankIndex_ == 1) ? '2' : '1';
    char mySymbol = (tankIndex_ == 1) ? '1' : '2';

    for (size_t y = 0; y < info.getRows(); ++y) {
        for (size_t x = 0; x < info.getCols(); ++x) {
            char obj = info.getObjectAt(x, y);
            grid[y][x] = obj;
            if (obj == mySymbol || obj == '%') { // Handle both player symbol and self marker
                myPos = Position((int)x, (int)y);
            } else if (obj == enemySymbol) {
                enemies.emplace_back((int)x, (int)y);
            }
        }
    }

    if (enemies.empty()) {
        return ActionRequest::DoNothing;
    }

    // 🔫 Priority: shoot if enemy directly in the current line of fire
    if (isEnemyInLineOfFire(myPos, grid, enemySymbol)) {
        return ActionRequest::Shoot;
    }

    // Choose closest enemy
    Position target = enemies.front();
    int bestDist = INT_MAX;
    for (const auto& e : enemies) {
        int dist = abs(e.getX() - myPos.getX()) + abs(e.getY() - myPos.getY());
        if (dist < bestDist) {
            bestDist = dist;
            target = e;
        }
    }

    // BFS path (avoids walls and mines)
    currentPath = runBFS(myPos, target, grid);

    if (currentPath.size() < 2) {
        // No path found, try to clear obstacles or rotate
        if (isWallAhead(myPos, grid)) {
            return ActionRequest::Shoot;
        }
        // Rotate toward target if no path
        Direction targetDir = getDirectionTo(myPos, target);
        if (targetDir != currentDirection_) {
            return rotateToward(currentDirection_, targetDir);
        }
        return ActionRequest::DoNothing;
    }

    Position nextStep = currentPath[1];
    Direction needed = getDirectionTo(myPos, nextStep);

    if (needed != currentDirection_) {
        ActionRequest act = rotateToward(currentDirection_, needed);
        // Update local direction model (simplified for 8 directions)
        if (act == ActionRequest::RotateRight90 || act == ActionRequest::RotateRight45) {
            currentDirection_ = static_cast<Direction>((static_cast<int>(currentDirection_) + 1) % 8);
        } else if (act == ActionRequest::RotateLeft90 || act == ActionRequest::RotateLeft45) {
            currentDirection_ = static_cast<Direction>((static_cast<int>(currentDirection_) + 7) % 8);
        }
        return act;
    }

    // If the next tile in our facing is a wall, clear it
    if (isWallAhead(myPos, grid)) {
        return ActionRequest::Shoot;
    }

    return ActionRequest::MoveForward;
}

vector<Position> MyTankAlgorithm::runBFS(const Position& start, const Position& goal,
                                    const vector<vector<char>>& grid) {
    int rows = (int)grid.size();
    int cols = (int)grid[0].size();
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<Position>> parent(rows, vector<Position>(cols, Position(-1, -1)));
    queue<Position> q;

    q.push(start);
    visited[start.getY()][start.getX()] = true;

    // 4-direction movement (up, down, left, right) - simpler for pathfinding
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        Position cur = q.front();
        q.pop();

        if (cur == goal) break;

        for (int i = 0; i < 4; ++i) {
            int nx = cur.getX() + dx[i];
            int ny = cur.getY() + dy[i];

            if (nx < 0 || ny < 0 || nx >= cols || ny >= rows) continue;
            if (visited[ny][nx]) continue;

            char cell = grid[ny][nx];
            if (cell == '#') continue;  // wall
            if (cell == '*') continue;  // avoid mines

            visited[ny][nx] = true;
            parent[ny][nx] = cur;
            q.emplace(nx, ny);
        }
    }

    if (!visited[goal.getY()][goal.getX()]) {
        return {};
    }

    vector<Position> path;
    Position step = goal;
    while (!(step == start)) {
        path.push_back(step);
        step = parent[step.getY()][step.getX()];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

Direction MyTankAlgorithm::getDirectionTo(const Position& from, const Position& to) const {
    int dx = to.getX() - from.getX();
    int dy = to.getY() - from.getY();

    // Simple approximation to 8 directions
    if (abs(dx) > abs(dy)) {
        return (dx > 0) ? Direction::Right : Direction::Left;
    } else {
        return (dy > 0) ? Direction::Down : Direction::Up;
    }
}

ActionRequest MyTankAlgorithm::rotateToward(Direction current, Direction target) const {
    int currentVal = static_cast<int>(current);
    int targetVal = static_cast<int>(target);

    int diff = (targetVal - currentVal + 8) % 8;

    if (diff == 0) return ActionRequest::MoveForward;

    // For small differences, use eighth rotations
    if (diff == 1 || diff == 7) {
        return (diff == 1) ? ActionRequest::RotateRight45 : ActionRequest::RotateLeft45;
    }

    // For larger differences, use 90-degree rotations
    if (diff <= 4) {
        return ActionRequest::RotateRight90;
    } else {
        return ActionRequest::RotateLeft90;
    }
}

bool MyTankAlgorithm::isEnemyInLineOfFire(const Position& myPos,
                                     const vector<vector<char>>& grid,
                                     char enemySymbol) const {
    int rows = (int)grid.size();
    int cols = (int)grid[0].size();

    int dx = 0, dy = 0;
    switch (currentDirection_) {
        case Direction::Up: dy = -1; break;
        case Direction::Down: dy = 1; break;
        case Direction::Left: dx = -1; break;
        case Direction::Right: dx = 1; break;
        case Direction::UpLeft: dx = -1; dy = -1; break;
        case Direction::UpRight: dx = 1; dy = -1; break;
        case Direction::DownLeft: dx = -1; dy = 1; break;
        case Direction::DownRight: dx = 1; dy = 1; break;
    }

    int x = myPos.getX() + dx;
    int y = myPos.getY() + dy;

    while (x >= 0 && y >= 0 && x < cols && y < rows) {
        char cell = grid[y][x];
        if (cell == '#') return false; // wall blocks view
        if (cell == enemySymbol) return true;
        if (cell != ' ') break; // other object blocks view
        x += dx;
        y += dy;
    }
    return false;
}

bool MyTankAlgorithm::isWallAhead(const Position& myPos,
                             const vector<vector<char>>& grid) const {
    int rows = (int)grid.size();
    int cols = (int)grid[0].size();

    int dx = 0, dy = 0;
    switch (currentDirection_) {
        case Direction::Up: dy = -1; break;
        case Direction::Down: dy = 1; break;
        case Direction::Left: dx = -1; break;
        case Direction::Right: dx = 1; break;
        case Direction::UpLeft: dx = -1; dy = -1; break;
        case Direction::UpRight: dx = 1; dy = -1; break;
        case Direction::DownLeft: dx = -1; dy = 1; break;
        case Direction::DownRight: dx = 1; dy = 1; break;
    }

    int nx = myPos.getX() + dx;
    int ny = myPos.getY() + dy;

    if (nx < 0 || ny < 0 || nx >= cols || ny >= rows) return false;
    return (grid[ny][nx] == '#');
}





REGISTER_TANK_ALGORITHM(MyTankAlgorithm);

}
