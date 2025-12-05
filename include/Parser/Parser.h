#ifndef PARSER_H
#define PARSER_H

#include "ISA/Op.h"
#include "ISA/ISAOps.h"

std::vector<std::unique_ptr<Op>> parseFile(const std::string &filename);

#endif // PARSER_H