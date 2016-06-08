/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MF_STORAGEMANAGER_HH_
#define MF_STORAGEMANAGER_HH_

#include <click/element.hh>
#include <click/timestamp.hh>
#include <click/vector.hh>
#include <click/deque.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>

#include <pthread.h>

#include "mf.hh"
#include "mflogger.hh"
//#include "mftransproxy.hh"

CLICK_DECLS

class Chunk; 
class MF_ChunkManager; 
class MF_TransProxy;


const uint32_t GIGABYTE_CPCT = 1073741824;
//const uint32_t TEN_MEGABYTE_CPCT = 10485760;
const uint32_t TEN_MEGABYTE_CPCT = 104857600;
const uint32_t CHK_STG_TIMEOUT_MSECOND = 1000;

class MF_StorageManager : public Element {
public:
  MF_StorageManager();
  ~MF_StorageManager();

  const char *class_name() const	{ return "MF_StorageManager"; }
  const char *port_count() const	{ return "0/0"; }
  const char *processing() const	{ return AGNOSTIC; }
 
  int configure(Vector<String> &, ErrorHandler *);
  uint8_t queryFlowBufOccupancy(Pair<uint32_t, uint32_t> &);

  // begin non-inherited functions

  /* a chk from intraLkup comes, try store it; 
    storing operation may fail due to quota reached for this src-dst pair */
  bool tryStore(Chunk &chk, uint32_t chk_id, Pair<uint32_t, uint32_t> &sd_pair, uint32_t seq, 
        uint32_t pld_size, Deque<flow_s*> &popped, MF_TransProxy &prx); 
  void store(Chunk &chk, uint32_t chk_id, uint32_t seq, uint32_t pld_size, flow_s &fs, 
        MF_TransProxy &prx);
  void reStore(Chunk &chk, Pair<uint32_t, uint32_t> &sd_pair, uint32_t seq);
  void unStore(Chunk &chk, Pair<uint32_t, uint32_t> &sd_pair, uint32_t seq,
        uint32_t pld_size);
  void unscheduleTimers(Pair<uint32_t, uint32_t> &sd_pair, uint32_t seqs[], 
        uint32_t num_chks, Vector<uint32_t> &chk_ids);
  void popOldestFlows(uint32_t, Deque<flow_s*> &);
  
  //static void handleExpiry(Timer *, void *);
  //void chkStgExpire(Chunk &chk, stg_timer_data_s *td);

private:
  const uint32_t _stg_cpct; // overall capacity for storage at this node.
  const uint32_t _flow_cpct; // per flow quota, default is 10MB 

  uint32_t _stg_bytes; // storage occupancy, should be no greater than _st_cpct

  // Data structures related to the storage
  Deque<flow_s*> _stg_q; // storage queue, each element is a src-dst pair.
  typedef HashTable<Pair<uint32_t, uint32_t>, Deque<flow_s*>::iterator> SrcDstToFlows;
  SrcDstToFlows _sd_to_flows;

  MF_ChunkManager *_chk_mgr;  
  MF_Logger _logger;
  uint32_t _chk_stg_ttl; // in seconds

  pthread_mutex_t _lock;

  class Lock {
  public:
    Lock(pthread_mutex_t &lock, MF_Logger &logger, uint32_t id) : 
      _rsc_lock(lock), _logger(logger), _id(id) { 

      pthread_mutex_lock(&_rsc_lock); 
      _logger.log(MF_DEBUG, "stg_mgr: acquired lock, id: %u", _id);
    } 
    ~Lock() { 
      pthread_mutex_unlock(&_rsc_lock); 
      _logger.log(MF_DEBUG, "stg_mgr: released lock, id: %u", _id);
    }
  private: 
    pthread_mutex_t &_rsc_lock;
    MF_Logger &_logger;
    uint32_t _id;
  };
};

CLICK_ENDDECLS

#endif /* MF_STORAGEMANAGER_HH_ */
