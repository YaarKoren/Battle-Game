//written with help of CHATGPT 5

#include "GameResultPrinter.h"

#define MAX_STEPS_AFTER_SHELLS_END 40
#define COMPARATIVE_STR_FOR_UNQ_PATH "comparative_results"
#define COMPETITION_STR_FOR_UNQ_PATH "competition"


void GameResultPrinter::printComparativeResults(
    std::vector<GMNameAndResult> results,
    std::string folder_path,
    size_t map_width, size_t map_height,
    std::string game_map_filename,
    std::string algo1_so_filename,
    std::string algo2_so_filename,
    size_t max_steps) {

    //---1) Build (key,item*) rows
    std::vector<std::pair<ResultKey, const GMNameAndResult*>> rows;
    rows.reserve(results.size());
    for (const auto& res : results) {
      rows.push_back({ makeKey(res.result, map_width, map_height), &res });
    }

    //---2)Sort by key
    std::sort(rows.begin(), rows.end(),
          [](const auto& A, const auto& B){ return ResultKeyLess{}(A.first, B.first); });

    //---3) Scan consecutive equal keys into groups
    std::vector<Group> groups;
    for (size_t i = 0; i < rows.size(); ) {
        size_t j = i + 1;

        while (j < rows.size() && rows[j].first == rows[i].first) {
            ++j;
        }

        Group g{ /*ResultKey*/rows[i].first, /*vector<GMNameAndResult>*/{} };
        g.members.reserve(j - i); //the vector<GMNameAndResult>'s size
        for (size_t k = i; k < j; ++k) g.members.push_back(rows[k].second); //push GMNameAndResult of games that got the same result
        groups.push_back(std::move(g));

        i = j; // advance to next group
    }

    //---4) Sort groups; Put the biggest group first (deterministic tie-break optional)
    auto name_of = [](const Group& g){
        return g.members.empty() ? std::string() : g.members.front()->name;
    };
    std::sort(groups.begin(), groups.end(), [&](const Group& a, const Group& b){
      if (a.members.size() != b.members.size()) return a.members.size() > b.members.size();
      return name_of(a) < name_of(b);
    });

    //---5) Try to open file, else fallback to screen with error
    std::string path = makeUniquePath(folder_path, COMPARATIVE_STR_FOR_UNQ_PATH);
    std::ofstream out(path);
    bool to_screen = false;
    if (!out) {
        std::cerr << "ERROR: cannot create output file: " << path
                  << " — printing results to screen instead.\n";
        to_screen = true;
    }
    std::ostream& os = to_screen ? std::cout : out;

    //---6) Header (lines 1–4)
    os << "game_map=" << game_map_filename << "\n";
    os << "algorithm1=" << algo1_so_filename << "\n";
    os << "algorithm2=" << algo1_so_filename << "\n";
    os << "\n";

    //---7) Groups info
    auto emit_group = [&](const Group& g){ //a local lambda function
      									   //[&] → capture everything by reference (so the lambda can use os,
											//max_steps, etc., from the outer scope).

        // 5th line: comma-separated biggest list of GM names (or for subsequent groups the list as well)
        for (size_t i=0;i<g.members.size();++i) {
            if (i) os << ",";
            os << g.members[i]->name;
        }
        os << "\n";

        // 6th line: result message (as in assignment 2)
        os << resultMessage(g.members.front()->result, max_steps) << "\n";

        // 7th line: round number
        os << g.members.front()->result.rounds << "\n";

        // 8th line on: full map
        os << g.key.final_snapshot;
    };

    if (!groups.empty()) {
        emit_group(groups.front()); //.front() returns a reference to the first element in the vector
        for (size_t i=1;i<groups.size();++i) {
            os << "\n"; // empty line between groups
            emit_group(groups[i]);
        }
    }
}

void GameResultPrinter::printCompetitionResults(const std::vector<AlgoAndScoreSmall>& algos_and_scores,
                                  const std:: string& maps_folder_path,
                                  const std::string& game_manager_clean_name,
                                  const std::string& algos_folder_path) {

    // 1) Build the full output into a string buffer
    std::ostringstream oss;

    // Lines 1–3
    oss << "game_maps_folder=" << maps_folder_path << "\n";
    oss << "game_manager="     << game_manager_clean_name << "\n";
    oss << "\n";

    // Copy then sort by total score (desc). Tie-breaker by name (asc) for determinism.
    //specs: “Algorithms with the same score can appear in any order.”
    //added a name tie‑break so output is deterministic
    std::vector<AlgoAndScoreSmall> rows = algos_and_scores;
    std::stable_sort(rows.begin(), rows.end(),
        [](const AlgoAndScoreSmall a, const AlgoAndScoreSmall& b){
            if (a.score != b.score) return a.score > b.score;   // higher first
            return a.name < b.name;                              // tie-break
        });

    // Lines 4+ : "<algorithm name> <total score>"
    for (const auto& r : rows) {
        oss << r.name << ' ' << r.score << "\n";
    }

    const std::string content = oss.str();

    // 2) Decide destination: file under *algorithms folder* or screen on failure.
    const std::string out_path = makeUniquePath(algos_folder_path, COMPETITION_STR_FOR_UNQ_PATH);

    std::ofstream out(out_path);
    if (!out) {
        std::cerr << "ERROR: cannot create output file: " << out_path
                  << " — printing results to screen instead.\n";
        std::cout << content;
        return;
    }

    out << content;

}




// time helper function
std::string GameResultPrinter::makeUniquePath(std::string folder_path, std::string mode_name) {
    using namespace std::chrono;
    namespace fs = std::filesystem;

    constexpr std::size_t NUM_DIGITS   = 9;                  // width to print
    constexpr std::uint64_t NUM_DIGITS_P = 1'000'000'000ULL; // 10^9

    // Try a few variants (time + i) to dodge a same-tick collision.
    for (int i = 0; i < 100; ++i) {
        // microsecond resolution is plenty; cast for a stable integer
        auto now_us = time_point_cast<microseconds>(system_clock::now());
        std::uint64_t ticks = static_cast<std::uint64_t>(now_us.time_since_epoch().count());

        // Bounded, fixed-width numeric token (padded with leading zeros)
        std::uint64_t token = (ticks + static_cast<std::uint64_t>(i)) % NUM_DIGITS_P;

        std::ostringstream num;
        num << std::setw(NUM_DIGITS) << std::setfill('0') << token;

        std::string path = folder_path + "/" + mode_name + "_" + num.str() + ".txt";

        // If it doesn't exist, we're good.
        if (!fs::exists(path)) return path;
    }

    // Fallback: unbounded tick value (unique enough even if files exist)
    auto now_us = time_point_cast<microseconds>(system_clock::now());
    return folder_path + "/" + mode_name + "_" + std::to_string(now_us.time_since_epoch().count()) + ".txt";
}





//---------------------Comparative mode - helper functions-----------------------

GameResultPrinter::ResultKey GameResultPrinter::makeKey(const GameResult& r, size_t map_width, size_t map_height) {
    return {
        r.winner,
        r.reason,
        r.rounds,
        r.gameState ? renderView(*r.gameState, map_width, map_height) : blankSnapshot(map_width, map_height)
    };
}

std::string GameResultPrinter::renderView(const SatelliteView& sv,
                                          size_t map_width, size_t map_height) {
    std::string s;
    s.reserve((map_width + 1) * map_height);
    for (size_t y = 0; y < map_height; ++y) {
        for (size_t x = 0; x < map_width; ++x) s.push_back(sv.getObjectAt(x, y));
        s.push_back('\n');
    }
    return s;
}

std::string GameResultPrinter::blankSnapshot(size_t map_width, size_t map_height)  {
    std::string s;
    s.reserve((map_width + 1) * map_height);
    for (size_t y = 0; y < map_height; ++y) {
        s.append(map_width, '?');
        s.push_back('\n');
    }
    return s;
}


std::string GameResultPrinter::reasonToString(GameResult::Reason r) {
    switch (r) {
    case GameResult::ALL_TANKS_DEAD: return "ALL_TANKS_DEAD";
    case GameResult::MAX_STEPS:      return "MAX_STEPS";
    case GameResult::ZERO_SHELLS:    return "ZERO_SHELLS";
    }
    return "UNKNOWN";
}

std::string GameResultPrinter::resultMessage(const GameResult& r, size_t max_steps) {
    //TODO: Adjust to match exact Assignment 2 wording
    std::ostringstream oss;
    size_t p1_num_of_tanks = getNumberOfTanks(r, 1);
    size_t p2_num_of_tanks = getNumberOfTanks(r, 2);
    if (r.winner == 0) {
        if (r.reason == GameResult::Reason::ALL_TANKS_DEAD) {
        oss << "Tie, both players have zero tanks";
      }
      else if (r.reason == GameResult::Reason::MAX_STEPS) {
        oss << "Tie, reached max steps = " << max_steps
            << ", player 1 has " << p1_num_of_tanks
            << " tanks, player 2 has " << p2_num_of_tanks << " tanks";
      }
      else if (r.reason == GameResult::Reason::ZERO_SHELLS) {
         oss << "Tie, both players have zero shells for " << MAX_STEPS_AFTER_SHELLS_END << " steps";
      }
    }
    else if (r.winner == 1) {
      oss << "Player " << 1 << " won with " << p1_num_of_tanks << " still alive";
    }
    else if (r.winner == 2) {
      oss << "Player " << 2 << " won with " << p2_num_of_tanks << " still alive";
    }
    return oss.str();
}

size_t GameResultPrinter::getNumberOfTanks(const GameResult& r, int player) {
//this is the relevant member in GameResult: std::vector<size_t> remaining_tanks; // index 0 = player 1, etc.
    return r.remaining_tanks[player];
}


//-----------------------Competition Mode - helper functions------------------------
