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
        ErrorMsg::error_and_usage("Map file not found: " + filename);
        exit(EXIT_FAILURE);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        ErrorMsg::error_and_usage("Map file found but cannot be opened: " + filename);
        exit(EXIT_FAILURE);
    }

    MapArgs args;
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }



    if (lines.size() < 5) {
        ErrorMsg::error_and_usage("Map file missing metadata lines");
        exit(EXIT_FAILURE);
    }

    // Parse metadata
    args.map_name_ = lines[0];

    int rows = 0, cols = 0;
    for (int i = 1; i <= 4; ++i) {
        line = lines[i];
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end()); // remove all spaces

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            ErrorMsg::error_and_usage("Malformed metadata line in map file");
            exit(EXIT_FAILURE);
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        int val = std::stoi(value);

        if (key == "MaxSteps")        args.max_steps_ = val;
        else if (key == "NumShells")  args.num_shells_ = val;
        else if (key == "Rows")       args.map_height_ = val;
        else if (key == "Cols")       args.map_width_ = val;
    }

    //get satellite
    std::vector<std::vector<char>> boardData(args.map_height_, std::vector<char>(args.map_width_));

    for (int y = 0;  args.map_height_; ++y) {
        std::string rowLine = (5 + y < (int)lines.size()) ? lines[5 + y] : "";
        if ((int)rowLine.length() < cols)
        {
            rowLine += std::string(cols - rowLine.length(), ' ');
        }
        else if ((int)rowLine.length() > cols)
        {
             rowLine = rowLine.substr(0, cols);
        }

        for (int x = 0;  args.map_width_; ++x)
        {
            boardData[y][x] = rowLine[x]; //boardData[row][column] → boardData[y][x]

        }

    }


    args.map_ = UserCommon_207177197_301251571::SatelliteViewImpl(boardData);

    return args;
}


