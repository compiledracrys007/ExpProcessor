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
constexpr int TILE_N = 32;
constexpr int TILE_K = 32;

int main() {
  std::cout << "Starting EPU Basic Test..." << std::endl;

  std::array<int, 3> tile_config = {TILE_M, TILE_N, TILE_K};

  auto target =
      createTarget("epu", GLOBAL_MEMORY, NUMBER_OF_CORES, LOCAL_MEMORY_PER_CORE,
                   NUMBER_OF_MM_UNITS_PER_CORE, tile_config);

  // // 3. Print device info
  // std::cout << target.get_device_info() << std::endl;

  if (std::getenv("ROOT_DIR") == nullptr) {
    throw std::runtime_error(
        "Error: ROOT_DIR environment variable is not set.");
  }

  std::string filename = std::string(std::getenv("ROOT_DIR")) +
                         "/test/Target/EPU/ParalellDispatchTest/parallel.asm";

  auto parser = getTargetParser(target);
  auto operations = parser->parseFile(filename);

  // for (const auto &op : operations) {
  //   op->dump();
  // }

  auto targetSim = getTargetSimulator(target);

  // register inputs & output handles
  float inputTensorA[32][32];
  float inputTensorB[32][64];
  float outputTensorC[32][64];

  // Initialize input tensors
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      inputTensorA[i][j] = static_cast<float>((i + j) / 10.0);
      outputTensorC[i][j] = 0.0f;
    }
  }

  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 64; ++j) {
      if (j >= 32)
        inputTensorB[i][j] = 0.0;
      else
        inputTensorB[i][j] = static_cast<float>((i - j) / 10.0);
      outputTensorC[i][j] = 0.0f;
    }
  }

  // calculate expected output for verification
  float expectedOutput[32][64];
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 64; ++j) {
      expectedOutput[i][j] = 0.0f;
      for (int k = 0; k < 32; ++k) {
        expectedOutput[i][j] += inputTensorA[i][k] * inputTensorB[k][j];
      }
    }
  }

  targetSim->registerInputHandle(1, inputTensorA, sizeof(inputTensorA));
  targetSim->registerInputHandle(2, inputTensorB, sizeof(inputTensorB));
  targetSim->registerOutputHandle(3, sizeof(outputTensorC));

  targetSim->simulateInstructions(operations);

  targetSim->retrieveOutputData(3, outputTensorC, sizeof(outputTensorC));

  float testInputA[32][32];
  targetSim->retrieveInputData(1, testInputA, sizeof(testInputA));
  // std::cout << "\n\ntest Input A\n";
  // for (int i = 0; i < 32; i++) {
  //   for (int j = 0; j < 32; j++) {
  //     std::cout << testInputA[i][j] << " ";
  //   }
  //   std::cout << "\n";
  // }
  // std::cout << "\n Input A\n";
  // for (int i = 0; i < 32; i++) {
  //   for (int j = 0; j < 32; j++) {
  //     std::cout << inputTensorA[i][j] << " ";
  //   }
  //   std::cout << "\n";
  // }

  std::cout << "\n Input B\n";
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 64; j++) {
      std::cout << inputTensorB[i][j] << " ";
    }
    std::cout << "\n";
  }
  float testInputB[32][64];
  targetSim->retrieveInputData(2, testInputB, sizeof(testInputB));
  std::cout << "\n\ntest Input B\n";
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 64; j++) {
      std::cout << testInputB[i][j] << " ";
    }
    std::cout << "\n";
  }

  float localMemBChunk1Of4096[32][32];
  std::cout << "\n Local chunk B in core 2 offset 4096\n";
  targetSim->retrieveLocalMemoryData(1, 4096, localMemBChunk1Of4096,
                                     sizeof(localMemBChunk1Of4096));
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      std::cout << localMemBChunk1Of4096[i][j] << " ";
    }
    std::cout << "\n";
  }

  float localMemBCore2Of4096[32][32];
  std::cout << "\n Local chunk B in core 2 offset 4096\n";
  targetSim->retrieveLocalMemoryData(2, 4096, localMemBCore2Of4096,
                                     sizeof(localMemBCore2Of4096));
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      std::cout << localMemBCore2Of4096[i][j] << " ";
    }
    std::cout << "\n";
  }

  // std::cout << "Device output\n";
  // for (int i = 0; i < 32; i++) {
  //   for (int j = 0; j < 64; j++) {
  //     std::cout << outputTensorC[i][j] << " ";
  //   }
  //   std::cout << "\n";
  // }

  // std::cout << "Expected output\n";
  // for (int i = 0; i < 32; i++) {
  //   for (int j = 0; j < 64; j++) {
  //     std::cout << expectedOutput[i][j] << " ";
  //   }
  //   std::cout << "\n";
  // }

  // Verify output
  bool correct = true;
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 64; ++j) {
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
