#include "Simulator.h"

Simulator::Simulator(CmdArgsParser::CmdArgs args)
    : args_(std::move(args)) {}

int main(int argc, char* argv[]) {
  CmdArgsParser parser;
  auto args = parser.parse(argc, argv); //exits or throws on bad args

  Simulator sim(std::move(args));
  int rc = sim.run();

  std::cout << "1..2.. test\n";
  return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}



int Simulator::run() {
  if (args_.mode_ == Mode::Comparative) {
    Simulator::runComparative();
  }

  if (args_.mode_ == Mode::Competitive) {
    Simulator::runCompetitive();
  }
}



int Simulator::runComparative() {
  return 0;

}

int Simulator::runCompetitive() {

  return 0;
}

