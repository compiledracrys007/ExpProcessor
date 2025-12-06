#include "Parser/Parser.h"
#include "Target/EPU/Asm/EPUOps.h"

#ifndef EPU_ASM_PARSER_H
#define EPU_ASM_PARSER_H

using namespace std;

class EPUAsmParser : public Parser {
  int parseInt(const string &s);

  Dim parseDim(const string &text);

  SliceOperand parseSlice(const string &text);

  MatmulOp parseMatmul(const std::string &line);

  LocalToGlobalMemCopyOp parseLocalToGlobalMemCopy(const std::string &line);

  GlobalToLocalMemCopyOp parseGlobalToLocalMemCopy(const std::string &line);

public:
  EPUAsmParser(const Processor &proc) : Parser(proc) {}

  ~EPUAsmParser() = default;

  std::vector<std::unique_ptr<Op>>
  parseFile(const std::string &filename) override;
};

#endif // EPU_ASM_PARSER_H