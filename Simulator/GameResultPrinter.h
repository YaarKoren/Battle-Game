#pragma once

#include <vector>

#include "Utils.h"



class GameResulePrinter {
  public:
    GameResulePrinter() = default;

    virtual ~GameResulePrinter() = default;

    static void printComparativeResults(std::vector<GMNameAndResult> results, std::string folder_path);

    static void printCompetitionResults();

  private:


};


