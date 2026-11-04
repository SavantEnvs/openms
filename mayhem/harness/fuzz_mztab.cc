#include "common.h"
#include <OpenMS/FORMAT/MzTabFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".mzTab", [](const std::string& path) {
    MzTab tab;
    MzTabFile().load(path, tab);
  });
}
