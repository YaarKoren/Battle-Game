#include "ErrorMsg.h"

//maybe print exact exe name instead of "./simulator_<submitter_ids>", using "exe"
void usage_msg(){
    std::cerr <<
          "Usage:\n"
          "Comparative run:\n"
          "./simulator_<submitter_ids> -comparative game_map=<game_map_filename>\n"
          "      game_managers_folder=<game_managers_folder>\n"
          "      algorithm1=<algorithm_so_filename> algorithm2=<algorithm_so_filename>\n"
          "      [num_threads=<num>] [-verbose]\n"
          "Competition run:\n"
          "./simulator_<submitter_ids> -competition game_maps_folder=<game_maps_folder>\n"
          "      game_manager=<game_manager_so_filename>\n"
          "      algorithms_folder=<algorithms_folder>\n"
          "      [num_threads=<num>] [-verbose]\n"
          "\n";
}

void error_and_usage(const std::string& msg) {
    std::cerr << "Error: " << msg << "\n\n";
    usage_msg();
}
