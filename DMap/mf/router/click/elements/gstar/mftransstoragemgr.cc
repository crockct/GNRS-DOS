#include <string>

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "mftransstoragemgr.hh"
#include "mftransproxy.hh"
#include "mfsettings.hh"

CLICK_DECLS

MF_StorageManager::MF_StorageManager() : _stg_cpct(GIGABYTE_CPCT), _stg_bytes(0),
    _flow_cpct(TEN_MEGABYTE_CPCT), _logger(MF_Logger::init()),
    _chk_stg_ttl(CHK_STG_TIMEOUT_MSECOND) {
  pthread_mutex_init(&_lock, NULL);

  std::string key("CHK_STG_TIMEOUT_MSECOND");
  std::string value = MF_Settings::instance()->lookup(key);

  if (!value.empty()) {
    _chk_stg_ttl = static_cast<uint32_t>(atoll(value.c_str()));
    _logger.log(MF_DEBUG, "StorageManager::ctor, settings lookup key: %s, get value in int: %u",
                key.c_str(), _chk_stg_ttl);
  }
}

MF_StorageManager::~MF_StorageManager() {
}

int MF_StorageManager::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (cp_va_kparse(conf, this, errh,
      "CHUNK_MANAGER", cpkP+cpkM, cpElement, &_chk_mgr,
      "STORAGE_CAPACITY", cpkP, cpUnsigned, &_stg_cpct,
    cpEnd) < 0)
    return -1;
  return 0;
}

// return value is a score with range of [0, 100]
uint8_t MF_StorageManager::queryFlowBufOccupancy(Pair<uint32_t, uint32_t> &sd_pair) {
  _logger.log(MF_DEBUG, "StorageManager::queryFlowBufOccupancy() for src: %u, dst: %u, pair", sd_pair.first, sd_pair.second);
  uint8_t ret = 100;

  if (_sd_to_flows.find(sd_pair) != _sd_to_flows.end()) {
    ret = static_cast<double>((*(_sd_to_flows[sd_pair]))->size)/_flow_cpct * 100;
  } else {
    ret = 0;
  }

  return ret;
}

// here, "store" means removing this chunk from chunkManager, insert in storeManager's buffer.
bool MF_StorageManager::tryStore(Chunk &chk, uint32_t chk_id, Pair<uint32_t, uint32_t> &sd_pair, 
    uint32_t seq, uint32_t pld_size, Deque<flow_s*> &popped, MF_TransProxy &prx) {
  _logger.log(MF_DEBUG, "StorageManager::tryStore() for src: %u, dst: %u, pair", sd_pair.first, sd_pair.second);
  
  //Lock lk(_lock); // resource lock
  
  bool stored = true;
  if (_sd_to_flows.find(sd_pair) != _sd_to_flows.end()) {
    _logger.log(MF_DEBUG, "StorageManager::tryStore() src: %u, dst: %u, pair found in sd_to_flows", sd_pair.first, sd_pair.second);
    flow_s *fs = *(_sd_to_flows[sd_pair]);
    uint32_t flow_size = fs->size;
    if (flow_size + pld_size <= _flow_cpct) {  
      popOldestFlows(pld_size, popped);
      store(chk, chk_id, seq, pld_size, *fs, prx);
    } else { 
      stored = false;
    }
  } else { 
    _logger.log(MF_DEBUG, "StorageManager::tryStore() src: %u, dst: %u, pair not found in sd_to_flows, inserting",
                 sd_pair.first, sd_pair.second);
    flow_s *fs = new flow_s(sd_pair.first, sd_pair.second, pld_size);
    //fs->chks.push_back(&chk);
    //fs->seq_to_chks[seq] = &chk;

    _stg_q.push_front(fs);
    _sd_to_flows.set(sd_pair, _stg_q.begin());

    popOldestFlows(pld_size, popped);
    store(chk, chk_id, seq, pld_size, *fs, prx);
  }
  return stored;
}

void MF_StorageManager::unscheduleTimers(Pair<uint32_t, uint32_t> &sd_pair, 
      uint32_t seqs[], uint32_t num_chks, Vector<uint32_t> &chk_ids) {
  _logger.log(MF_DEBUG, "StorageManager::unscheduleTimers() for src: %u, dst: %u, pair", sd_pair.first, sd_pair.second);

  //Lock lk(_lock);
  flow_s *fs = *(_sd_to_flows[sd_pair]);
  std::map<uint32_t, Timer*> &seq_to_timers = fs->seq_to_timers;
  std::map<uint32_t, Chunk*> &seq_to_chks = fs->seq_to_chks;
  for (uint32_t i=0; i<num_chks; ++i) {
    if (seq_to_timers.find(seqs[i]) == seq_to_timers.end()) {
      _logger.log(MF_ERROR, "stg mgr: this is not a stored chunk\n");
    } else {
      Timer *tm = seq_to_timers[seqs[i]];
      tm->unschedule();
      if (seq_to_chks.find(seqs[i]) == seq_to_chks.end()) {
        _logger.log(MF_FATAL, "stg mgr: data structures not matching, aborting\n");
        exit(-1);
      }
      Chunk *chk = seq_to_chks[seqs[i]];
      chk_ids.push_back(chk->getChunkID());
    }
  }
}

void MF_StorageManager::popOldestFlows(uint32_t pld_size, Deque<flow_s*> &popped) {
  _logger.log(MF_DEBUG, "StorageManager::popOldestFlows()"); 
  // meaning storage capacity is reached, need to pop oldest src-dst chunks.
  while (_stg_bytes + pld_size > _stg_cpct) { 
      if (_stg_q.empty()) {
        _logger.log(MF_ERROR, "StorageManager::popOldestFlows() q empty but size is not 0, exiting");
        exit(-1);
      }

      flow_s *fs = _stg_q.back();
      _logger.log(MF_DEBUG, "StorageManager::popOldestFlows() poped flow for src: %u, dst: %u",
                  fs->src, fs->dst);
      _stg_q.pop_back();
      popped.push_back(fs);
      Pair<uint32_t, uint32_t> sd(fs->src, fs->dst);
      _sd_to_flows.erase(sd);
      
      _stg_bytes -= fs->size;
  }
}

void MF_StorageManager::store(Chunk &chk, uint32_t chk_id, uint32_t seq, uint32_t pld_size, 
      flow_s &fs, MF_TransProxy &prx) {
  _logger.log(MF_DEBUG, "StorageManager::store() for seq %u", seq);
  // remove from chunkManager's, add to storageManager's
  //_chk_mgr->popChunk(chk);
  //fs.chks.push_back(&chk); 
  fs.seq_to_chks[seq] = &chk;
  
  // borrowed timer-related code from neighbortable.cc
  stg_timer_data_s *td = new stg_timer_data_s;
  td->trans_proxy = &prx;
  td->chk = &chk;
  td->chk_ID = chk_id;

  Timer *timer = new Timer(&MF_TransProxy::handleStgExpiry, td);
  //timer->initialize(static_cast<Element*>(&prx));
  timer->initialize((Element*)&prx);
  timer->schedule_after_msec(_chk_stg_ttl);

  //fs.timers.push_back(timer);
  fs.seq_to_timers[seq] = timer;

  // init timer.  Maybe put chunk and a timer/timestamp in a single struct?

  // update flow and overall buf occupancy
  fs.size += pld_size;
  _stg_bytes += pld_size;
}

void MF_StorageManager::reStore(Chunk &chk, Pair<uint32_t, uint32_t> &sd_pair, 
      uint32_t seq) {
  _logger.log(MF_DEBUG, "StorageManager::reStore() for seq %u, src: %u, dst: %u", seq, 
              sd_pair.first, sd_pair.second);

  //Lock lk(_lock);
  if (_sd_to_flows.find(sd_pair) == _sd_to_flows.end()) {
    _logger.log(MF_ERROR, "stg mgr: chunk for reStore not found: sd pair not found \n");
    exit(-1);
  }
  flow_s *fs = *(_sd_to_flows[sd_pair]); 
  std::map<uint32_t, Timer*> &seq_to_timers = fs->seq_to_timers;
  std::map<uint32_t, Chunk*> &seq_to_chks = fs->seq_to_chks;
  if (seq_to_chks.find(seq) == seq_to_chks.end() || 
      seq_to_timers.find(seq) == seq_to_timers.end()) {
    _logger.log(MF_ERROR, "stg mgr: chunk for reStore not found\n");
    exit(-1);
  }
  
  Timer *tm = seq_to_timers[seq];
  if (!tm->scheduled())
    tm->schedule_after_msec(_chk_stg_ttl);
  else
    tm->reschedule_after_msec(_chk_stg_ttl);
}

void MF_StorageManager::unStore(Chunk &chk, Pair<uint32_t, uint32_t> &sd_pair, 
      uint32_t seq, uint32_t pld_size) {
  _logger.log(MF_DEBUG, "StorageManager::unStore() for seq %u, src: %u, dst: %u pair", seq, sd_pair.first, sd_pair.second);
  //Lock lk(_lock);
  if (_sd_to_flows.find(sd_pair) == _sd_to_flows.end()) {
    _logger.log(MF_ERROR, "stg mgr: chunk for unStore not found\n");
    exit(-1);
  }
  flow_s *fs = *(_sd_to_flows[sd_pair]);
  std::map<uint32_t, Timer*> &seq_to_timers = fs->seq_to_timers;
  std::map<uint32_t, Chunk*> &seq_to_chks = fs->seq_to_chks;
  if (seq_to_chks.find(seq) == seq_to_chks.end() || 
      seq_to_timers.find(seq) == seq_to_timers.end()) {
    _logger.log(MF_ERROR, "stg mgr: chunk for unStore not found\n");
    exit(-1);
  }

  // I'm assuming timerdata's destruction is handled by timer's destructor
  delete seq_to_timers[seq];
  seq_to_timers.erase(seq);
  seq_to_chks.erase(seq);

  fs->size -= pld_size; 
  _stg_bytes -= pld_size;
}

/*
MF_StorageManager:: {
}*/

CLICK_ENDDECLS
EXPORT_ELEMENT(MF_StorageManager)
ELEMENT_REQUIRES(userlevel)
