
#include "memory.h"
#include "string.h"

void Memory::HandleRequest(uint64_t addr, int bytes, int read, char *content, int &hit, int &time) {
  if (addr < MaxMemory) {
	  if (read == 1)
		  memcpy(content, mpointer + addr, bytes);
	  else
		  memcpy(mpointer + addr, content, bytes);
  }
  else {
	  if(read == 1)
		  memcpy(content, mpointer, bytes);
  }
  hit = 1;
  time = latency_.hit_latency + latency_.bus_latency;
  stats_.access_time += time;
}
