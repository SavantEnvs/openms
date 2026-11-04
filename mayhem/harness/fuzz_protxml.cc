#include "common.h"
#include <OpenMS/FORMAT/ProtXMLFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".protXML", [](const std::string& path) {
    ProteinIdentification prot;
    PeptideIdentification pep;
    ProtXMLFile().load(path, prot, pep);
  });
}
