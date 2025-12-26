#include "Simulator/Simulator.h"
#include "ISA/Op.h"
#include <assert.h>
#include <cstring>
#include <exception>
#include <iostream>

void Simulator::registerInputHandle(int handleId, const void *rawData,
                                    size_t numBytes, std::vector<int> shape) {
  if (nextFreeGlobalMemoryOffset + numBytes > globalMemorySize)
    throw std::runtime_error("No space in global memory");

  assert(shape.size() == 2 && "Supports only 2d input/output type for now");

  // Copy input bytes into memory
  std::memcpy(memory + nextFreeGlobalMemoryOffset, rawData, numBytes);
  inputHandleToMemoryLocMap[handleId] = nextFreeGlobalMemoryOffset;
  inputHandleToShapeMap[handleId] = shape;

  nextFreeGlobalMemoryOffset += numBytes;
}

void Simulator::registerOutputHandle(int handleId, size_t numBytes,
                                     std::vector<int> shape) {
  if (nextFreeGlobalMemoryOffset + numBytes > globalMemorySize)
    throw std::runtime_error("No space in global memory");

  assert(shape.size() == 2 && "Supports only 2d input/output type for now");

  outputHandleToMemoryLocMap[handleId] = nextFreeGlobalMemoryOffset;
  outputHandleToShapeMap[handleId] = shape;

  nextFreeGlobalMemoryOffset += numBytes;
}

void Simulator::retrieveLocalMemoryData(int coreNum, int offset,
                                        void *outputBuffer, size_t numBytes) {

  int memOffset = (globalMemorySize + (coreNum * localMemoryPerCore)) + offset;

  std::memcpy(outputBuffer, memory + memOffset, numBytes);
}

void Simulator::retrieveInputData(int handleId, void *outputBuffer,
                                  size_t numBytes) {
  if (inputHandleToMemoryLocMap.count(handleId) == 0) {
    throw std::runtime_error("Unknown output handle ID");
  }

  int memOffset = inputHandleToMemoryLocMap[handleId];
  std::memcpy(outputBuffer, memory + memOffset, numBytes);
}

void Simulator::retrieveOutputData(int handleId, void *outputBuffer,
                                   size_t numBytes) {
  if (outputHandleToMemoryLocMap.count(handleId) == 0) {
    throw std::runtime_error("Unknown output handle ID");
  }

  int memOffset = outputHandleToMemoryLocMap[handleId];
  std::memcpy(outputBuffer, memory + memOffset, numBytes);
}