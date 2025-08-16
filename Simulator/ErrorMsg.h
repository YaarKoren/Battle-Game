#pragma once

#include <iostream>
#include <string>

//errors and usage printing//
class ErrorMsg {
  public:

    //usage printing
    static void usage_msg();

    static void error_and_usage(const std::string& msg);
};