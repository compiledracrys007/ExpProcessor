#ifndef PARSER_H
#define PARSER_H

#include "ISA/Op.h"
#include "Processor/Processor.h"

class Parser {
protected:
  Processor processor;

public:
  Parser(const Processor &proc) : processor(proc) {}

  virtual std::vector<std::unique_ptr<Op>>
  parseFile(const std::string &filename) = 0;

  virtual ~Parser() = default;
};

#endif // PARSER_H