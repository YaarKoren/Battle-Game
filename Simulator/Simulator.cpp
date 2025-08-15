#include "Simulator.h"





int main(int argc, char* argv[]) {
  CmdArgsParser parser;
  CmdArgsParser::CmdArgs args = parser.parse(argc, argv);


  std::cout << "1..2.. test" << std::endl;


  return(EXIT_SUCCESS);

}
