#include "common.h"
#include <OpenMS/FORMAT/MascotGenericFile.h>
#include <OpenMS/KERNEL/MSExperiment.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".mgf", [](const std::string& path) {
    PeakMap map;
    MascotGenericFile().load(path, map);
  });
}
