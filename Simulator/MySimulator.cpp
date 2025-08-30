//written with help of ChatGPT 5

#include <scoped_allocator>

#include "MySimulator.h"
#include "ErrorMsg.h"


namespace fs = std::filesystem;


MySimulator::MySimulator(CmdArgsParser::CmdArgs args)
    : args_(std::move(args))
{
    mode_ = args_.mode_;
    verbose_ = args_.verbose_;
    threads_num_ = args_.threads_num_;

    if (mode_ == Mode::Comparative) {
        mapPath        = args_.map_filename_;
        managersFolder = args_.game_managers_folder_name_;
        algo1SO        = args_.algorithm1_so_filename_;
        algo2SO        = args_.algorithm2_so_filename_;
        mapsFolder.clear();
        managerPath.clear();
        algosFolder.clear();

    } else if (mode_ == Mode::Competitive)
    {
        mapsFolder     = args_.maps_folder_name_;
        managerPath    = args_.game_manager_so_name_;
        algosFolder    = args_.algos_folder_name_;
        mapPath.clear();
        managersFolder.clear();
        algo1SO.clear();
        algo2SO.clear();
    }
}

int main(int argc, char* argv[]) {
  //std::cout << "1..2.. test\n";

   //---0) parse cmd args
   std::optional<decltype(CmdArgsParser::parse(argc, argv))> args;

  try {
      args.emplace(CmdArgsParser::parse(argc, argv));
  } catch (const std::exception& e) {
      ErrorMsg::error_and_usage(e.what());;
      return 1;
  } catch (...) { //add general catch block for safety
      ErrorMsg::error_and_usage("unknown error");
      return 1;
  }

    //---1) run the simulaion
  MySimulator sim(std::move(*args));
  try {
    const int rc = sim.run();   // no exit(); the run() function catches errors (and on case of error, print error msg and returns 1)
    //TODO: maybe add printing to screen if run failed or succeeded
    return rc;                  //0- success, 1-fail
  } catch (const std::exception& e) { //for catching errors not caught by run() itself
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 2; // non-zero so shell knows it failed
  } catch (...) {                     //general catch block for safety
     std::cerr << "Fatal unknown error.\n";
     return 3;
  }
}

int MySimulator::run() //int cuz we want the program to always finish gracefully through main, so check for success.
{
    // a string buffer, to build the input_errors file (for the recoverable errors in map + .so file)
    std::ostringstream oss;

    //TODO: separate the run to (1) read input files, then create errors_file or print error and finish (2) run games
    //TODO: pros: make sense, no need to pass oss, multi threading only in the run part
    //TODO: cons: I need all the params from the reading, in the running function

    if (args_.mode_ == Mode::Comparative) {
        try {
           runComparative(oss);
        } catch (const std::exception& e) {
            ErrorMsg::error_and_usage(e.what()); //msg to screen on fatal errors (including input files fatal errors)
            return 1;
        }
            catch (...) {
            std::cout << "Unknown error\n";
            return 1;
        }
    }
    else if (args_.mode_ == Mode::Competitive) {
        try {
            runCompetitive(oss);
        } catch (const std::exception& e) {
            ErrorMsg::error_and_usage(e.what()); //msg to screen on fatal errors (including input files fatal errors)
            return 1;
        }
        catch (...) {
            std::cout << "Unknown error\n";
            return 1;
        }
    }

    //print input_errors to a file / to the screen
    std::string errors_input_content = oss.str();
    if (!errors_input_content.empty()) { //specs: "Create this file only if there are errors"
        //choose destination for the errors_input
        const std::string out_path = "input_errors.txt"; // in working directory (according to forum's specs)
        std::ofstream out(out_path);
        if (!out) {
            std::cerr << "ERROR: cannot create output file: " << out_path
                      << " — printing results to screen instead.\n";
            std::cout << errors_input_content;
        }
        out << errors_input_content;
    }

    return 0;
}


void MySimulator::runComparative(std::ostringstream& oss) {
    // --- 0) Load the map snapshot and params from map file
    std::string map_name;
    size_t map_width;
    size_t map_height;
    size_t max_steps;
    size_t num_shells ;
    std::unique_ptr<SatelliteView> map;

    parse_map(oss, map_name, map_width, map_height,
        max_steps, num_shells, map); //throws an error on bad map data

    // --- 1) dlopen algorithm .so files (auto-registration happens here) ---
    std::unique_ptr<Player> player1;
    std::unique_ptr<Player> player2;
    TankAlgorithmFactory p1_algo_factory;
    TankAlgorithmFactory p2_algo_factory;

    load_and_validate_comparative(player1, player2, p1_algo_factory, p2_algo_factory,
                                map_width, map_height, max_steps, num_shells); //throws an error on failure to open file / load / registrate
                                                                               //no need to pass oss, cuz can't have recoverable errors here

    // --- 2) dlopen all GameManager .so files from folder (auto-register) ---
    std::vector<GMObjectAndName> GMs;
    load_and_validate_comparative(oss, GMs); //throws an error on failure to open file / load / registrate / empty folder

    // --- 3) Run each newly registered GameManager on the single map with both algos ---
    std::vector<GMNameAndResult> GMs_and_results;
    for (const auto& GM_and_name : GMs)
    {
        const std::string name = GM_and_name.name;
        //run game
        GameResult game_result = GM_and_name.GM->run(
                map_width, map_height, *map, map_name,
                max_steps, num_shells,
                *player1, getCleanFileName(algo1SO), *player1, getCleanFileName(algo2SO),
                p1_algo_factory, p2_algo_factory);

        //keep name and result
        GMs_and_results.push_back({name, std::move(game_result)});
    }

    // --- 4) format results and print them to the output file / screen ---
    GameResultPrinter::printComparativeResults(GMs_and_results, managersFolder,
                                                map_width, map_height,
                                                mapPath, algo1SO, algo2SO,
                                                max_steps);
}



void MySimulator::runCompetitive(std::ostringstream& oss) {
    // --- 0) dlopen Game Manager .so file (auto-registration happens here) ---
    std::unique_ptr<AbstractGameManager> GM;
    load_and_validate_competition(GM); //throws an error on failure to open file / load / registrate
                                     //no need to pass oss, cuz can't have recoverable errors here


    // --- 1) dlopen all Algorithm .so files from folder (auto-register) ---
    std::vector<AlgoAndScore> algos_and_scores;
    load_and_validate_competition(oss, algos_and_scores); //throws an error on failure to open file / load / registrate / empty folder / less than 2 algos
    const size_t N = algos_and_scores.size(); //N>=2 , was validated in the function above

    // ---2) for each given map, read map, and keep its MapArgs in a vector
    std::vector<MapParser::MapArgs> maps_data;
    read_maps(oss, maps_data); //throws an error on failure to open file / empty file / bad map data
    size_t K = maps_data.size(); //K > 0, was validated in the function above


    // --- 3) for each read map, run games of the N algos on this map, and keep the results

    //prepare variable for the nested loop
    std::string map_name;
    size_t map_width;
    size_t map_height;
    size_t max_steps;
    size_t num_shells;
    std::unique_ptr<SatelliteView> map;
    int opp;

    bool isOnlyHalfGames;

    for (size_t k = 0; k < K; ++k)
    {
        //parse map data we got
        map_name = maps_data[k].map_name_;
        map_width = maps_data[k].map_width_;
        map_height = maps_data[k].map_height_;
        max_steps = maps_data[k].max_steps_;
        num_shells = maps_data[k].num_shells_;
        map = std::move(maps_data[k].map_);

        isOnlyHalfGames = (N % 2 == 0) && (k == N/2 - 1); //  offset == N/2
                                                          //specs :Note that in the case of k = N/2 - 1 (if N is even),
                                                          //the pairing for each algorithm in both games would be exactly the same.
                                                          //You are supposed to run only one game for each pair in this case

       if (isOnlyHalfGames)
       {
           for (size_t l = 0; l < N/2; ++l )
           {
               opp = getOpponentIdx(l, k, N);
               runGameAndKeepScore(l, opp, algos_and_scores, map_width, map_height,
                   max_steps, num_shells, map_name,
                   map, GM);
           }
       }
       else
       {
           for (size_t l = 0; l < N; ++l )
           {
               opp = getOpponentIdx(l, k, N);
               runGameAndKeepScore(l, opp, algos_and_scores, map_width, map_height,
                    max_steps, num_shells, map_name,
                    map, GM);
           }
       }

    }

    // --- 4) format results and print them to the output file / screen ---
    //make parameters for the printing function
    std::string game_manager_name = getCleanFileName(managerPath);
    //make a vector of algos and scores (no factories) to pass to the printing function
    std::vector<AlgoAndScoreSmall> scores;
    scores.reserve(algos_and_scores.size());
    for (const auto& a : algos_and_scores) {
        scores.push_back({a.name, a.score});
    }
    GameResultPrinter::printCompetitionResults(scores, mapsFolder, game_manager_name, algosFolder);
}



//----------------------------------general helper functions---------------------------------------------
void MySimulator::parse_map(std::ostringstream& oss, std::string& map_name,
    size_t& map_width, size_t& map_height, size_t& max_steps, size_t& num_shells,
    std::unique_ptr<SatelliteView>& map) const
{

    auto map_args = MapParser::parse(mapPath, oss); //throws an error on bad map data

    map_name = map_args.map_name_;
    map_width = map_args.map_width_;
    map_height = map_args.map_height_;
    max_steps = map_args.max_steps_;
    num_shells = map_args.num_shells_;
    map = std::move(map_args.map_);
}



//----------------------------------general helper functions - so files loading-------------------------------//

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
// Validate the loading and registration, throws an error on failure to open file / load / registrate
 size_t MySimulator::loadAlgoAndPlayerAndGetIndex(const std::string& so_path)
{
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
        algo_libs_.emplace_back(std::make_unique<SharedLib>(so_path));
    } catch (const std::exception& e) {
        reg.removeLast(); // rollback empty slot
        throw std::runtime_error(std::string("Failed to open Algorithm .so file\n") + e.what());  // with the dlopen error text
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
         throw std::runtime_error(std::string("Failed to Register Algorithm .so file: ") + msg);
    }

    // Return the index of the newly validated entry (last)
    return reg.count() - 1;
}

// Return index of the registrar entry that corresponds to this load.
// If the SO is the same path as a previously-loaded one, don't dlopen again;
// just return the existing entry index.
// Validate the loading and registration, throw an error on failure to load/registrate
 size_t MySimulator::loadGameManagerAndGetIndex(const std::string& so_path)
{
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
        GM_libs_.emplace_back(std::make_unique<SharedLib>(so_path));
    } catch (const std::exception& e) {
        reg.removeLast(); // rollback empty slot
        throw std::runtime_error (std::string("Failed to open Game Manager .so file\n") + e.what());  // with the dlopen error text
    }

    // Validate that factory was provided by the .so
    try {
        reg.validateLastRegistration();
    } catch (const GameManagerRegistrar::BadRegistrationException& bad) {
        reg.removeLast(); // rollback
        std::string msg = "Bad registration in '" + bad.name + "':";
        if (!bad.hasName) msg += " missing name;"; //should not happen; if path argument is missing in cmd args, we catch it before
        if (!bad.hasGMFactory) msg += " missing Game Manager factory;";
        throw std::runtime_error (std::string("Failed to Register Algorithm .so file: ") + msg);
    }

    // Return the index of the newly validated entry (last)
    return reg.count() - 1;
}

//iterate through the folder and returns a list paths of files that end with ".so"
std::vector<std::string> MySimulator::getSoFilesList(const std::string& dir_path) {
    std::vector<std::string> file_paths;
    for (const auto& e : fs::directory_iterator(fs::absolute(dir_path))) {
        if (e.is_regular_file() && e.path().extension() == ".so")
            file_paths.push_back(fs::absolute(e.path()).string());
    }
   std::sort(file_paths.begin(), file_paths.end());
   file_paths.erase(std::unique(file_paths.begin(), file_paths.end()), file_paths.end());
   return file_paths;
}


//------------------------------comparative mode helper functions - so files loading-----------------------------
//no need of std::ostringstream& oss, cuz can't have recoverable errors here
void MySimulator::load_and_validate_comparative(std::unique_ptr<Player>& player1, std::unique_ptr<Player>& player2,
      TankAlgorithmFactory& p1_algo_factory,  TankAlgorithmFactory& p2_algo_factory,
       const size_t map_width, const size_t map_height, const size_t max_steps, const size_t num_shells)
{

    algo_libs_.reserve(2);

    const size_t idx1 = loadAlgoAndPlayerAndGetIndex(algo1SO); //throws an error on failure to open file / load / registrate
    const size_t idx2 = loadAlgoAndPlayerAndGetIndex(algo2SO); //throws an error on failure to open file / load / registrate

    //Grab the two entries (we only need them by iterator; no private types leak)
    //it1, it2 are std::vector<AlgorithmAndPlayerFactories>::const_iterator.
    //after advancing, "it1" is like writing: algoReg.algorithms[idx1] (not exactly, but kind of)
    const auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto it1 = algoReg.begin(); std::advance(it1, static_cast<long>(idx1));
    auto it2 = algoReg.begin(); std::advance(it2, static_cast<long>(idx2));

    //Create Player instances for each side using the registered factories
    //The constructor signature is fixed by the mandatory interface
    player1 = it1->createPlayerFactory(/*player_index=*/1,
        map_width, map_height, max_steps, num_shells);
    player2 = it2->createPlayerFactory(/*player_index=*/2,
        map_width, map_height, max_steps, num_shells);

    //Build TankAlgorithmFactory callables that delegate to the registrar entries
    //(We don’t need to fetch the raw factory object; no dlsym needed.)
    p1_algo_factory = [it1](int player_index, int tank_index) {
        return it1->createTankAlgorithmFactory(player_index, tank_index);
    };
    p2_algo_factory = [it2](int player_index, int tank_index) {
        return it2->createTankAlgorithmFactory(player_index, tank_index);
    };
}


void MySimulator::load_and_validate_comparative(std::ostringstream& oss, std::vector<GMObjectAndName>& GMs)
{
    //get so files from folder
    std::vector<std::string> gm_so_paths = getSoFilesList(managersFolder);

    //debug printing
    std::cerr << "[SIM] gm_so_paths:\n";
    for (auto& p : gm_so_paths) std::cerr << "  " << p << "\n";

    if (gm_so_paths.empty()) {
        throw std::runtime_error(std::string("No .so files in Game Managers dir:  ") + managersFolder);
    }

    const size_t gm_so_paths_num = gm_so_paths.size();

    //load and validate
    GM_libs_.reserve(gm_so_paths_num);

    std::vector<size_t> indices;

    for (const auto& so : gm_so_paths) {
        try {
            size_t idx = loadGameManagerAndGetIndex(so);
            indices.push_back(idx);
        } catch (const std::exception& e) {
            //add to oss (=input_errors file) the info about the error; it includes the file path (see implementation of SharedLib)
            oss << "Error in Game Manger .so file:\n" << e.what() << "\n\n";
        }
    }

    //debug printing
    std::cerr << "[SIM] indices:\n";
    for (auto i : indices) std::cerr << "  " << i << "\n";

    size_t gm_num = indices.size();
    if (gm_num == 0) {
        throw std::runtime_error (std::string("All .so files in Game Managers dir: ") + managersFolder + std::string("could not be loaded"));
    }

    //for each .so file, create GM and keep it and its name in the vector
    const auto& GMReg = GameManagerRegistrar::getGameManagerRegistrar();
    for (size_t idx : indices)
    {
        auto it = GMReg.begin(); std::advance(it, static_cast<long>(idx));
        //get the GM name
        std::string name = it->name();
        name = getCleanFileName(name);
        //Create GM instance, using the registered factories
        std::unique_ptr<AbstractGameManager> GM = it->createGameManagerFactory(verbose_);

        //keep GM and its name
        GMs.push_back({name, std::move(GM)});
    }

}





//------------------------------competition mode - helper function--------------------------------
//iterate through the folder and returns a list paths of files
//TODO: maybe check if the files ends with ".txt"?
//TODO: unite with prev func to avoid duplicates
std::vector<std::string> MySimulator::getFilesList(const std::string& dir_path) {
    std::vector<std::string> file_paths;
    for (const auto& e : fs::directory_iterator(fs::absolute(dir_path))) {
            file_paths.push_back(fs::absolute(e.path()).string());
    }
    std::sort(file_paths.begin(), file_paths.end());
    file_paths.erase(std::unique(file_paths.begin(), file_paths.end()), file_paths.end());
    return file_paths;
}

//assuming N >=2, l >= 0, k > 0
int MySimulator::getOpponentIdx(const size_t l, const size_t k, const size_t N)
{
    // offset = 1 + (k % (N-1))
    const size_t offset = 1 + (k % (N - 1));

    // opponent index per spec: (l + offset) % N
    const size_t opp = (static_cast<size_t>(l) + offset) % N;

    return static_cast<int>(opp);
}






void MySimulator::runGameAndKeepScore(const size_t l, const int opp, std::vector<AlgoAndScore>& algos_and_scores,
    const size_t map_width, const size_t map_height, const size_t max_steps, const size_t num_shells,
    const std::string& map_name,
    const std::unique_ptr<SatelliteView>& map,
    const std::unique_ptr<AbstractGameManager>& GM
    )
{
    //create Player for the l-th algo and for its opponent
    std::unique_ptr<Player> player1 = algos_and_scores[l].player_factory(/*player_index=*/1,
    map_width, map_height, max_steps, num_shells);

    std::unique_ptr<Player> player2 = algos_and_scores[opp].player_factory(/*player_index=*/2,
    map_width, map_height, max_steps, num_shells);

    //run the game with the 2 creates players and their corresponding tank algorithms
    const GameResult game_result = GM->run(
        map_width, map_height, *map, map_name,
        max_steps, num_shells,
        *player1, algos_and_scores[l].name, *player1, algos_and_scores[opp].name,
        algos_and_scores[l].algo_factory, algos_and_scores[opp].algo_factory);

    //check the result and score accordingly
    if (game_result.winner == 0)
    {
        algos_and_scores[l].score += 1;
        algos_and_scores[opp].score += 1;
    }
    else if (game_result.winner == 1)
    {
        algos_and_scores[l].score += 3;
    }
    else if (game_result.winner == 2)
    {
        algos_and_scores[opp].score += 3;
    }


}




//no need of std::ostringstream& oss, cuz can't have recoverable errors here
void MySimulator::load_and_validate_competition(std::unique_ptr<AbstractGameManager>& GM)
{
    size_t index = 0;

    //we usr vector gm_libs_, even thought it's only one so file, for convenience (one member, one function, for both comparative and competition mode)

    index = loadGameManagerAndGetIndex(managerPath); //throws an error on failure to open file / load / registrate
    //Grab the entry (we only need them by iterator; no private types leak)
    //it is std::vector<GMFactoryNamePair>::const_iterator.
    //after advancing, writing "it" is like writing: GMReg.GMFactories[idx] (not exactly, but kind of)
    const auto& GMReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = GMReg.begin(); std::advance(it, static_cast<long>(index));

    //Create GM instance, using the registered factories
    GM = it->createGameManagerFactory(verbose_);
}

void MySimulator::load_and_validate_competition(std::ostringstream& oss, std::vector<AlgoAndScore>& algos_and_scores)
{
    std::vector<size_t> indices;

    //get so files from folder
    std::vector<std::string> algos_so_paths = getSoFilesList(algosFolder);
    if (algos_so_paths.empty()) {
        throw std::runtime_error((std::string("No .so files in algorithms dir: ") + algosFolder));
    }
    size_t algos_so_paths_num = algos_so_paths.size();
    if (algos_so_paths_num < 2) {
        throw std::runtime_error(std::string("There are less than 2 algorithm .so files in the algorithms folder: ") + algosFolder);
    }

    //load and validate
    algo_libs_.reserve(algos_so_paths_num);

    for (const auto& so : algos_so_paths) {
        try
        {
            size_t idx = loadAlgoAndPlayerAndGetIndex(so);
            indices.push_back(idx);
        }
        catch (const std::exception& e)
        {
            //add to oss (=input_errors file) the info about the error; it includes the file path (see implementation of SharedLib)
            oss << "Error in Game Manger .so file:\n" << e.what() << "\n\n";
        }
    }

    const size_t N = indices.size(); //number of successfully loaded algos, that are going to play
    if (N == 0) {
        throw std::runtime_error (std::string("All .so files in the algorithms folder: ") + algosFolder + std::string("could not be loaded"));
    } else if (N == 1) {
        throw std::runtime_error (std::string("Only one .so file in the algorithms folder: ") + algosFolder + std::string(" could be loaded"));
    }

    //algos_and_scores - each vector entry holds an AlgoAndScore for a different so file loaded successfully
    //struct holds: name + player factory + tank algorithm factory + points
    algos_and_scores.reserve(N);
    const auto& algoReg = AlgorithmRegistrar::getAlgorithmRegistrar();
    for (size_t i = 0; i < N; ++i)
    {
        auto iter = algoReg.begin(); std::advance(iter, static_cast<long>(indices[i]));

        //get algo name and keep it
        algos_and_scores[i].name = getCleanFileName(iter->name());

        //create and keep Player Factory
        algos_and_scores[i].player_factory =
            [iter](int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
            {
                return iter->createPlayerFactory(player_index, x, y, max_steps, num_shells);
            };

        //create and keep Algorithm Factory
        algos_and_scores[i].algo_factory =
            [iter](int player_index, int tank_index)
            {
                return iter->createTankAlgorithmFactory(player_index, tank_index);
            };

        //initialize score as 0
        algos_and_scores[i].score = 0;
    }
}

void MySimulator::read_maps(std::ostringstream& oss, std::vector<MapParser::MapArgs>& maps_data) const
{
    const std::vector<std::string> maps_paths = getFilesList(mapsFolder);
    const size_t maps_num = maps_paths.size();

    if (maps_num == 0) {
        throw std::runtime_error (std::string("There are no no maps file in the maps folder: ") + mapsFolder);
    }

    for (size_t i = 0; i < maps_num; ++i)
    {
        //try to read and catch errors
        try
        {
            maps_data.push_back(MapParser::parse(maps_paths[i], oss)); //recoverable errors in the map is written to oss
        }

        catch (const std::exception& e)
        {
            //map that had unrecoverable error - we document the error in oss and don't add it to the maps vector
            oss << "Unrecoverable error in map file:\n" << mapPath << ":\n " << e.what() << "\n\n";
        }
    }

    size_t K = maps_data.size(); //number of maps successfully read

    if (K == 0) { // no maps were read successfully

        throw std::runtime_error (std::string("No maps were read successfully from the maps folder: ") + mapsFolder);
    }

}
