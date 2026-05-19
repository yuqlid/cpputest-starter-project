#include "CppUTest/CommandLineTestRunner.h"
#include "TestOutputPath.hpp"

int main(int ac, char** av) {
  TestOutputPath::initialize();
  return CommandLineTestRunner::RunAllTests(ac, av);
}
