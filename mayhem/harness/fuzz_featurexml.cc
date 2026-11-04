#include "common.h"
#include <OpenMS/FORMAT/FeatureXMLFile.h>
#include <OpenMS/KERNEL/FeatureMap.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".featureXML", [](const std::string& path) {
    FeatureMap fm;
    FeatureXMLFile().load(path, fm);
  });
}
