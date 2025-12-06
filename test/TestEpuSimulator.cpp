// This test simulates a simple EPU hypothetical processor executing a matrix
// multiplication operation defined in an assembly file. It sets up the
// processor, loads the instructions, registers input and output tensors, runs
// the simulation, and verifies the output against expected results.

#include "Parser/Parser.h"
#include "Simulator/Simulator.h"
#include "Target/EPU/Simulator/EPUSimulator.h"
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
constexpr int TILE_N = 32;
constexpr int TILE_K = 32;

int main() {
  std::array<int, 3> tile_config = {TILE_M, TILE_N, TILE_K};

  Processor target = create_target("epu", GLOBAL_MEMORY, NUMBER_OF_CORES,
                                   LOCAL_MEMORY_PER_CORE,
                                   NUMBER_OF_MM_UNITS_PER_CORE, tile_config);

  // // 3. Print device info
  // std::cout << target.get_device_info() << std::endl;

  if (std::getenv("ROOT_DIR") == nullptr) {
    throw std::runtime_error(
        "Error: ROOT_DIR environment variable is not set.");
  }

  std::string filename =
      std::string(std::getenv("ROOT_DIR")) + "/test/test_epu.asm";

  auto operations = parseFile(filename);

  // for (const auto &op : operations) {
  //   op->dump();
  // }

  auto targetSim = EPUSimulator(target);

  // register inputs & output handles
  float inputTensorA[32][32];
  float inputTensorB[32][32];
  float outputTensorC[32][32];

  // Initialize input tensors
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      inputTensorA[i][j] = static_cast<float>((i + j) / 10.0);
      inputTensorB[i][j] = static_cast<float>((i - j) / 10.0);
      outputTensorC[i][j] = 0.0f;
    }
  }

  // calculate expected output for verification
  float expectedOutput[32][32];
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      expectedOutput[i][j] = 0.0f;
      for (int k = 0; k < 32; ++k) {
        expectedOutput[i][j] += inputTensorA[i][k] * inputTensorB[k][j];
      }
    }
  }

  targetSim.registerInputHandle(1, inputTensorA, sizeof(inputTensorA));
  targetSim.registerInputHandle(2, inputTensorB, sizeof(inputTensorB));
  targetSim.registerOutputHandle(3, sizeof(outputTensorC));

  targetSim.simulateInstructions(operations);

  targetSim.retrieveOutputData(3, outputTensorC, sizeof(outputTensorC));

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
