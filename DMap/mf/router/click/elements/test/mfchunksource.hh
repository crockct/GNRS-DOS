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
 * MF_ChunkSource.hh
 *
 */

#ifndef MF_CHUNKSOURCE_HH_
#define MF_CHUNKSOURCE_HH_

#include <click/element.hh>
#include <click/task.hh>
#include <click/notifier.hh>
#include <click/vector.hh>
#include <click/standard/scheduleinfo.hh>

//for generate random number
#include <cstdlib>
#include <ctime>

#include "mf.hh"
#include "mfrouterstats.hh"
#include "mfchunkmanager.hh"
#include "mflogger.hh"

#define DEFAULT_CHUNK_SIZE 1024000

class MF_ChunkSource : public Element {
 public:
  MF_ChunkSource();
  ~MF_ChunkSource();

  const char *class_name() const		{ return "MF_ChunkSource"; }
  const char *port_count() const		{ return "0/1"; }
  const char *processing() const		{ return "h/h"; }

  int configure(Vector<String>&, ErrorHandler *);
  int initialize(ErrorHandler *);
  void add_handlers();

  bool run_task(Task *);

 private:
  int parseDstNAs();
  int parseChunkSizeList();
  
  int getChunkSize();
  
  bool sendStartMeasurementMsg(); 
  
  Task _task;
  Timer _timer;
  bool _active;
  bool _start_msg_sent; 
  /* to check if downstream has capacity */
  NotifierSignal _signal;

  /* supplied arguments */
  uint32_t _src_GUID;
  uint32_t _dst_GUID;
  uint32_t _service_id;
  int32_t _chk_lmt;
  uint32_t _chk_intval_msec;
  uint32_t _delay_sec;
  uint32_t _pkt_size;
  String _dst_NAs_str;
  String _chk_size_list_str;
  uint32_t _chk_size_variation;   //default is 0 which means chunk size is fixed. maxi is _chk_size, 
                                  //in this case chunk size ranges from 0 to _chk_size + variation
    
  /* monotonically increasing unique chunk identifier, starts at 1 */
  uint32_t _curr_chk_ID;
  uint32_t _curr_chk_cnt;

  Vector<NA> _dst_NAs;

  Vector<uint32_t> _chk_size_list;
  
  MF_RouterStats *_routerStats;               
  MF_ChunkManager *_chunkManager; 
  MF_Logger logger;
};

CLICK_ENDDECLS
#endif /* MF_CHUNKSOURCE_HH_ */
