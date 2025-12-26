// This test simulates a simple EPU hypothetical processor executing a matrix
// multiplication operation defined in an assembly file. It sets up the
// processor, loads the instructions, registers input and output tensors, runs
// the simulation, and verifies the output against expected results.

#include "Utils/Utils.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
  std::cout << "\nStarting EPU Parallel Dispatch Test..." << std::endl;

  auto target = createEPUTarget();

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

  targetSim->registerInputHandle(1, inputTensorA, sizeof(inputTensorA),
                                 {32, 32});
  targetSim->registerInputHandle(2, inputTensorB, sizeof(inputTensorB),
                                 {32, 64});
  targetSim->registerOutputHandle(3, sizeof(outputTensorC), {32, 64});

  targetSim->simulateInstructions(operations);

  targetSim->retrieveOutputData(3, outputTensorC, sizeof(outputTensorC));

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
