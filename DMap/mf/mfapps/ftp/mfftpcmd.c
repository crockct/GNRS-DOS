#include "mfftpcmd.h"

int send_list_msg(struct Handle *handle, uint32_t my_guid, uint32_t dst_guid, 
                  uint32_t svc_opt) {
  //fill list msg
  list_msg_t list;
  memset(&list, 0, sizeof(list_msg_t));
  list.type = htonl(LIST_MSG);
  list.src_guid = htonl(my_guid);
  
  gettimeofday(&start, NULL);

  //send list msg
  int ret = mfsend(handle, (char*)&list, sizeof(list_msg_t), dst_guid, svc_opt); 
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
    return ret; 
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT LIST to dst_guid: %u\n", 
            dst_guid);
  }
  return 0; 
}

int send_get_msg(struct Handle *handle, uint32_t my_guid, uint32_t dst_guid,
                 char *filename) {
  
  //fill get_msg
  get_msg_t get;
  memset(&get, 0, sizeof(get_msg_t));
  get.type = htonl(GET_MSG);
  get.src_guid = htonl(my_guid);
  strcpy(get.filename, filename);
  
  uint32_t msg_size = sizeof(uint32_t)*2 + strlen(filename) + 1;

  int32_t ret = mfsend(handle, &get, msg_size, dst_guid, 0);
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
    return ret;
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT GET to dst guid: %u\n",
            dst_guid);
  }
  return 0; 
}

int send_put_msg(struct Handle *handle, uint32_t my_guid, uint32_t dst_guid,
                 char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: Cannot open file %s\n", filepath);
    return -1; 
  } 
  
  const char *filename = extract_filename(filepath); 
  fseek(fp, 0L, SEEK_END);
  int32_t file_size = ftell(fp);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: file size: %u\n", file_size);
  }
  //fill put msg
  put_msg_t put;
  put.type = htonl(PUT_MSG);
  put.src_guid = htonl(my_guid);
  put.file_size = htonl(file_size); 
  strcpy(put.filename, filename);

  fseek(fp, 0L, SEEK_SET);
  fclose(fp);

  //send buf
  uint32_t msg_size = sizeof(uint32_t) * 3 + strlen(filename) + 1; 
  int32_t ret = mfsend(handle, &put, msg_size, dst_guid, 0);
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
    return ret;
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT PUT to dst_guid: %u filename: %s "
            "size: %u\n", 
            dst_guid, filename, file_size);
  }
  return 0; 
}

int send_no_file_msg(struct Handle *handle, uint32_t dst_guid, 
                         char *filename) {
  no_file_msg_t no_file;
  no_file.type = htonl(NO_FILE_MSG);
  strcpy(no_file.filename, filename);

  uint32_t msg_size = sizeof(uint32_t) + strlen(filename) + 1;

  int32_t ret = mfsend(handle, &no_file, msg_size, dst_guid, 0) ;
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
    return -1;
  }
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT NO_FILE to dst_guid: %u\n", dst_guid);
  }
  return 0; 
}

int send_file_info(struct Handle *handle, uint32_t dst_guid, char *filepath) {
  char *filename = extract_filename(filepath); 
  
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: Cannot open file %s\n", filepath);
    send_no_file_msg(handle, dst_guid, filename); 
    return -1; 
  }

  fseek(fp, 0L, SEEK_END);
  //fill file_info_msg
  int32_t file_size = ftell(fp);
  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: file size: %u\n", file_size);
  }
  
  file_info_msg_t file_info;
  file_info.type = htonl(FILE_INFO_MSG);
  file_info.file_size = htonl(file_size);
  strcpy(file_info.filename, filename);

  //send file info
  uint32_t msg_size = sizeof(uint32_t) * 2 + strlen(filename) + 1;
  int32_t ret = mfsend(handle, &file_info, msg_size, dst_guid, 0);
  if (ret < 0) {
    fprintf(stderr, "MFFTP_LOG: ERROR: mfsend error\n");
    return -1;
  }

  if (debug) {
    fprintf(stdout, "MFFTP_LOG: DEBUG: SENT FILE_INFO, filename: %s\n",
            filename);
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  return 0; 
}

int send_file(struct Handle *handle, uint32_t dst_guid, char *filepath, 
              uint32_t svc_opt) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    fprintf(stderr, "MFFTP_LOG: ERROR: Cannot open file %s\n", filepath);
    return -1;
  }

  //send file data
  char *buf = malloc(MAX_CHUNK_LEN * sizeof(char));
  *(uint32_t*)buf = htonl(DATA);
  uint32_t chk_cnt = 0; 
  int ret = 0; 
  do {
    uint32_t sz = fread(buf + sizeof(uint32_t), sizeof(char), 
                          CHUNK_SIZE - sizeof(uint32_t), fp);
    if (!sz) {
      break; 
    }
    if (debug) {
      fprintf(stdout, "MFFTP_LOG: DEBUG: chunk #: %u size: %u\n", 
              chk_cnt, sz);
    }
    ret = mfsend(handle, buf, sz + sizeof(uint32_t), dst_guid, svc_opt);
    if ( ret < 0 ) {
      fprintf(stderr, "MFFTP_LOG: ERROR: mfsendmsg error\n");
      return EXIT_FAILURE;
    }
    if (debug) {
      fprintf(stdout, "MFFTP_LOG: DEBUG: SENT chunk #: %u bytes: %u\n", 
              chk_cnt, ret); 
    }
    chk_cnt++; 
  } while (ret == CHUNK_SIZE);
  fclose(fp);
  return 0; 
}
