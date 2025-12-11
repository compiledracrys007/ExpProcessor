#include "Target/EPU/CodeGen/EPUCodeGen.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <M> <K> <N>\n";
    return 1;
  }

  // Parse arguments
  int M = std::atoi(argv[1]);
  int K = std::atoi(argv[2]);
  int N = std::atoi(argv[3]);

  auto asmStr = generateMatmulISAForEPU(M, N, K);

  std::cout << "Asm generated\n\n" << asmStr << "\n";

  // Here you can extend to actual matmul or other logic.
  return 0;
}