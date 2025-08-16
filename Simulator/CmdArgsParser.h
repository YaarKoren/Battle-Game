#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstddef>
#include <unordered_map>
#include <cstdlib>
#include <optional>
#include <algorithm>

#include "Mode.h"

class CmdArgsParser {
public:
    struct CmdArgs {
        std::string submitters_ids_;
        Mode mode_;
        bool verbose_ = false; //optional arg; verbose_ == true iff this flag is given
        unsigned threads_num_ = 1; //optional arg; default is 1

        //Comparative
        std::string map_filename_;
        std::string game_managers_folder_name_;
        std::string algorithm1_so_filename_;
        std::string algorithm2_so_filename_;

        //Competition
        std::string maps_folder_name_;
        std::string game_manager_so_name_;
        std::string algos_folder_name_;

    };

    CmdArgs parse(int argc, char* argv[]);


private:
    //helper functions//
    bool hasFlag(int argc, char* argv[], const std::string& flag);
    std::optional<std::string> getFlagValue(int argc, char* argv[], const std::string& flag);
    std::string getAndValidateFileName(int argc, char* argv[], std::string fileName);

    //helper functions to print list of unsupported args
    static inline bool starts_with(const std::string& s, const std::string& prefix);
    std::vector<std::string> collectUnsupportedArgs(int argc, char* argv[],
        const std::vector<std::string>& exactFlags,
        const std::vector<std::string>& kvPrefixes);
    static std::string joinArgs(const std::vector<std::string>& args, const std::string& sep = ", ");



};

