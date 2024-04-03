#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect* as, const char* filename) {
  void* dest = DEFAULT_ENTRY;
  size_t len = get_ramdisk_size();
  ramdisk_read(dest, 0, len);
  return (uintptr_t)DEFAULT_ENTRY;
}

