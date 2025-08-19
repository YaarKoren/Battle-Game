//written with help of ChatGPT 5

#include "MySimulator.h"
#include "ErrorMsg.h"



namespace fs = std::filesystem;


MySimulator::MySimulator(CmdArgsParser::CmdArgs args)
    : args_(std::move(args)) {}

void main(int argc, char* argv[]) {
  std::cout << "1..2.. test\n";
  auto args = CmdArgsParser::parse(argc, argv); //prints msg and exits on bad args
  MySimulator sim(std::move(args));
  sim.run();
}

void MySimulator::run() {
  if (args_.mode_ == Mode::Comparative) {
    MySimulator::runComparative();
  }
  if (args_.mode_ == Mode::Competitive) {
    MySimulator::runCompetitive();
  }
}

void MySimulator::runComparative() {
    const bool verbose = args_.verbose_();
    const std::string mapPath        = args_.map_filename_;
    const std::string managersFolder = args_.game_managers_folder_name_;
    const std::string algo1SO        = args_.algorithm1_so_filename_;
    const std::string algo2SO        = args_.algorithm2_so_filename_;

    // --- 0) Load the map snapshot and params from map file
    MapParser mapParser;
    const auto mapArgs = mapParser.parse(mapPath); //prints msg and exits on bad args
    std::string map_name = mapArgs.map_name_;
    size_t map_width = mapArgs.map_width_;
    size_t map_height = mapArgs.map_height_;
    size_t max_steps = mapArgs.max_steps_;
    size_t num_shells = mapArgs.num_shells_;
    UserCommon_207177197_301251571::SatelliteViewImpl map = mapArgs.map_;


    // --- 1) dlopen algorithm .so files (auto-registration happens here) ---
    // TODO make it a func
    std::vector<std::unique_ptr<SharedLib>> algo_libs; // TankAlgo+Player .so handles
                                                        // Keep SO handles alive for the entire match (RAII)
    algo_libs.reserve(2);

    size_t idx1, idx2;

    //load and validate
    try
    {
        idx1 = loadAlgoAndPlayerAndGetIndex(algo1SO, algo_libs);
    }
    catch (const std::exception& e)
    {
        ErrorMsg::error_and_usage(e.what());
    }

   try
   {
       idx2 = loadAlgoAndPlayerAndGetIndex(algo2SO, algo_libs);
   }
    catch (const std::exception& e)
    {
        ErrorMsg::error_and_usage(e.what());
    }

    //Grab the two entries (we only need them by iterator; no private types leak)
    //it1, it2 are std::vector<AlgorithmAndPlayerFactories>::const_iterator.
    //after advancing, "it1" is like writing: algoReg.algorithms[idx1] (not exactly, but kind of)
    const auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto it1 = algoReg.begin(); std::advance(it1, static_cast<long>(idx1));
    auto it2 = algoReg.begin(); std::advance(it2, static_cast<long>(idx2));

    //Create Player instances for each side using the registered factories
    //The constructor signature is fixed by the mandatory interface.
    std::unique_ptr<Player> player1 = it1->createPlayer(/*player_index=*/1,
        mapArgs.map_width_, mapArgs.map_height_, mapArgs.max_steps_, mapArgs.num_shells_);
    std::unique_ptr<Player> player2 = it2->createPlayer(/*player_index=*/2,
        mapArgs.map_width_, mapArgs.map_height_, mapArgs.max_steps_, mapArgs.num_shells_);

    //Build TankAlgorithmFactory callables that delegate to the registrar entries
    //(We don’t need to fetch the raw factory object; no dlsym needed.)
    TankAlgorithmFactory p1_algo_factory = [it1](int player_index, int tank_index) {
        return it1->createTankAlgorithm(player_index, tank_index);
    };
    TankAlgorithmFactory p2_algo_factory = [it2](int player_index, int tank_index) {
        return it2->createTankAlgorithm(player_index, tank_index);
    };

    // --- 2) dlopen all GameManager .so files from folder (auto-register) ---
    // TODO make it a func

    //get so files from folder
    std::vector<std::string> gm_so_paths;
    gm_so_paths = getSoFilesList(managersFolder);
    if (gm_so_paths.empty()) ErrorMsg::error_and_usage("No .so files in Game Managers dir:  " + managersFolder);
    size_t gm_so_paths_num = gm_so_paths.size();

    //load and validate
    std::vector<std::unique_ptr<SharedLib>> GM_libs; // TankAlgo+Player .so handles
                                                    // Keep SO handles alive for the entire match (RAII)
    GM_libs.reserve(gm_so_paths_num);

    std::vector<size_t> idxs;
    for (const auto& so : gm_so_paths) {
        try
        {
            size_t idx = loadGameManagerAndGetIndex(so, GM_libs);
            idxs.push_back(idx);
        }
        catch (const std::exception& e)
        {
            //TODO: figure what they want us to do with the info that one file failed
            std::cerr << "Error: " << e.what() << "\n\n";
        }
    }

    size_t gm_num = idxs.size();
    if (gm_num == 0) ErrorMsg::error_and_usage("All .so files in Game Managers dir: " + managersFolder + "could not be loaded\n");


    // --- 3) Run each newly registered GameManager on the single map with both algos ---
    const auto& GMReg = GameManagerRegistrar::getGameManagerRegistrar();

    std::vector<GMNameAndResult> name_and_results;

    for (size_t idx : idxs)
    {
        auto it = GMReg.begin(); std::advance(it, static_cast<long>(idx));
        //get the GM name
        std::string name = it->name();
        name = getCleanFileName(name);
        //Create GM instance, using the registered factories
        std::unique_ptr<AbstractGameManager> GM = it->createGameManager(verbose);

        //run game and
        GameResult game_result = GM->run(
                map_width, map_height, map, map_name,
                max_steps, num_shells,
                *player1, getCleanFileName(algo1SO), *player1, getCleanFileName(algo2SO),
                p1_algo_factory, p2_algo_factory);


        //keep GM name and result
        name_and_results.push_back({name, std::move(game_result)});
    }

    // --- 4) format results and print them to the output file / screen ---
    GameResultPrinter::printComparativeResults(name_and_results, managersFolder,
                                                map_width, map_height,
                                                mapPath, algo1SO, algo2SO,
                                                max_steps);
}



void MySimulator::runCompetitive() {
    const bool verbose = args_.verbose_();
    const std::string mapsFolder     = args_.maps_folder_name_;
    const std::string managerPath    = args_.game_manager_so_name_;
    const std::string algosFolder    = args_.algos_folder_name_;

    // --- 0) dlopen Game Manager .so file (auto-registration happens here) ---
    //TODO: maybe improve this (We need only one, no need of vector) and make it a func
    std::vector<std::unique_ptr<SharedLib>> gm_libs; //Game Manager .so handles
                                                     // Keep SO handles alive for the entire match (RAII)
                                                     //create a vector, even thought it's only one so file, cuz this is what out helper func expects to get

    size_t index = 0;
    //TODO: understand if and why index is needed here? only one gm registered in the registrar

    //load and validate
    try
    {
        index = loadGameManagerAndGetIndex(managerPath, gm_libs);
    }
    catch (const std::exception& e)
    {
        ErrorMsg::error_and_usage(e.what());
    }

    //Grab the entry (we only need them by iterator; no private types leak)
    //it is std::vector<GMFactoryNamePair>::const_iterator.
    //after advancing, writing "it" is like writing: GMReg.GMFactories[idx] (not exactly, but kind of)
    const auto& GMReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = GMReg.begin(); std::advance(it, static_cast<long>(index));

    //Create GM instance, using the registered factories
    std::unique_ptr<AbstractGameManager> GM = it->createGameManager(verbose);

    // --- 2) dlopen all Algorithm .so files from folder (auto-register) ---
    // TODO make it a func

    //get so files from folder
    std::vector<std::string> algos_so_paths;
    algos_so_paths = getSoFilesList(algosFolder);
    if (algos_so_paths.empty()) ErrorMsg::error_and_usage("No .so files in Algorithms dir:  " + algosFolder);
    size_t algos_so_paths_num = algos_so_paths.size();

    //load and validate
    std::vector<std::unique_ptr<SharedLib>> algos_libs; // TankAlgo+Player .so handles
                                                        // Keep SO handles alive for the entire match (RAII)
    algos_libs.reserve(algos_so_paths_num);


    std::vector<size_t> idxs;
    for (const auto& so : algos_so_paths) {
        try
        {
            size_t idx = loadAlgoAndPlayerAndGetIndex(so, algos_libs);
            idxs.push_back(idx);
        }
        catch (const std::exception& e)
        {
            //TODO: figure what they want us to do with the info that one file failed
            std::cerr << "Error: " << e.what() << "\n\n";
        }
    }

    size_t N = idxs.size(); //number of sucessfully loaded algos, that are going to play
    if (N == 0) ErrorMsg::error_and_usage("All .so files in Algorithms dir: " + algosFolder + "could not be loaded\n");

    //create a vector of _____ - struct for each file, holding: name + player + algo factory + points, to use later in the loop


    //create Player instance
    //create AlgorithmFactory In stance

    // ---3) for each given map, read map and run the games of N algos on this map, and keep the results

    //maps are in folder - open the file (ENDS WITH .TXT ALWAYS? AND ONLY THOSE?)
    //count map - K

    // --- 4) format results and print them to the output file / screen ---

}




//----------------------------------so files loading - helper functions-------------------------------//

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
// Validate the loading and registration, throws an error on failure to load/registrate
 size_t MySimulator::loadAlgoAndPlayerAndGetIndex(
    const std::string& so_path,
    std::vector<std::unique_ptr<SharedLib>>& open_libs // keep handles alive
) {
    auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();

    // If we already have an entry with the same name, reuse it.
    //(createAlgorithmFactoryEntry() stored the so_path as the "name()".)
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
        throw std::runtime_error("Failed to open Algorithm so file\n" + *e.what());  // with the dlopen error text
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
         throw std::runtime_error("Failed to Register Algorithm so file: " + msg);
    }

    // Return the index of the newly validated entry (last)
    return reg.count() - 1;
}

// Return index of the registrar entry that corresponds to this load.
// If the SO is the same path as a previously-loaded one, don't dlopen again;
// just return the existing entry index.
// Validate the loading and registration, throw an error on failure to load/registrate
 size_t MySimulator::loadGameManagerAndGetIndex(
    const std::string& so_path,
    std::vector<std::unique_ptr<SharedLib>>& open_libs // keep handles alive
) {
    auto& reg = GameManagerRegistrar::getGameManagerRegistrar();

    // If we already have an entry with the same name, reuse it.
    //(createGMFactoryEntry() stores the so_path as the "name()".)
    size_t idx = 0;
    for (auto it = reg.begin(); it != reg.end(); ++it, ++idx) {
        if (it->name() == so_path) {
            return idx; // reuse existing entry
        }
    }

    // Create a new slot that the .so's static REGISTER_* objects will fill.
    reg.createGMFactoryEntry(so_path);

    // Load the library; static objects in that .so will run now and call the
    // GameManagerRegistration constructors,
    // which add factories to the *last* entry we just created.
    try {
        open_libs.emplace_back(std::make_unique<SharedLib>(so_path));
    } catch (const std::exception& e) {
        reg.removeLast(); // rollback empty slot
        throw std::runtime_error ("Failed to open Game Manager so file\n" + *e.what());  // with the dlopen error text
    }

    // Validate that factory was provided by the .so
    try {
        reg.validateLastRegistration();
    } catch (const GameManagerRegistrar::BadRegistrationException& bad) {
        reg.removeLast(); // rollback
        std::string msg = "Bad registration in '" + bad.name + "':";
        if (!bad.hasName) msg += " missing name;"; //should not happen; if path argument is missing in cmd args, we catch it before
        if (!bad.hasGMFactory) msg += " missing Game Manager factory;";
        throw std::runtime_error ("Failed to Register Algorithm so file: " + msg);
    }

    // Return the index of the newly validated entry (last)
    return reg.count() - 1;
}

//iterate through the folder and returns a list paths of files that end with ".so"
std::vector<std::string> getSoFilesList(const std::string& dir_path) {
    std::vector<std::string> file_paths;
    for (const auto& e : fs::directory_iterator(fs::absolute(dir_path))) {
        if (e.is_regular_file() && e.path().extension() == ".so")
            file_paths.push_back(fs::absolute(e.path()).string());
    }
   std::sort(file_paths.begin(), file_paths.end());
   file_paths.erase(std::unique(file_paths.begin(), file_paths.end()), file_paths.end());
   return file_paths;
}

//Return indices of the registrar entries that corresponds to this load
//prints error msg and exits if there are no .so files in the dir
std::vector<size_t> MySimulator::loadTankAlgosAndPlayersFromDir(const std::string& dir,
    std::vector<std::unique_ptr<SharedLib>>& open_libs) // keep handles alive
    {
    auto files = getSoFilesList(dir);
    if (files.empty()) ErrorMsg::error_and_usage("No .so files in Algorithm dir:  " + dir);
    std::vector<size_t> idxs;
    for (const auto& so : files) {
        //TODO: add here try and catch or delete func if not used
        size_t idx = loadAlgoAndPlayerAndGetIndex(so, open_libs);   ////throws error if fails
        idxs.push_back(idx);    // remember indices
    }
    return idxs;
}

//Return indices of the registrar entries that corresponds to this load
//prints error msg and exits if there are no .so files in the dir
std::vector<size_t> MySimulator::loadGMFromDir(const std::string& dir,
    std::vector<std::unique_ptr<SharedLib>>& open_libs) // keep handles alive
    {
    auto files = getSoFilesList(dir);
    if (files.empty()) ErrorMsg::error_and_usage("No .so files in Game Managers dir:  " + dir);
    std::vector<size_t> idxs;
    for (const auto& so : files) {
        //TODO: add here try and catch or delete func if not used
        size_t idx = loadGameManagerAndGetIndex(so, open_libs);   //throws error if fails

        idxs.push_back(idx);    // remember indices
    }
    return idxs;
    }
