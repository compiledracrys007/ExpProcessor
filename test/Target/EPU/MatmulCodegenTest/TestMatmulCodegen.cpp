// This test simulates a simple EPU hypothetical processor executing a matrix
// multiplication operation defined in an assembly file. It sets up the
// processor, loads the instructions, registers input and output tensors, runs
// the simulation, and verifies the output against expected results.

#include "Target/EPU/CodeGen/EPUCodeGen.h"
#include "Utils/Utils.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

bool testMatmul(std::string filename, int M, int K, int N) {
  auto target = createEPUTarget();
  auto parser = getTargetParser(target);
  auto operations = parser->parseFile(filename);

  auto targetSim = getTargetSimulator(target);

  // register inputs & output handles
  float inputTensorA[M][K];
  float inputTensorB[K][N];
  float outputTensorC[M][N];

  // Initialize input tensors
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < K; ++j) {
      inputTensorA[i][j] = static_cast<float>((i + j) / 10.0);
    }
  }

  for (int i = 0; i < K; ++i) {
    for (int j = 0; j < N; ++j) {
      inputTensorB[i][j] = static_cast<float>((i - j) / 10.0);
    }
  }

  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      outputTensorC[i][j] = 0.0f;
    }
  }

  // calculate expected output for verification
  float expectedOutput[M][N];
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      expectedOutput[i][j] = 0.0f;
      for (int k = 0; k < K; ++k) {
        expectedOutput[i][j] += inputTensorA[i][k] * inputTensorB[k][j];
      }
    }
  }

  targetSim->registerInputHandle(1, inputTensorA, sizeof(inputTensorA), {M, K});
  targetSim->registerInputHandle(2, inputTensorB, sizeof(inputTensorB), {K, N});
  targetSim->registerOutputHandle(3, sizeof(outputTensorC), {M, N});

  targetSim->simulateInstructions(operations);

  targetSim->retrieveOutputData(3, outputTensorC, sizeof(outputTensorC));

  // Verify output
  bool correct = true;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      if (std::abs(outputTensorC[i][j] - expectedOutput[i][j]) > 1e-5) {
        std::cout << "Mismatch at (" << i << ", " << j << "): expected "
                  << expectedOutput[i][j] << ", got " << outputTensorC[i][j]
                  << std::endl;
        correct = false;
        break;
      }
    }
    if (!correct)
      break;
  }

  if (correct) {
    return true;
  }
  return false;
}

int main() {
  std::cout << "\nStarting EPU Codegen Test..." << std::endl;

  // std::string filename = std::string(std::getenv("ROOT_DIR")) +
  //                        "/test/Target/EPU/MultiCoreTest/multicore.asm";

  // // // 3. Print device info
  // // std::cout << target.get_device_info() << std::endl;

  if (std::getenv("ROOT_DIR") == nullptr) {
    throw std::runtime_error(
        "Error: ROOT_DIR environment variable is not set.");
  }

  std::vector<std::vector<int>> tests = {
      {32, 32, 32}, {32, 32, 64}, {32, 32, 128}};

  for (auto test : tests) {
    // Parse arguments
    int M = test[0];
    int K = test[1];
    int N = test[2];

    std::cout << "Testing codegen + simulation for " << M << ", " << K << ", "
              << N << "\n";

    auto asmStr = generateMatmulISAForEPU(M, N, K);

    char file[] = "/tmp/mytmpfileXXXXXX";
    int fd = mkstemp(file);

    std::ofstream ofs(file);
    ofs << asmStr;
    ofs.close();
    close(fd);

    if (!testMatmul(std::string(file), M, K, N)) {
      std::__throw_runtime_error("Test failed\n");
    } else {
      std::cout << "Test passed!\n";
    }
  }

  return 0;
}
