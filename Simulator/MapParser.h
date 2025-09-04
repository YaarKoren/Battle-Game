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
            std::unique_ptr<SatelliteView> map_;
        };

    static MapArgs parse(const std::string& filename, std::ostringstream& oss);

private:
    static std::tuple<std::string, size_t, size_t, size_t, size_t> parseMetadata(std::vector<std::string>& lines,
        const std::string& filename);

    static std::unique_ptr<SatelliteView> parseMap(std::vector<std::string>&lines, size_t height, size_t width,
        std::ostringstream& oss, const std::string& filename);
};

