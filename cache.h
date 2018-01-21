#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"

struct CacheStruct_ {
  char valid;
  char dirty;
  int lru;
  uint64_t tag;
  char data[256];
};

typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  CacheConfig_(int size_, int ass_, int set_, int wth_, int alloc_) {
	  size = size_;
	  associativity = ass_;
	  set_num = set_;
	  write_through = wth_;
	  write_allocate = alloc_;
  }
  CacheConfig_() {}
} CacheConfig;

class Cache: public Storage {
 public:
  Cache() {}
  ~Cache() {}

  // Sets & Gets
  void SetConfig(CacheConfig cc, CacheStruct_* p) {
    config_ = cc;
    cpointer = p;
  }
  CacheConfig GetConfig() {
    return config_;
  }
  void SetLower(Storage *ll) { lower_ = ll; }
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision();
  void ReplaceAlgorithm();
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm();

  CacheConfig config_;
  Storage *lower_;
  CacheStruct_* cpointer;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 