#include "common.h"
#include <OpenMS/FORMAT/ConsensusXMLFile.h>
#include <OpenMS/KERNEL/ConsensusMap.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".consensusXML", [](const std::string& path) {
    ConsensusMap cm;
    ConsensusXMLFile().load(path, cm);
  });
}
