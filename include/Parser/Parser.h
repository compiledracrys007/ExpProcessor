#ifndef PARSER_H
#define PARSER_H

#include "ISA/Op.h"

std::vector<Op> parseFile(const std::string &filename);

#endif // PARSER_H