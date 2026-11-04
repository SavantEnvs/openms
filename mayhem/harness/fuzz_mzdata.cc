#include "common.h"
#include <OpenMS/FORMAT/MzDataFile.h>
#include <OpenMS/KERNEL/MSExperiment.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".mzData", [](const std::string& path) {
    PeakMap map;
    MzDataFile().load(path, map);
  });
}
