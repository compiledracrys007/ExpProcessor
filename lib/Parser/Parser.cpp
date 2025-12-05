#include "Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

enum ErrorCode {
  SUCCESS = 0,
  FILE_NOT_FOUND,
  PARSE_ERROR
};

// Parsers for each instruction using simple tokenization and regex where
// helpful
static bool starts_with(const string &s, const string &pref) {
  return s.rfind(pref, 0) == 0;
}

// Utility: trim
static inline string trim(const string &s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == string::npos)
    return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}


Op parse(const std::string &s) {
    return Op(OpCode::MATMUL, 1);
}

std::vector<Op> parseFile(const std::string &filename) {
  std::vector<Op> parsedOps;

  std::ifstream fin(filename);
  if (!fin.is_open()) {
    exit(ErrorCode::FILE_NOT_FOUND);
  }

  std::string line;
  while (true) {
    if (!std::getline(fin, line))
      break;
    std::string s = trim(line);
    if (s.empty())
      continue;

    if (starts_with(s, "cp_global_to_local")) {
      auto instr = parse(s);
      parsedOps.push_back(instr);
    } else if (starts_with(s, "cp_local_to_global")) {
      auto instr = parse(s);
      parsedOps.push_back(instr);
    } else if (starts_with(s, "matmul")) {
      auto instr = parse(s);
      parsedOps.push_back(instr);
    } else {
      exit(ErrorCode::PARSE_ERROR);
    }
  }

  return parsedOps;
}