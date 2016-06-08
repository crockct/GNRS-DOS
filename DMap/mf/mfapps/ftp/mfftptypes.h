#ifndef MF_FTPTYPES_H_
#define MF_FTPTYPES_H_

#include <stdint.h>
#include <stdbool.h>

#define DATA 0
#define LIST_MSG 1
#define LIST_RESP_MSG 2
#define GET_MSG 3
#define PUT_MSG 4
#define NO_FILE_MSG 5
#define FILE_INFO_MSG 6

#define MAX_FILENAME_LEN 256
#define CHUNK_SIZE 1024000

extern bool debug; 
//true if receiving
extern bool receiving;

extern uint32_t my_guid; 

typedef struct {
  uint32_t type; 
  uint32_t src_guid; 
} list_msg_t;

typedef struct {
  uint32_t type;
  uint32_t src_guid;
  char filename[MAX_FILENAME_LEN];
} get_msg_t; 

typedef struct {
  uint32_t type;
  char filename[MAX_FILENAME_LEN]; 
} no_file_msg_t;

typedef struct {
  uint32_t type;
  uint32_t src_guid;
  uint32_t file_size; 
  char filename[MAX_FILENAME_LEN]; 
} put_msg_t; 

typedef struct {
  uint32_t type;
  uint32_t file_size;
  char filename[MAX_FILENAME_LEN]; 
} file_info_msg_t;

#endif
