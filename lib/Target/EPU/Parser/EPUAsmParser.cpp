#include "Target/EPU/Parser/EPUAsmParser.h"
#include "ISA/Op.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

enum ErrorCode { SUCCESS = 0, FILE_NOT_FOUND, PARSE_ERROR };

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

// Parse an integer safely
int EPUAsmParser::parseInt(const string &s) { return stoi(trim(s)); }

// Parse a dimension of form start:end:stride
Dim EPUAsmParser::parseDim(const string &text) {
  string t = trim(text);
  vector<string> parts;
  string cur;
  for (char c : t) {
    if (c == ':') {
      parts.push_back(cur);
      cur.clear();
    } else
      cur.push_back(c);
  }
  parts.push_back(cur);
  if (parts.size() != 3)
    throw runtime_error("Invalid dim: " + t);

  return Dim(parseInt(parts[0]), parseInt(parts[1]), parseInt(parts[2]));
}

// Parse a slice of form base[s0:e0:st0, s1:e1:st1]
SliceOperand EPUAsmParser::parseSlice(const string &text) {
  // Format: <base_offset, dim1, dim0>
  // Example: <1024, 0:32:1, 0:32:1>

  string t = trim(text);
  if (t.front() != '<' || t.back() != '>')
    throw runtime_error("Slice must be enclosed in < > : " + t);

  string inside = t.substr(1, t.size() - 2);
  vector<string> parts;
  string cur;
  int depth = 0;

  for (char c : inside) {
    if (c == ',' && depth == 0) {
      parts.push_back(trim(cur));
      cur.clear();
    } else
      cur.push_back(c);
  }
  if (!cur.empty())
    parts.push_back(trim(cur));

  if (parts.size() != 3)
    throw runtime_error("Slice requires 3 fields <base, dim1, dim0>: " +
                        inside);

  // base offset (string or int)
  auto baseOffset = parseInt(parts[0]);

  Dim d1 = parseDim(parts[1]);
  Dim d0 = parseDim(parts[2]);

  return SliceOperand(baseOffset, d1, d0);
}

GlobalToLocalMemCopyOp
EPUAsmParser::parseGlobalToLocalMemCopy(const std::string &line) {
  // remove prefix
  string rest = trim(line.substr(strlen("cp_global_to_local")));
  // we need three comma-separated top-level fields: <src>, <core>, <dst>
  vector<string> parts;
  string cur;
  int depth = 0;
  for (size_t i = 0; i < rest.size(); ++i) {
    char c = rest[i];
    if (c == '<')
      depth++;
    if (c == '>')
      depth--;
    if (c == ',' && depth == 0) {
      parts.push_back(cur);
      cur.clear();
      continue;
    }
    cur.push_back(c);
  }

  if (!cur.empty())
    parts.push_back(cur);
  if (parts.size() != 3)
    throw runtime_error("cp_global_to_local parse failed: " + rest);

  auto src = parseSlice(parts[0]);
  int core = parseInt(parts[1]);
  auto dst = parseSlice(parts[2]);

  return GlobalToLocalMemCopyOp(core, src, dst);
}

LocalToGlobalMemCopyOp
EPUAsmParser::parseLocalToGlobalMemCopy(const std::string &line) {
  // remove prefix
  string rest = trim(line.substr(strlen("cp_local_to_global")));
  // we need three comma-separated top-level fields: <core>, <src>, <dst>
  vector<string> parts;
  string cur;
  int depth = 0;
  for (size_t i = 0; i < rest.size(); ++i) {
    char c = rest[i];
    if (c == '<')
      depth++;
    if (c == '>')
      depth--;
    if (c == ',' && depth == 0) {
      parts.push_back(cur);
      cur.clear();
      continue;
    }
    cur.push_back(c);
  }

  if (!cur.empty())
    parts.push_back(cur);
  if (parts.size() != 3)
    throw runtime_error("cp_local_to_global parse failed: " + rest);

  auto core = parseInt(parts[0]);
  auto src = parseSlice(parts[1]);
  auto dst = parseSlice(parts[2]);

  return LocalToGlobalMemCopyOp(core, src, dst);
}

MatmulOp EPUAsmParser::parseMatmul(const std::string &line) {
  // remove prefix
  string rest = trim(line.substr(strlen("matmul")));
  // we need five comma-separated top-level fields:
  // <core>, <mm_unit>, <sliceA>, <sliceB>, <sliceC>, <accumulate = True/False>
  vector<string> parts;
  string cur;
  int depth = 0;
  for (size_t i = 0; i < rest.size(); ++i) {
    char c = rest[i];
    if (c == '<')
      depth++;
    if (c == '>')
      depth--;
    if (c == ',' && depth == 0) {
      parts.push_back(cur);
      cur.clear();
      continue;
    }
    cur.push_back(c);
  }

  if (!cur.empty())
    parts.push_back(cur);
  if (parts.size() != 6)
    throw runtime_error("matmul parse failed: " + rest);

  auto core = parseInt(parts[0]);
  auto mmUnit = parseInt(parts[1]);
  auto sliceA = parseSlice(parts[2]);
  auto sliceB = parseSlice(parts[3]);
  auto sliceC = parseSlice(parts[4]);

  auto acc_pos = parts[5].find("accumulator=");
  if (acc_pos == string::npos)
    throw runtime_error("matmul missing accumulator spec");
  string head = trim(parts[5].substr(0, acc_pos));
  string acc_str = trim(parts[5].substr(acc_pos + 12)); // after "accumulator="
  bool acc = false;
  if (acc_str == "True" || acc_str == "true")
    acc = true;
  else if (acc_str == "False" || acc_str == "false")
    acc = false;
  else
    throw runtime_error("Invalid accumulator: " + acc_str);

  return MatmulOp(core, mmUnit, sliceA, sliceB, sliceC, BoolOperand(acc));
}

std::vector<std::unique_ptr<Op>>
EPUAsmParser::parseFile(const std::string &filename) {
  std::vector<std::unique_ptr<Op>> parsedOps;

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
      auto instr = parseGlobalToLocalMemCopy(s);
      parsedOps.push_back(std::make_unique<GlobalToLocalMemCopyOp>(instr));
    } else if (starts_with(s, "cp_local_to_global")) {
      auto instr = parseLocalToGlobalMemCopy(s);
      parsedOps.push_back(std::make_unique<LocalToGlobalMemCopyOp>(instr));
    } else if (starts_with(s, "matmul")) {
      auto instr = parseMatmul(s);
      parsedOps.push_back(std::make_unique<MatmulOp>(instr));
    } else {
      exit(ErrorCode::PARSE_ERROR);
    }
  }

  return parsedOps;
}
