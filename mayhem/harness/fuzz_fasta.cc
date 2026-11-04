#include "common.h"
#include <OpenMS/FORMAT/FASTAFile.h>
using namespace OpenMS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  return with_input_file(data, size, ".fasta", [](const std::string& path) {
    std::vector<FASTAFile::FASTAEntry> entries;
    FASTAFile().load(path, entries);
  });
}
