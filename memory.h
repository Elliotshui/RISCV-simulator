#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"
#define MaxMemory 100000000

class Memory: public Storage {
 public:
  Memory() {}
  ~Memory() {}

  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);
  void AssignMemory(unsigned char* p) {
    mpointer = p;
  }

 private:
  // Memory implement
    unsigned char *mpointer;

  DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 