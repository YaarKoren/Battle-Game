#pragma once

#include <vector>
#include <string>
#include <memory>
#include <tuple>

#include "../common/SatelliteView.h"
#include "../UserCommon/Board.h"
#include "../UserCommon/Tank.h"
#include "../UserCommon/Mine.h"
#include "../UserCommon/Wall.h"

#include "../UserCommon/SatelliteViewImpl.h"

class MapParser {
public:
        struct MapArgs {
            std::string map_name_;
            size_t map_width_; //cols num
            size_t map_height_; //rows num
            size_t max_steps_;
            size_t num_shells_;
            UserCommon_207177197_301251571::SatelliteViewImpl map_;
        };

    MapArgs parse(const std::string& filename);

private:
    std::tuple<std::string, size_t, size_t, size_t, size_t> MapParser::parseMetadata(std::vector<std::string> lines);
    UserCommon_207177197_301251571::SatelliteViewImpl parseMap(std::vector<std::string> lines, size_t height, size_t width);
};
