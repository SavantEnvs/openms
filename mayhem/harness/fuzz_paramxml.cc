#include "common.h"
#include <OpenMS/FORMAT/ParamXMLFile.h>
#include <OpenMS/DATASTRUCTURES/Param.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".ini", [](const std::string& path) {
    Param p;
    ParamXMLFile().load(path, p);
  });
}
