//written with the help of ChatGPT 5
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "MapParser.h"
#include "ErrorMsg.h"

namespace fs = std::filesystem;

MapParser::MapArgs MapParser::parse(const std::string& filename, std::ostringstream& oss)
{
    if (!fs::exists(filename)) {
      throw std::runtime_error(std::string("Map file not found: ") + filename);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Map file found but cannot be opened: ") + filename);
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
         args.num_shells_) = parseMetadata(lines, filename);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }

    try
    {
        args.map_ = parseMap(lines, args.map_height_, args.map_width_, oss, filename);
    }
    catch (const std::exception& e)
    {
    throw std::runtime_error(e.what());
    }

    return args;
}

//get metadata
std::tuple<std::string, size_t, size_t, size_t, size_t> MapParser::parseMetadata (std::vector<std::string>& lines,
    const std::string& filename)
{
    if (lines.size() < 5) {
        throw std::runtime_error(std::string("Missing metadata lines in the map input file: ") + filename);
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
            throw std::runtime_error(std::string("Malformed metadata line in the map input file: ") + filename);
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        int val;
        try {
            val = std::stoi(value);
        } catch (const std::exception&) {
            throw std::runtime_error(std::string("Non-integer value in metadata line: ") + line + std::string("in the map input file: ") + filename);
        }

        if (val < 0) {
            throw std::runtime_error(std::string("Negative value in metadata line: ") + line + std::string("in the map input file: ") + filename);
        }

        if (key == "MaxSteps")        max_steps  = static_cast<size_t>(val);
        else if (key == "NumShells")  num_shells = static_cast<size_t>(val);
        else if (key == "Rows")       map_height = static_cast<size_t>(val);
        else if (key == "Cols")       map_width  = static_cast<size_t>(val);
        else {
            throw std::runtime_error(std::string("Unknown metadata key: ") + key + std::string("in the map input file: ") + filename);
        }
    }

    return {map_name, map_width, map_height, max_steps, num_shells};
}

//get satellite
std::unique_ptr<SatelliteView> MapParser::parseMap(std::vector<std::string>& lines, size_t height, size_t width,
    std::ostringstream& oss, const std::string& filename)
{
    std::vector<std::vector<char>> boardData(height, std::vector<char>(width));

    // Counters for recoverable errors
    size_t ignored_rows = 0, autofilled_rows = 0;
    size_t ignored_cols = 0, autofilled_cols = 0;
    size_t invalid_chars = 0;

    for (size_t y = 0; y < height; ++y) {
        std::string rowLine;

        if (5 + y < lines.size()) {
            rowLine = lines[5 + y];
        } else {
            // not enough rows → autofill empty
            rowLine = std::string(width, ' ');
            ++autofilled_rows;
        }

        if (rowLine.length() < width) {
            autofilled_cols += (width - rowLine.length());
            rowLine += std::string(width - rowLine.length(), ' ');
        } else if (rowLine.length() > width) {
            ignored_cols += (rowLine.length() - width);
            rowLine = rowLine.substr(0, width);
        }

        for (size_t x = 0; x < width; ++x) {
            //under the assumption Tanks are represented only by "1" and "2"
            char c = rowLine[x];
            switch (c)
            {
            case '#': // wall
            case '@': // mine
            case '1': // tank P1
            case '2': // tank P2
            case ' ': // empty
                boardData[y][x] = c; //boardData[row][column] = boardData[y][x]
                break;

            default:
                // invalid char → empty
                    boardData[y][x] = ' ';
                ++invalid_chars;
                break;
            }
        }
    }

    // If there are extra rows beyond `height` → ignored
    if (lines.size() > 5 + height) {
        ignored_rows = lines.size() - (5 + height);
    }

    // Report recoverable errors
    if ( (ignored_rows > 0) || (autofilled_rows > 0) || (autofilled_cols > 0) || (invalid_chars > 0)
    || (invalid_chars > 0) )
    {
        oss << "Recoverable errors found in the map input file: " << filename << ":\n";
    }

    if (ignored_rows > 0)   oss << "Ignored "   << ignored_rows   << " extra row(s)\n";
    if (autofilled_rows > 0) oss << "Autofilled " << autofilled_rows << " missing row(s)\n";
    if (ignored_cols > 0)   oss << "Ignored "   << ignored_cols   << " extra col(s)\n";
    if (autofilled_cols > 0) oss << "Autofilled " << autofilled_cols << " missing col(s)\n";
    if (invalid_chars > 0)  oss << "Ignored "   << invalid_chars  << " invalid char(s)\n";

    // build SatelliteView
    std::unique_ptr<SatelliteView> satellite_view =
        std::make_unique<UserCommon_207177197_301251571::SatelliteViewImpl>(boardData);
    return satellite_view;

}




