#include "MySimulator.h"

MySimulator::MySimulator(CmdArgsParser::CmdArgs args)
    : args_(std::move(args)) {}

int main(int argc, char* argv[]) {
  std::cout << "1..2.. test\n";
  //CmdArgsParser parser;
  //auto args = parser.parse(argc, argv); //exits or throws on bad args

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
  return 0;

}

int MySimulator::runCompetitive() {

  return 0;
}

