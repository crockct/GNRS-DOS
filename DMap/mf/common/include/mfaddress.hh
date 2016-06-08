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
#ifndef MF_ADDRESS_HH
#define MF_ADDRESS_HH

#define ADDRESS_LENGTH 4     //bytes
#define ADDRESS_IPV4 1

#define ADDRESS_LOG_BUF_SIZE 9

class NA {
public:
  NA() : alloc_flag(false) {
  }
  
  NA(const NA& addr) {
    na = new uint8_t[ADDRESS_LENGTH]();
    alloc_flag = true; 
    memcpy(this->na, addr.na, ADDRESS_LENGTH);
  }
  
  inline int32_t init (uint32_t addr_int) {
    na = new uint8_t[ADDRESS_LENGTH];
    alloc_flag = true; 
    uint8_t *p = (uint8_t*)&addr_int;
    for( int i = 0; i != ADDRESS_LENGTH; i++ ) {
      this->na[i] = *p++; 
    }
    return 0; 
  }
  
  inline int32_t init (const uint8_t* addr) {
    na = const_cast<uint8_t*>(addr); 
    return 0; 
  }

  inline int32_t init (const NA& addr) {
    this->na = new uint8_t[ADDRESS_LENGTH];
    alloc_flag = true; 
    memcpy(this->na, addr.na, ADDRESS_LENGTH);
    return 0; 
  }
  ~NA() {
    if (alloc_flag) {
      delete [] na; 
    }
  }
  
  uint32_t to_int() {
    return *(uint32_t*)na; 
  }
  
  bool isEmpty() {
    for (uint32_t i = 0; i != ADDRESS_LENGTH; i++) {
      if (this->na[i] != 0) {
        return false;
      }
    }
    return true;  
  }

  void setNA (const uint8_t* addr) {
    na = const_cast<uint8_t*>(addr); 
  }
  
  NA& operator= (const NA& addr); 
  NA& operator= (uint32_t addr_int); 
  
  friend bool operator==(const NA &lhs, const NA &rhs); 
  friend bool operator==(const NA &lhs, uint32_t rhs); 

  friend bool operator!=(const NA &lhs, const NA &rhs); 
  friend bool operator!=(const NA &lhs, uint32_t rhs); 
  
  char * to_log( char* buf ) {
    for( uint32_t i = 0; i != ADDRESS_LENGTH; i++) {
      uint8_t temp = this->na[i] & 0x0f;
      sprintf( buf + 2 * i, "%x", temp ); 
      temp = this->na[i]>>4; 
      sprintf( buf + 2 * i + 1, "%x", temp ); 
    }
    return buf; 
  }

private:
  uint8_t *na;
  bool alloc_flag; 
};

inline NA& NA::operator=(const NA& addr) {
  memcpy(this->na, addr.na, ADDRESS_LENGTH);
  return *this;
}

inline NA& NA::operator=(uint32_t addr_int) {
  uint8_t *p = (uint8_t*)&addr_int;
  for (int i = 0; i != ADDRESS_LENGTH; i++) {
    this->na[i] = *p++;
  }  
  return *this; 
}

inline bool operator==(const NA &lhs, const NA &rhs) {
  for( int i = 0 ; i != ADDRESS_LENGTH; i++ ) {
    if(lhs.na[i] != rhs.na[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator==(const NA &lhs, const uint32_t rhs) {
  NA addr;
  addr.init(rhs); 
  return lhs == addr; 
}

inline bool operator!=(const NA &lhs, const NA &rhs) {
  return !(lhs == rhs); 
}

inline bool operator!=(const NA &lhs, uint32_t rhs) {
  return !(lhs == rhs); 
}

#endif  /*MF_ADDRESS_HH_*/
