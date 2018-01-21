
#include "cache.h"
#include "def.h"
#include "math.h"
#include "string.h"
uint64_t getbit(uint64_t a, int l, int w) {
	uint64_t b = a >> l;
	uint64_t c = (1ull << w) - 1;
	return b & c;
}
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {

  hit = 0;
  time = 0;
  time += latency_.bus_latency + latency_.hit_latency;
  stats_.access_time += time;
  stats_.access_counter += 1;
  int sz = config_.size,
      as = config_.associativity,
      sn = config_.set_num,
      wth = config_.write_through,
      wal = config_.write_allocate; 
  int bwidth = (int)log2(sz),
      swidth = (int)log2(sn), 
      twidth = 64 - bwidth - swidth;
  uint64_t set = getbit(addr, bwidth, swidth),
           off = getbit(addr, 0, bwidth),
           tag = getbit(addr, bwidth + swidth, twidth);
  int hiti = 0, placei = 0;
  //Bypass?
  int isfull = 1;
  if (!BypassDecision()) {  
    //Miss? Need replace?
    char valid_;
    uint64_t tag_;
    for(int i = 0; i < as; ++i) {
      valid_ = cpointer[set * as + i].valid;
      tag_ = cpointer[set * as + i].tag;
      if(valid_ == 1 && tag_ == tag) {
        hit = 1;
        hiti = i;
        cpointer[set * as + i].lru = 0;
      }
      else if(valid_ == 1 && tag_ !=  tag) {
        cpointer[set * as + i].lru += 1;
      }
    }
    //If not hit, choose a position to place
    if(hit == 0) {
	  stats_.miss_num++;
	  int maxlru = -1;
      for(int i = 0; i < as; ++i) {
        //Find maxlru line
        if(cpointer[set * as + i].valid == 1) {
          if(cpointer[set * as + i].lru > maxlru) {
            maxlru = cpointer[set * as + i].lru;
            placei = i;
          }
        }
        //If empty line exist, then no replace
        else {
          placei = i;
		  isfull = 0;
          break;
        }
      }
	  if (isfull == 1) {
		  stats_.replace_num += 1;
	  }
    }
    else {
      if(read == 1) {
        memcpy(content, cpointer[set * as + hiti].data + off, bytes);
        return;
      }
      else {
        memcpy(cpointer[set * as + hiti].data + off, content, bytes);
        if(wth == 1) {
          int lower_hit, lower_time;
          lower_->HandleRequest(addr, bytes, 0, content, lower_hit, lower_time);
          time += latency_.bus_latency + lower_time;
          stats_.access_time += latency_.bus_latency; 
        }
        else {
          cpointer[set * as + hiti].dirty = 1;
        }
        return;
      }
    }
  }

  if(PrefetchDecision()) {
    PrefetchAlgorithm();
  }
  else {
    if(read == 1) {
      //Fetch block from lower layer 
      int lower_hit, lower_time;
      lower_->HandleRequest(addr - off, sz, read, content, lower_hit, lower_time);
      time += latency_.bus_latency + lower_time;
      stats_.access_time += latency_.bus_latency;

      //Deal with dirty line
      CacheStruct_& l = cpointer[set * as + placei];
      if(l.dirty == 1 && l.valid == 1) {
		uint64_t raddr;
		raddr = (l.tag << (bwidth + swidth)) + (set << bwidth);
        lower_->HandleRequest(raddr, sz, 0, l.data, lower_hit, lower_time);
        time += latency_.bus_latency + lower_time;
        stats_.access_time += latency_.bus_latency;
      }

      //Place the block in cache
      l.valid = 1;
      l.lru = 0;
      l.dirty = 0;
	  l.tag = tag;
      memcpy(l.data, content, sz);

      //update content
      memcpy(content, l.data + off, bytes);
      return;
    }
    else {
      int lower_hit, lower_time;
      if(wal == 0) {
        lower_->HandleRequest(addr, bytes, 0, content, lower_hit, lower_time);
		time += latency_.bus_latency + lower_time;
		stats_.access_time += latency_.bus_latency;
      }
      else {
        char temp[256];
        lower_->HandleRequest(addr - off, sz, 1, temp, lower_hit, lower_time);
		time += latency_.bus_latency + lower_time;
		stats_.access_time += latency_.bus_latency;

        CacheStruct_& l = cpointer[set * as + placei];
		uint64_t raddr;
		raddr = (l.tag << (bwidth + swidth)) + (set << bwidth);
        if(l.dirty == 1 && l.valid == 1) {
          lower_->HandleRequest(addr - off, sz, 0, l.data, lower_hit, lower_time);
          time += latency_.bus_latency + lower_time;
          stats_.access_time += latency_.bus_latency;
        }

        l.valid = 1;
        l.lru = 0;
        l.dirty = 1;
		l.tag = tag;
        memcpy(l.data, temp, sz);
        memcpy(l.data + off, content, bytes);
      }
    }
  }
    /*
    if (ReplaceDecision()) {
      // Choose victim
      ReplaceAlgorithm();
    } else {
      // return hit & time
      hit = 1;
      time += latency_.bus_latency + latency_.hit_latency;
      stats_.access_time += time;
      return;
    }
    */

  // Prefetch?
  /*
  if (PrefetchDecision()) {
    PrefetchAlgorithm();
  } else {
    // Fetch from lower layer
    int lower_hit, lower_time;
    lower_->HandleRequest(addr, bytes, read, content,
                          lower_hit, lower_time);
    hit = 0;
    time += latency_.bus_latency + lower_time;
    stats_.access_time += latency_.bus_latency;
  }
  */
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm() {
}

int Cache::ReplaceDecision() {
  return TRUE;
  //return FALSE;
}

void Cache::ReplaceAlgorithm(){
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}
