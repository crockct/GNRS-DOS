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
#ifndef MF_GUID_HH_
#define MF_GUID_HH_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#ifndef NO_PKI
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#endif

//define error code 
#define GUID_SUCCESS 0 
#define GUID_NO_PUB_KEY_ERROR -1
#define GUID_WRONG_KEY_FILE_ERROR -2

//define guid length
#define GUID_LENGTH 20
#define HASHED_GUID_LENGTH 20
#define COMPRESSED_GUID_LENGTH 33
#define UNCOMPRESSED_GUID_LENGTH 65

//buffer size for logging
#define GUID_LOG_BUF_SIZE 2 * GUID_LENGTH + 1

class GUID{
public:
  //constructors
  GUID() : alloc_flag(false) {};
  GUID(const GUID& cguid);
  int init(); 
  int init(uint32_t guid_int);
  int init(uint8_t *cguid);
  int init(const uint8_t *cguid); 
  int init(const GUID& cguid);
#ifndef NO_PKI
  int init(const char* filename);
#endif

  ~GUID() {
    if (alloc_flag) {
      delete [] guid;
    }
  }
  //getter 
  uint8_t* getGUID() {                             
    return this->guid; 
  };
 
  //operators
  //assignment
  GUID& operator=(const GUID & guid);
  GUID& operator=(uint8_t * guid);
  GUID& operator=(const uint8_t * guid); 
  GUID& operator=(uint32_t guid_int);               //for test
  

  //equality
  friend bool operator==(const GUID &lhs, const GUID &rhs);
  friend bool operator==(const GUID &lhs, const uint8_t *rhs); 
  friend bool operator==(const GUID &lhs, uint32_t rhs); 
  
  //inequality
  friend bool operator!=(const GUID &lhs, const GUID &rhs);
  friend bool operator!=(const GUID &lhs, const uint8_t *rhs); 
  friend bool operator!=(const GUID &lhs, uint32_t rhs); 
  
  //to_log
  char * to_log(char * buf);

  //convert guid to int 
  uint32_t to_int(); 
  uint8_t *getBytes();
   
private:
  uint8_t *guid;
  bool alloc_flag; 
#ifndef NO_PKI
  const EC_GROUP *ec_paras;
#endif
};

inline GUID::GUID(const GUID & cguid): alloc_flag(true) {
  this->guid = new uint8_t[GUID_LENGTH]();
  memcpy(this->guid, cguid.guid, GUID_LENGTH); 
}

inline int GUID::init() {
  this->guid = new uint8_t[GUID_LENGTH]();
  alloc_flag = true; 
  return 0; 
}

inline int GUID::init(uint32_t guid_int) {        //if use a Int to construct a GUID, use the last 4 bytes
  this->guid = new uint8_t[GUID_LENGTH]();
  memset(this->guid, 0, GUID_LENGTH);
  alloc_flag = true; 
  uint8_t *p = (uint8_t*)&guid_int;
  this->guid[GUID_LENGTH - 4] = *p++;
  this->guid[GUID_LENGTH - 3] = *p++;
  this->guid[GUID_LENGTH - 2] = *p++;
  this->guid[GUID_LENGTH - 1] = *p;
  return 0;
}

inline int GUID::init(const GUID& cguid) {
  this->guid = new uint8_t[GUID_LENGTH];
  alloc_flag = true; 
  memcpy(this->guid, cguid.guid, GUID_LENGTH);
  return 0;
}

inline int GUID::init(uint8_t *cguid) {
  this->guid = cguid;
  return 0;
}

inline int GUID::init(const uint8_t *cguid) {
  this->guid = const_cast<uint8_t*>( cguid );
  return 0;
}

#ifndef NO_SSL
inline int GUID::init(const char* filename) {       //use a public key file to construct a guid
  this->guid = new uint8_t[GUID_LENGTH];
  alloc_flag = true;                                    //alloc memory
  FILE *pubKeyFile = fopen(filename, "r");              //read .pem file
  if( pubKeyFile == NULL ) {
    return GUID_NO_PUB_KEY_ERROR;
  }
  uint32_t bufLen = 100;
  uint8_t *buf = (uint8_t*)malloc(bufLen);
  EVP_PKEY *evp_pkey = EVP_PKEY_new();
  evp_pkey = PEM_read_PUBKEY(pubKeyFile, NULL, NULL, NULL);     //store public key in struct EVP_PKEY
  if( EVP_PKEY_type(evp_pkey->type) != EVP_PKEY_EC ){           //check whether EC_KEY
    return GUID_WRONG_KEY_FILE_ERROR;
  }
  EC_KEY *eckey = EC_KEY_new();
  eckey = EVP_PKEY_get1_EC_KEY(evp_pkey);                       //get ec_key from struct EVP_PKEY
  ec_paras = EC_KEY_get0_group(eckey);
  uint32_t len = EC_POINT_point2oct( EC_KEY_get0_group(eckey),  //write public to buf with length 100
                      EC_KEY_get0_public_key(eckey),
                      POINT_CONVERSION_COMPRESSED, buf, bufLen, 0);//POINT_CONVERSION_COMPRESSED or UNCOMPRESSED
  switch( GUID_LENGTH ) {
  case HASHED_GUID_LENGTH:
    SHA1(buf, len, guid);                                //hash public key to guid;
    break;
  case COMPRESSED_GUID_LENGTH:
    memcpy(guid, buf, COMPRESSED_GUID_LENGTH);
    break;
  case UNCOMPRESSED_GUID_LENGTH:
    break;
  default:
    break;
  }
  return 0;
}
#endif
//assignment operator
inline GUID & GUID::operator=(const GUID &cguid) {
  memcpy(this->guid, cguid.guid, GUID_LENGTH);
  return *this;
}

inline GUID& GUID::operator=(uint8_t * cguid) {
  memcpy(this->guid, cguid, GUID_LENGTH);
  return *this;
}
inline GUID& GUID::operator=(const uint8_t* cguid) {
  memcpy(this->guid, cguid, GUID_LENGTH);
  return *this;
}

inline GUID & GUID::operator=(uint32_t guid_int) {
  memset(this->guid, 0, GUID_LENGTH);
  uint8_t *p = (uint8_t*)&guid_int ;
  this->guid[GUID_LENGTH - 4] = *p++;
  this->guid[GUID_LENGTH - 3] = *p++;
  this->guid[GUID_LENGTH - 2] = *p++;
  this->guid[GUID_LENGTH - 1] = *p;
  return *this;
}

inline bool operator== (const GUID &lhs, const GUID &rhs) {
  for (int i = 0; i != GUID_LENGTH; i++) {
    if (lhs.guid[i] != rhs.guid[i]) {
      return false;
    }
  }
  return true;
};

inline bool operator== (const GUID &lhs, const uint8_t *rhs) {
  for (int i = 0; i != GUID_LENGTH; i++) {
    if (lhs.guid[i] != rhs[i]) {
      return false;
    }
  }
  return true;
};

inline bool operator==(const GUID &lhs, uint32_t rhs) {
  GUID cguid;
  cguid.init(rhs);
  return lhs == cguid;
}

inline bool operator!=(const GUID &lhs, const GUID &rhs) {
  return !(lhs == rhs);
}

inline bool operator!=(const GUID &lhs, uint32_t rhs) {
  return !(lhs == rhs);
}

inline char* GUID::to_log (char* buf) {
  for (int i = 0; i != GUID_LENGTH; ++i) {
    uint8_t temp = this->guid[i] & 0x0f;  
    sprintf( buf + 2 * i, "%x", temp);
    temp = this->guid[i] >> 4;
    sprintf( buf + 2 * i + 1, "%x", temp);  
  }
  buf[GUID_LOG_BUF_SIZE-1] = '\0';
  return buf; 
}

inline uint32_t GUID::to_int() {
  uint32_t *guid = (uint32_t*)(this->guid + GUID_LENGTH - sizeof(uint32_t));
  return *guid;
}

inline uint8_t *GUID::getBytes() {
	return guid;
}


#endif 
