#pragma once
// Shared driver: OpenMS parsers read from a path, so spill the fuzz input to a temp file.
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <unistd.h>

template <typename Fn>
static int with_input_file(const uint8_t* data, size_t size, const char* suffix, Fn&& fn)
{
  const char* tmp = getenv("TMPDIR");
  std::string path = std::string(tmp && *tmp ? tmp : "/tmp") + "/oms-fuzz-XXXXXX" + suffix;
  int fd = mkstemps(&path[0], (int)strlen(suffix));
  if (fd < 0) return -1;
  size_t off = 0;
  while (off < size)
  {
    ssize_t w = write(fd, data + off, size - off);
    if (w <= 0) break;
    off += (size_t)w;
  }
  close(fd);
  int rc = 0;
  try
  {
    fn(path);
  }
  catch (const std::exception&)
  {
    rc = -1;
  }
  unlink(path.c_str());
  return rc;
}
