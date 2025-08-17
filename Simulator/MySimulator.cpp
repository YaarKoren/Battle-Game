//written with help of ChatGPT 5

#include "MySimulator.h"
#include "ErrorMsg.h"



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
    MapParser mapParser;
    auto mapArgs = mapParser.parse(mapPath); //prints msg and exits on bad args

    // --- 1) dlopen algorithm .so files (auto-registration happens here) ---
    std::vector<std::unique_ptr<SharedLib>> algo_libs; // TankAlgo+Player .so handles
                                                        // Keep SO handles alive for the entire match (RAII)
    algo_libs.reserve(2);

    //load and validate (prints error msg and exits if fails)
    const size_t idx1 = loadAlgoAndPlayerAndGetIndex(algo1SO, algo_libs);
    const size_t idx2 = loadAlgoAndPlayerAndGetIndex(algo2SO, algo_libs);

    //Grab the two entries (we only need them by iterator; no private types leak) - Ask GPT!!!!!
    auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto it1 = algoReg.begin(); std::advance(it1, static_cast<long>(idx1));
    auto it2 = algoReg.begin(); std::advance(it2, static_cast<long>(idx2));

    // Create Player instances for each side using the registered factories
    //    The constructor signature is fixed by the mandatory interface.
    std::unique_ptr<Player> p1 = it1->createPlayer(/*player_index=*/0,
        start_x_p1, start_y_p1, max_steps, num_shells);
    std::unique_ptr<Player> p2 = it2->createPlayer(/*player_index=*/1,
        start_x_p2, start_y_p2, max_steps, num_shells);

    // Build TankAlgorithmFactory callables that delegate to the registrar entries
    //    (We don’t need to fetch the raw factory object; no dlsym needed.)
    TankAlgorithmFactory p1_algo_factory = [it1](int player_index, int tank_index) {
        return it1->createTankAlgorithm(player_index, tank_index);
    };
    TankAlgorithmFactory p2_algo_factory = [it2](int player_index, int tank_index) {
        return it2->createTankAlgorithm(player_index, tank_index);
    };





    // --- 2) dlopen all GameManager .so files from folder (auto-register) ---

    // --- 3) Run each newly registered GameManager on the single map with both algos ---



            //GameResult gr = gm->run(
                //mapArgs.map_width_, mapArgs.map_height_, mapArgs.map_, mapArgs.map_name_,
                //mapArgs.max_steps_, mapArgs.num_shells_,
                //*p1, getCleanFileName(algo1SO), *p2, getCleanFileName(algo2SO),
                //algoFactory1, algoFactory2
            //);

            // TODO: collect results for grouping/printing as per your output spec
            // (you can store gr in a vector and process after the loop)


    // TODO: format and write the comparative results file per your exact spec.

    return failures ? 1 : 0;
}



int MySimulator::runCompetitive() {

  return 0;
}




//helper functions//

//strip the path and ".so" suffix
std::string MySimulator::getCleanFileName(const std::string& path) {
    // find last path separator (handles both Unix / and Windows \)
    auto pos = path.find_last_of("/\\");
    std::string fname = (pos == std::string::npos) ? path : path.substr(pos + 1);

    // strip trailing ".so" if present
    const std::string suffix = ".so";
    if (fname.size() > suffix.size() &&
        fname.compare(fname.size() - suffix.size(), suffix.size(), suffix) == 0) {
        fname.erase(fname.size() - suffix.size());
        }

    return fname;
}

// Return index of the registrar entry that corresponds to this load.
// If the SO is the same path as a previously-loaded one, don't dlopen again;
// just return the existing entry index.
// Validate the loading and registration, prints error msf and exits on failure to load/registrate
 size_t MySimulator::loadAlgoAndPlayerAndGetIndex(
    const std::string& so_path,
    std::vector<std::unique_ptr<SharedLib>>& open_libs // keep handles alive
) {
    auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();

    // If we already have an entry with the same name, reuse it.
    // (createAlgorithmFactoryEntry stored the so_path as the "name()".)
    size_t idx = 0;
    for (auto it = reg.begin(); it != reg.end(); ++it, ++idx) {
        if (it->name() == so_path) {
            return idx; // reuse existing entry (same .so for both players)
        }
    }

    // Create a new slot that the .so's static REGISTER_* objects will fill.
    reg.createAlgorithmFactoryEntry(so_path);

    // Load the library; static objects in that .so will run now and call the
    // PlayerRegistration/TankAlgorithmRegistration constructors,
    // which add factories to the *last* entry we just created.
    try {
        open_libs.emplace_back(std::make_unique<SharedLib>(so_path));
    } catch (const std::exception& e) {
        reg.removeLast(); // rollback empty slot
        ErrorMsg::error_and_usage("Failed to open Algorithm so file\n" + *e.what());  // print with the dlopen error text
        exit(EXIT_FAILURE);
    }

    // Validate that both factories were provided by the .so
    try {
        reg.validateLastRegistration();
    } catch (const AlgorithmRegistrar::BadRegistrationException& bad) {
        reg.removeLast(); // rollback
        std::string msg = "Bad registration in '" + bad.name + "':";
        if (!bad.hasName) msg += " missing name;"; //should not happen; if path argument is missing in cmd args, we catch it before
        if (!bad.hasPlayerFactory) msg += " missing Player factory;";
        if (!bad.hasTankAlgorithmFactory) msg += " missing TankAlgorithm factory;";
        ErrorMsg::error_and_usage("Failed to Register Algorithm so file: " + msg);  // print with the dlopen error text
        exit(EXIT_FAILURE);
    }

    // Return the index of the newly validated entry (last)
    return reg.count() - 1;
}

