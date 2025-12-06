#ifndef PARSER_H
#define PARSER_H

#include "ISA/Op.h"
#include "Target/EPU/Ops/EPUOps.h"

std::vector<std::unique_ptr<Op>> parseFile(const std::string &filename);

#endif // PARSER_H