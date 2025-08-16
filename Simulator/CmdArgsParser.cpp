#include "CmdArgsParser.h"
#include "ErrorMsg.h"

CmdArgsParser::CmdArgs CmdArgsParser::parse(int argc, char* argv[]) {
  CmdArgs args;

  const bool hasComparative = hasFlag(argc, argv, "-comparative");
  const bool hasCompetition = hasFlag(argc, argv, "-competition");

  if (hasComparative && hasCompetition) {
      ErrorMsg::error_and_usage("Choose exactly one mode (comparative or competition), can't have both");
      exit(EXIT_FAILURE);
  }

  if (!hasComparative && !hasCompetition) {
      ErrorMsg::error_and_usage("Missing argument: simulation mode (comparative or competition)");
      exit(EXIT_FAILURE);
  }

  if (hasComparative) {
      args.mode_ = Mode::Comparative;

      //parse aregs for Comparative mode
      args.map_filename_ = getAndValidateFileName(argc, argv, "game_map");
      args.game_managers_folder_name_ = getAndValidateFileName(argc, argv, "game_managers_folder");
      args.algorithm1_so_filename_ = getAndValidateFileName(argc, argv, "algorithm1");
      args.algorithm2_so_filename_ = getAndValidateFileName(argc, argv, "algorithm2");

      //build allow-lists for this mode
      const std::vector<std::string> exactFlags = {
          "-comparative", "-verbose"
      };

      const std::vector<std::string> kvPrefixes = {
          "num_threads=",
          "game_maps_folder=",
          "game_manager=",
          "algorithms_folder=",
      };

      // After parsing, verify no extras
      auto bad = collectUnsupportedArgs(argc, argv, exactFlags, kvPrefixes);
      if (!bad.empty()) {
           ErrorMsg::error_and_usage("Unsupported arguments: " + joinArgs(bad));
          exit(EXIT_FAILURE);
      }

  }

  else { //competition
      args.mode_ = Mode::Competitive;

      //parse args for Competitive mode
      args.maps_folder_name_ = getAndValidateFileName(argc, argv, "game_maps_folder");
      args.game_manager_so_name_ = getAndValidateFileName(argc, argv, "game_manager_so_filename");
      args.algos_folder_name_ = getAndValidateFileName(argc, argv, "algorithms_folder");

      //build allow-lists for this mode
      const std::vector<std::string> exactFlags = {
          "-comparative", "-verbose"
      };

      const std::vector<std::string> kvPrefixes = {
          "num_threads=",
          "game_map_=",
          "game_managers_folder=",
          "algorithm1=",
          "algorithm2="
      };

      // After parsing, verify no extras
      auto bad = collectUnsupportedArgs(argc, argv, exactFlags, kvPrefixes);
      if (!bad.empty()) {
           ErrorMsg::error_and_usage("Unsupported arguments: " + joinArgs(bad));
          exit(EXIT_FAILURE);
      }
  }

  if (hasFlag(argc, argv, "-verbose")) {
    args.verbose_ = true;
  }

  try {
    if (auto val = getFlagValue(argc, argv, "num_threads")) { //if not, no flag, we don't change threads_num_
      if (std::all_of(val->begin(), val->end(), ::isdigit)){ //if its an integer
        if (std::stoi(*val) >= 2) { //if its 1 we don't change
            args.threads_num_ += std::stoi(*val);
        }
      }
      else {
           ErrorMsg::error_and_usage("Illegal argument: num_threads value must be a positive integer");
          exit(EXIT_FAILURE);
      }
    }
  }
  //getFlagValue throws an error if the flag's value is missing
  catch (const std::exception& e) {
     ErrorMsg::error_and_usage(e.what());
    exit(EXIT_FAILURE);
  }
  return args;
}



//helper functions//

// Look for a flag that does NOT take a value
bool CmdArgsParser::hasFlag(int argc, char* argv[], const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (flag == argv[i]) return true;
    }
    return false;
}


// Returns the value for a flag, or std::nullopt if the flag found
// Throws an error if flag is present but value is missing
std::optional<std::string> CmdArgsParser::getFlagValue(int argc, char* argv[], const std::string& flag) {
    const std::string prefix = flag + "=";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind(prefix, 0) == 0) { // starts with "flag="
            std::string value = arg.substr(prefix.size()); //// everything after '='
            if (value.empty()) {
                throw std::runtime_error("Missing value for: " + flag);
            }
            return value; // everything after '='
        }
    }
    return std::nullopt; // not found
}

//it's called file name, but it's also a folder name :)
std::string CmdArgsParser::getAndValidateFileName(int argc, char* argv[], std::string fileName) {
    try {
        if (auto val = getFlagValue(argc, argv, fileName)) {
            return *val; //we will check if the name is legal (there is such file/folder, can be opoend, etc) when trying to open the files
        }
        else {
             ErrorMsg::error_and_usage("Missing argument: " + fileName);
            exit(EXIT_FAILURE);
        }
    }

    //getFlagValue throws an error if the flag's value is missing (the flag is there, but its value is missing)
    catch (const std::exception& e) {
        ErrorMsg::error_and_usage(e.what());
        exit(EXIT_FAILURE);
    }
}








//helper functions to print list of unsupported args

//simple starts_with
inline bool CmdArgsParser::starts_with(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

//collect all unsupported args given exact flags and key=value prefixes
std::vector<std::string> CmdArgsParser::collectUnsupportedArgs(
    int argc, char* argv[],
    const std::vector<std::string>& exactFlags,
    const std::vector<std::string>& kvPrefixes)
{
    std::vector<std::string> bad;

    for (int i = 1; i < argc; ++i) { // skip argv[0]
        const std::string arg = argv[i];

        // skip empty just in case
        if (arg.empty()) continue;

        // supported exact flags?
        const bool isExact = std::find(exactFlags.begin(), exactFlags.end(), arg) != exactFlags.end();

        // supported key=value?
        bool isKV = false;
        if (!isExact) {
            for (const auto& p : kvPrefixes) {
                if (starts_with(arg, p)) { isKV = true; break; }
            }
        }

        if (!isExact && !isKV) {
            bad.push_back(arg);
        }
    }
    return bad;
}


std::string CmdArgsParser::joinArgs(const std::vector<std::string>& args, const std::string& sep) {
    std::string out;
    for (size_t i = 0; i < args.size(); ++i) {
        out += args[i];
        if (i + 1 < args.size()) out += sep;
    }
    return out;
}

