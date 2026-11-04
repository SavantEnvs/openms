#include "common.h"
#include <OpenMS/FORMAT/MzXMLFile.h>
#include <OpenMS/KERNEL/MSExperiment.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".mzXML", [](const std::string& path) {
    PeakMap map;
    MzXMLFile().load(path, map);
  });
}
