#include "common.h"
#include <OpenMS/FORMAT/IdXMLFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".idXML", [](const std::string& path) {
    std::vector<ProteinIdentification> prots;
    PeptideIdentificationList peps;
    IdXMLFile().load(path, prots, peps);
  });
}
