//written with help of ChatGPT 5

#include "MySimulator.h"

#include <common/TankAlgorithmRegistration.h>

namespace fs = std::filesystem;


MySimulator::MySimulator(CmdArgsParser::CmdArgs args)
    : args_(std::move(args)) {}

int main(int argc, char* argv[]) {
  std::cout << "1..2.. test\n";
  CmdArgsParser parser;
  auto args = parser.parse(argc, argv); //prints msg and exits on bad args

  //Simulator sim(std::move(args));
  //int rc = sim.run();

  //std::cout << "1..2.. test\n";
  //return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  return 0;
}

MapArgs readMap(const std::string& mapPath)
{
    //check if mapPath exists and can open - if not, print usage+err msg

    // returns  0- success,
    //returns 1- problem during reading. what to do in this case? finish also?

    //get relevant params from map, to pass them to Game Manager:
    //size_t map_width, size_t map_height,
    //const SatelliteView& map, // <= a snapshot, NOT updated
    //string map_name,
    //size_t max_steps, size_t num_shells,
    //players name, algos- we don't get them from map




    size_t width=0, height=0, maxSteps=0, numShells=0;
    std::unique_ptr<SatelliteView> initialView;

    if (readMap(mapPath) == 1)
    {
        //what to do
    }
    return 0;
};

int MySimulator::run() {
  if (args_.mode_ == Mode::Comparative) {
    MySimulator::runComparative();
  }

  if (args_.mode_ == Mode::Competitive) {
    MySimulator::runCompetitive();
  }

  return 0;
}



int MySimulator::runComparative() {
    const bool verbose = args_.verbose_();
    const std::string mapPath        = args_.map_filename_;
    const std::string managersFolder = args_.game_managers_folder_name_;
    const std::string algo1SO        = args_.algorithm1_so_filename_;
    const std::string algo2SO        = args_.algorithm2_so_filename_;

    // --- 0) Load the map snapshot and params from map file

    auto mapArgs = readMap(mapPath);






    // --- 1) dlopen algorithm .so files (auto-registration happens here) ---
    const auto algosBefore   = RegistrationAPI::algosSize();
    const auto playersBefore = RegistrationAPI::playersSize();

    SharedLib algo1Lib(algo1SO);
    if (!algo1Lib) return 1;

    // Same file allowed; if same file, one dlopen is enough to register,
    // but opening twice is also fine.
    SharedLib algo2Lib;
    if (algo2SO != algo1SO) {
        algo2Lib = SharedLib(algo2SO);
        if (!algo2Lib) return 1;
    }

    auto allAlgos   = TankAlgorithmRegistration::algos();
    auto allPlayers = RegistrationAPI::players();
    if (allAlgos.size() <= algosBefore) {
        std::cerr << "[algo] no algorithms registered by .so files\n";
        return 1;
    }
    if (allPlayers.size() <= playersBefore) {
        std::cerr << "[algo] no players registered by .so files\n";
        return 1;
    }

    // Pick factories that were added by these dlopen calls.
    // If each .so registers exactly one TankAlgorithmFactory and one PlayerFactory:
    TankAlgorithmFactory algoFactory1 = allAlgos.back();
    // if same .so: both algos refer to the same factory
    TankAlgorithmFactory algoFactory2 = (algo2SO == algo1SO)
        ? allAlgos.back()
        : allAlgos[allAlgos.size() - 2];

    // Choose a PlayerFactory (or track deltas per .so if you expose that):
    PlayerFactory playerFactory = allPlayers.back();

    if (verbose) {
        std::cout << "[load] alg1=" << algo1SO << " alg2=" << algo2SO << "\n";
    }

    // --- 2) dlopen all GameManager .so files from folder (auto-register) ---
    const auto managersBefore = RegistrationAPI::managersSize();
    std::vector<SharedLib> managerLibs; // keep libs alive during the runs
    try {
        for (const auto& e : fs::directory_iterator(managersFolder)) {
            if (!e.is_regular_file()) continue;
            if (e.path().extension() != ".so") continue;
            managerLibs.emplace_back(e.path().string());
            if (verbose) std::cout << "[load GM] " << e.path().filename().string() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[gm] cannot read folder: " << managersFolder << " (" << e.what() << ")\n";
        return 1;
    }

    auto gmFactories = RegistrationAPI::managers();
    if (gmFactories.size() <= managersBefore) {
        std::cerr << "[gm] no GameManager factories registered from folder: " << managersFolder << "\n";
        return 1;
    }

    // --- 3) Run each newly registered GameManager on the single map with both algos ---
    int failures = 0;
    for (size_t i = managersBefore; i < gmFactories.size(); ++i) {
        try {
            auto makeGM = gmFactories[i];
            std::unique_ptr<AbstractGameManager> gm = makeGM(verbose);

            // Create players for this run. Adjust parameters to your PlayerFactory signature.
            auto p1 = playerFactory(/*playerIdx*/1, /*x*/0, /*y*/0, /*maxSteps*/maxSteps, /*numShells*/numShells);
            auto p2 = playerFactory(/*playerIdx*/2, /*x*/0, /*y*/0, /*maxSteps*/maxSteps, /*numShells*/numShells);

            if (verbose) {
                std::cout << "[run] GM#" << i << " on map=" << mapPath << "\n";
            }

            // NOTE: adapt the call below to your exact AbstractGameManager::run(...) signature.
            GameResult gr = gm->run(
                width, height, *initialView, fs::path(mapPath).filename().string(),
                maxSteps, numShells,
                *p1, "algorithm1", *p2, "algorithm2",
                algoFactory1, algoFactory2
            );

            // TODO: collect results for grouping/printing as per your output spec
            // (you can store gr in a vector and process after the loop)

            if (gr.status != GameResult::OK) {
                ++failures;
                if (verbose) std::cerr << "[run] GM#" << i << " returned non-OK\n";
            }
        } catch (const std::exception& e) {
            ++failures;
            std::cerr << "[run] GM#" << i << " threw: " << e.what() << "\n";
        }
    }

    // TODO: format and write the comparative results file per your exact spec.

    return failures ? 1 : 0;
}



int MySimulator::runCompetitive() {

  return 0;
}

