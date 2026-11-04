#include "common.h"
#include <OpenMS/FORMAT/MzIdentMLFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".mzid", [](const std::string& path) {
    std::vector<ProteinIdentification> prots;
    PeptideIdentificationList peps;
    MzIdentMLFile().load(path, prots, peps);
  });
}
