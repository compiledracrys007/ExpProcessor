// This test simulates a simple EPU hypothetical processor executing a matrix
// multiplication operation defined in an assembly file. It sets up the
// processor, loads the instructions, registers input and output tensors, runs
// the simulation, and verifies the output against expected results.

#include "Utils/Utils.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

constexpr size_t GLOBAL_MEMORY = 1024ULL * 1024 * 1024; // 1 GB
constexpr int NUMBER_OF_CORES = 4;
constexpr size_t LOCAL_MEMORY_PER_CORE = 512 * 1024; // 512 KB
constexpr int NUMBER_OF_MM_UNITS_PER_CORE = 4;
constexpr int TILE_M = 32;
constexpr int TILE_N = 64;
constexpr int TILE_K = 32;

int main() {
  std::cout << "Starting EPU Acc Test..." << std::endl;

  std::array<int, 3> tile_config = {TILE_M, TILE_N, TILE_K};

  auto target =
      createTarget("epu", GLOBAL_MEMORY, NUMBER_OF_CORES, LOCAL_MEMORY_PER_CORE,
                   NUMBER_OF_MM_UNITS_PER_CORE, tile_config);

  if (std::getenv("ROOT_DIR") == nullptr) {
    throw std::runtime_error(
        "Error: ROOT_DIR environment variable is not set.");
  }

  std::string filename = std::string(std::getenv("ROOT_DIR")) +
                         "/test/Target/EPU/AccTest/acc.asm";

  auto parser = getTargetParser(target);
  auto operations = parser->parseFile(filename);

  auto targetSim = getTargetSimulator(target);

  // register inputs & output handles
  float inputTensorA[32][64];
  float inputTensorB[64][32];
  float outputTensorC[32][32];

  // Initialize input tensors
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 64; ++j) {
      inputTensorA[i][j] = static_cast<float>((i + j) / 10.0);
    }
  }
  for (int i = 0; i < 64; ++i) {
    for (int j = 0; j < 32; ++j) {
      inputTensorB[i][j] = static_cast<float>((i - j) / 10.0);
    }
  }
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      outputTensorC[i][j] = 0.0f;
    }
  }

  // calculate expected output for verification
  float expectedOutput[32][32];
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      expectedOutput[i][j] = 0.0f;
      for (int k = 0; k < 64; ++k) {
        expectedOutput[i][j] += inputTensorA[i][k] * inputTensorB[k][j];
      }
    }
  }

  targetSim->registerInputHandle(1, inputTensorA, sizeof(inputTensorA),
                                 {32, 64});
  targetSim->registerInputHandle(2, inputTensorB, sizeof(inputTensorB),
                                 {64, 32});
  targetSim->registerOutputHandle(3, sizeof(outputTensorC), {32, 32});

  targetSim->simulateInstructions(operations);

  targetSim->retrieveOutputData(3, outputTensorC, sizeof(outputTensorC));

  // Verify output
  bool correct = true;
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
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
    std::cout << "Output verified successfully" << std::endl;
  } else {
    throw std::runtime_error("Error: Output verification failed");
  }

  return 0;
}
