#include "common.h"
#include <OpenMS/FORMAT/TraMLFile.h>
#include <OpenMS/ANALYSIS/TARGETED/TargetedExperiment.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".TraML", [](const std::string& path) {
    TargetedExperiment te;
    TraMLFile().load(path, te);
  });
}
