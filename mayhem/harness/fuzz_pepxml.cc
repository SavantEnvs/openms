#include "common.h"
#include <OpenMS/FORMAT/PepXMLFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".pepXML", [](const std::string& path) {
    std::vector<ProteinIdentification> prots;
    PeptideIdentificationList peps;
    PepXMLFile().load(path, prots, peps);
  });
}
