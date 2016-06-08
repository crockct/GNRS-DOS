#include "mfftpreceiving.h"

void *receiving_thread(void *arg) {
  //thread_data_t *data = (thread_data_t*)arg;
  //struct Handle *handle = data->handle;
  //FILE *get_fp = data->get_fp;
  struct Handle *handle = (struct Handle*)arg; 
  char buf[MAX_CHUNK_LEN];
  int ret = 0;
  FILE *fp = NULL;
  uint32_t file_size = 0;
  uint32_t received_bytes = 0;
  struct timeval start_chk; 
  uint32_t chk_cnt; 
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: Receiving....\n");
  }
  //set receiving status
  receiving = true; 
  //receiving data
  while (1) {
    ret = mfrecv_blk(handle, NULL, buf, MAX_CHUNK_LEN, NULL, 0);
    if (ret < 0) {
      fprintf(stderr, "MFFTP_LOG: ERROR: mfrecv_blk error\n");
      pthread_exit(NULL); 
    }
    if (debug) {
      fprintf(stdout, "MFFTP_LOG: DEBUG: Received %u bytes from MFAPI\n", ret); 
    }
    uint32_t *type = (uint32_t*)buf;
    switch (ntohl(*type)) {
    case DATA: {
      struct timeval end;
      gettimeofday(&end, NULL);
      double interval = end.tv_sec * 1000.0 + end.tv_usec / 1000.0 -
          start_chk.tv_sec * 1000.0 - start_chk.tv_usec / 1000.0;

      if (debug) {
        fprintf(stdout, "MFFTP_LOG: DEBUG: Received Data chunk #: %u, "
                "interval: %2.4f msec\n", chk_cnt, interval);
      }
      gettimeofday(&start_chk, NULL); 
      received_bytes += (ret - sizeof(uint32_t)); 
      process_data(buf, ret, fp);
      if (received_bytes == file_size) {
        double time = end.tv_sec * 1000.0 + end.tv_usec / 1000.0 -
            start.tv_sec * 1000.0 - start.tv_usec / 1000.0;
        fprintf(stdout, "MFFTP_LOG: DEBUG: All data received! total time: %2.4f\n", 
                time);
        fclose(fp);
        file_size = 0;
        received_bytes = 0;
        chk_cnt = 0; 
      }
      ++chk_cnt; 
      break; 
    }
    case LIST_MSG:
      process_list_msg(handle, buf); 
      break;
    case LIST_RESP_MSG:
      process_list_resp_msg(buf, ret); 
      break;
    case GET_MSG:
      process_get_msg(handle, buf); 
      break;
    case PUT_MSG:
      gettimeofday(&start, NULL);
      gettimeofday(&start_chk, NULL);
      //put_msg_t *put = (put_msg_t*)buf;   
      process_put_msg(buf, &file_size, &fp); 
      break;
    case NO_FILE_MSG:
      process_no_file_msg(buf);
      break; 
    case FILE_INFO_MSG:
      gettimeofday(&start, NULL);
      gettimeofday(&start_chk, NULL); 
      process_file_info_msg(buf, &file_size, &fp); 
      break;
    default:
      fprintf(stderr, "MFFTP_LOG: ERROR: Unsupported command\n"); 
      break;
    }
  }
}

int32_t process_file_info_msg(char* buf, uint32_t *file_size, FILE **fpp) {
  file_info_msg_t *file_info = (file_info_msg_t*)buf;
  *file_size = ntohl(file_info->file_size);
  *fpp = fopen(file_info->filename, "a");
  if (!fpp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: cannot open file %s\n", 
            file_info->filename); 
    return -1; 
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: RECEIVED FILE INFO, "
            "filename: %s size: %u\n", 
            file_info->filename, *file_size);
  }
  return 0; 
}

int32_t process_data(char *buf, uint32_t size, FILE *fp) {  
  if (!fp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: File isn't open\n"); 
    return -1; 
  }
  uint32_t sz = fwrite(buf + sizeof(uint32_t), sizeof(char), 
                         size - sizeof(uint32_t), fp);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: WRITE %u bytes to file\n", sz); 
  }
  if (size != CHUNK_SIZE) {
    return 1; 
  }
  return 0; 
}

int32_t process_list_msg(struct Handle *handle, char *buf) {
  //parse list_msg
  list_msg_t *list = (list_msg_t*)buf;
  uint32_t dst_guid = ntohl(list->src_guid);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: RECEIVED LIST from src guid: %u\n", 
            dst_guid);
  }

  uint32_t cnt = 0; 
  uint32_t type = htonl(LIST_RESP_MSG);
  memcpy(buf, &type, sizeof(uint32_t));
  cnt += sizeof(uint32_t);
  
  DIR *dirptr;
  struct dirent *entry;
  dirptr = opendir(MFFTP_DIRECTORY);
  
  if (dirptr) {
    while (entry = readdir(dirptr)) {
      if (entry->d_type == DT_REG) {
        memcpy((void*)(buf+cnt), (void*)entry->d_name, 
                  strlen(entry->d_name) + 1);
        if (debug) {
          fprintf(stdout, "%s\n", entry->d_name);
        }
        cnt = cnt + strlen(entry->d_name) + 1;
      }
    }
    closedir (dirptr);
  } else {
    fprintf(stderr, "MFFTP_LOG: ERROR: cannot open current directory\n");
  }
  
  int32_t ret = mfsend(handle, buf, cnt, dst_guid, 0);
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
    return EXIT_FAILURE;
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT LIST_RESP to dst guid: %u "
            "size: %u\n", 
            dst_guid, cnt);
  }
  return 0; 
}

int32_t process_list_resp_msg(char *buf, int size) {
  struct timeval end;
  gettimeofday(&end, NULL);
  double time_spend = end.tv_sec * 1000.0 + (end.tv_usec / 1000.0) - 
                        start.tv_sec * 1000.0 - (start.tv_usec/1000.0);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: RECEIVED LIST_RESP\n");
  }
  uint32_t cnt = sizeof(uint32_t);
  //char filename[256];
  do {
    fprintf(stdout, "%s\r\n", buf + cnt);
    cnt = cnt + strlen(buf+cnt) + 1;
  } while (cnt < size);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: Latency: %2.4f mecs\n", time_spend);
  }
  
  return 0; 
}

int32_t process_get_msg(struct Handle *handle, char *buf) {
  get_msg_t *get = (get_msg_t*)buf;
  uint32_t dst_guid = ntohl(get->src_guid);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: RECEIVED GET from %u filename: %s\n", 
            dst_guid, get->filename);
  }
  char path[65535];
  gen_server_path(get->filename, MFFTP_DIRECTORY, path);
 
  fprintf(stdout, "MFFTP_LOG: SENDING file %s ...\n", get->filename); 
  if (send_file_info(handle, dst_guid, path) == 0) {
    send_file(handle, dst_guid, path, 0);
  }
  fprintf(stdout, "MFFTP_LOG: DONE\n");
  return 0; 
}

int32_t process_no_file_msg(char *buf) {
  no_file_msg_t *no_file = (no_file_msg_t*)buf;
  fprintf(stdout, "MFFTP_LOG: No file: %s\n", no_file->filename);
  return 0; 
}

int32_t process_put_msg(char *buf, uint32_t *file_size, FILE **fpp) {
  put_msg_t *put = (put_msg_t*)buf;
  *file_size = ntohl(put->file_size);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: RECEIVED PUT from src guid: %u, "
            "filename: %s size: %u\n",
            ntohl(put->src_guid), put->filename, *file_size);
  }

  char path[65535];
  gen_server_path(put->filename, MFFTP_DIRECTORY, path);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: path %s\n", path); 
  }
  *fpp = fopen(path, "w");
  if (!fpp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: cannot open file %s\n",
            put->filename);
    return -1;
  }
  return 0; 
}

