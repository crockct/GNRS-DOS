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
/*
 * MF_Segmentor.hh
 *
 *  Created on: Jun 2, 2011
 *      Author: Kai Su
 */

#ifndef MF_SEGMENTOR_HH_
#define MF_SEGMENTOR_HH_

#include <click/element.hh>
#include <click/timer.hh>
#include <click/vector.hh>
#include <click/hashtable.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/task.hh>
#include <click/sync.hh>
#include <click/deque.hh>

#include <pthread.h>

#include "mf.hh"
//#include "mfcsyntimer.hh"
#include "mfchunkmanager.hh"
#include "mfrouterstats.hh"
#include "mflogger.hh"

/**
 * In Ports: 0 - Chunk packet; 1 - CSYN-ACK packet
 * Out Ports: 0 - Data packet, CSYN packet ;
 *            1 - Chunk packet for rebinding
 */

#define SEG_PORT_OUT_DATA 0
#define SEG_PORT_TO_NET_BINDER 1
#define SEG_PORT_OUT_PRIORITY_DATA 2

#define READY_CHUNK_Q_SCHEDULE_FIFO 0

#define SMALL_CHUNK_LIMIT 32
CLICK_DECLS

class HandlerCall;
class MF_Segmentor; 

/*
  |-----InterfaceQ(sending)-----|------PktOutQ--------|----ReadyChunkQ------|------WaitingChunkTable----|
 */

typedef struct CsynTimerData {
  MF_Segmentor *elem;
  uint32_t hop_id;
  uint32_t chk_pkt_count;
} csyn_timer_data_t;

typedef struct {
  Chunk *chunk;
  bool csyn_acked;
  bool buffer_granted; 
  bool small;
  int priority; 
  uint32_t rcvd_num_pkts;
  Timer *csyn_timer;
  Task *csyn_task; 
  csyn_timer_data_t *timer_data;  
} outstanding_chunk_t; 

class MF_Segmentor : public Element {
 public:
  MF_Segmentor();
  ~MF_Segmentor();

  const char *class_name() const		{ return "MF_Segmentor"; }
  const char *port_count() const		{ return "2/3"; }
  const char *processing() const		{ return "h/h"; }

  int configure(Vector<String>&, ErrorHandler *);
  inline void push(int port, Packet *p); 

  int initialize(ErrorHandler *);
  bool run_task(Task *); 
  
  static void handleCsynExpiry(Timer *, void *);
  //static bool handleCsynExpiry(Task *, void *); 
  void CsynExpire(uint32_t hop_id, uint32_t chk_pkt_cnt);
  void add_handlers();
  void selected(int fd, int mask);

  void rescheduleTask();

 private:
  static String read_handler(Element *e, void *thunk);

  void handleChkPkt(Packet *);
  void handleACKPkt(Packet *);
  
  Packet* makeCsynPacket(uint32_t hop_id, uint32_t chunk_size, uint32_t next_hop_guid);
  outstanding_chunk_t * createOutstandingChunkStruct(uint32_t hop_id, 
                                                     Chunk *chunk, bool direct = false); 
  void deleteOutstandingChunkStruct(outstanding_chunk_t *outstanding_chk); 
  
  //void sendCSYN(uint32_t hop_id, int chk_size, bool is_resend, bool priority = false);
  
  //void sendCsynAskForBuffer();
  //void sendCsynEnd();
  
  void resendCsyn(uint32_t hop_id, uint32_t chk_size); 
   
  void sendPackets(uint32_t hop_id, Chunk *chunk, Vector<int> *lost, bool is_resend); 
  bool sendSmallChunk(Chunk *chunk);
  bool sendChunk(Chunk *chunk); 
  
  //parse lost seq no from CSYN-ACK
  Vector<int>* getLostSeq(char *bitmap, uint32_t chunksize);
  
  //thread safe hopID++
  inline uint32_t getNewLocalHopID() {
    pthread_mutex_lock(&_hop_id_lock);
    ++_mono_hop_ID;
    pthread_mutex_unlock(&_hop_id_lock);
    return _mono_hop_ID; 
  }

  void checkWaitingChunkCache(); 
  
  int releaseOutstandingChunk(uint32_t hop_id);  
  //bool insertOutstandingChunk(uint32_t hop_id, Chunk *chunk, bool is_small = false);

  bool startOutstandingCsynTimer(uint32_t hop_id, uint32_t msec = CSYN_TIMEOUT); 
  bool stopOutstandingCsynTimer(uint32_t hop_id); 
  
  //return msec
  uint32_t computeCsynTimeout(); 
   
  //push chunk to ready chunk q
  inline void pushReadyChunkQ(Chunk *chunk) {
    _ready_chunkQ->push_back(chunk);
  }

  //default method is 0 which is FIFO
  //TODO: a scheduler
  Chunk* popReadyChunkQ(uint32_t method = READY_CHUNK_Q_SCHEDULE_FIFO); 

  //This Task is used to send csyn for new chunks.
  Task _task;
  // because this task can be scheduled by 1. seg thread (csyn timeout); 2. agg thread (ack/chk packet
  // received).
  pthread_mutex_t _task_lock;
  
  typedef HashTable<uint32_t, outstanding_chunk_t*> OutstandingChunkBuffer;
  OutstandingChunkBuffer *_outstanding_chk_buf;
  //pthread_mutex_t _outstanding_chk_buf_lock;
  Spinlock _outstanding_chk_buf_lock;

  typedef Deque<Packet*> PacketOutQ;
  PacketOutQ *_packet_outQ;
  pthread_mutex_t _packet_outQ_lock; 
  
  uint32_t popPacketOutQ();
  
  typedef Deque<Chunk*> ReadyChunkQ;
  ReadyChunkQ *_ready_chunkQ; 
  pthread_mutex_t _ready_chunkQ_lock;

  //imcomplete chunks, waiting for pkts
  typedef HashTable<uint32_t, Chunk*> WaitingChunkTable; 
  WaitingChunkTable *_waiting_chunk_cache; 
  pthread_mutex_t _waiting_chunk_cache_lock;
  
  //volatile int _cur_sending_win;
  int _cur_sending_win;
  pthread_mutex_t _sending_win_lock; 
 
  
  int _sending_win_size;

  //window of cysn in process
  //volatile int _cur_csyn_win;
  int _cur_csyn_win;
  pthread_mutex_t _csyn_win_lock;
  
  //Monotonically increasing final hop ID assigned 
  //to outgoing chunks 
  uint32_t _mono_hop_ID;
  pthread_mutex_t _hop_id_lock;  
  
  //for update router's readyCache size and WaitingCache size
  MF_RouterStats *_routerStats;
 
  typedef HashTable<uint32_t, Packet*> DgramTable;
  DgramTable *_internal_msg_tbl;
  pthread_mutex_t _internal_msg_tbl_lock;
  
  void pushToNetBinder(uint32_t chunk_id);
      
  MF_ChunkManager *_chunkManager; 
  MF_Logger logger;
  
  //for statistic
  int _sent_chk_count;

  // file descriptors used in selected()
  int _rfd;
  int _wfd; 
};

CLICK_ENDDECLS;

#endif /* MF_SEGMENTOR_HH_ */
