#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "MapParser.h"
#include "ErrorMsg.h"

namespace fs = std::filesystem;

MapParser::MapArgs MapParser::parse(const std::string& filename)
{
    if (!fs::exists(filename)) {
      throw std::runtime_error("Map file not found: " + filename);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Map file found but cannot be opened: " + filename);
    }

    MapArgs args;
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    try
    {
        std::tie(args.map_name_,
         args.map_width_,
         args.map_height_,
         args.max_steps_,
         args.num_shells_) = MapParser::parseMetadata(lines);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }

    try
    {
        args.map_ = MapParser::parseMap(lines, args.map_height_, args.map_width_);
    }
    catch (const std::exception& e)
    {
    throw std::runtime_error(e.what());
    }

    return args;
}

//get metadata
std::tuple<std::string, size_t, size_t, size_t, size_t> MapParser::parseMetadata (std::vector<std::string> lines)
{
    if (lines.size() < 5) {
        throw std::runtime_error("Map file missing metadata lines");
    }

    std::string map_name = lines[0];

    std::string line;

    size_t max_steps  = 0;
    size_t num_shells = 0;
    size_t map_height = 0;
    size_t map_width  = 0;

    for (int i = 1; i <= 4; ++i) {
        line = lines[i];
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end()); // remove all spaces

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            throw std::runtime_error("Malformed metadata line in map file");
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        int val = std::stoi(value);

        if (key == "MaxSteps")        max_steps = val;
        else if (key == "NumShells")  num_shells = val;
        else if (key == "Rows")       map_height = val;
        else if (key == "Cols")       map_width = val;
    }

    return {map_name, map_width, map_height, max_steps, num_shells};
}

//get satellite
std::unique_ptr<SatelliteView> MapParser::parseMap(std::vector<std::string> lines, size_t height, size_t width)
{
    std::vector<std::vector<char>> boardData(height, std::vector<char>(width));

    for (size_t y = 0;  y < height; ++y) {
        std::string rowLine = (5 + y < lines.size()) ? lines[5 + y] : "";
        if (rowLine.length() < width)
        {
            rowLine += std::string(width - rowLine.length(), ' ');
        }
        else if (rowLine.length() > width)
        {
            rowLine = rowLine.substr(0, width);
        }

        for (size_t x = 0; x < width; ++x)
        {
            boardData[y][x] = rowLine[x]; //boardData[row][column] → boardData[y][x]
        }
    }

    std::unique_ptr<SatelliteView> satellite_view =
        std::make_unique<UserCommon_207177197_301251571::SatelliteViewImpl>(boardData);
    return satellite_view;
}




