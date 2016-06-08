#include "mfclient.h"

void usage() {
  printf("Usage:\n");
  printf("\t-s\t work as a sender\n");
  printf("\t-r\t work as a receiver\n");
  printf("\t-o\t option choosing mode\n"); 
  printf("\t-m\t my_guid (required)\n");
  printf("\t-d\t dst_guid(required without -o)\n");
  printf("\t-P\t mfopen profile (optional, default is basic)\n");
  printf("\t-S\t service id (optional, default is unicast)\n"); 
  printf("\t[-M/-f]\t message or file. Note message format: "
         "'message'(required without -o)\n");
  printf("\t-a\t attach guids, Note: GUIDs format: '{guid1, guid2,...}' "
         "(optional)\n");
  printf("\t-h\t help, print this\n");
  printf("\t-v\t version\n");
  printf("Examples:\n");
  printf("\t./mfclient -s -m 1 -d 5 -M 'hello world'\n");
  printf("\t./mfclient -s -m 1 -d 5 -f filename\n"); 
  printf("\t./mfclient -r -m 1 -a '{3, 7}'\n");
  printf("\t./mfclient -m 1 -o\n"); 
}

int main(int argc, char *argv[]) {
  extern char* optarg;
  bool sendflag = false;
  bool receiveflag = false;
  bool optmodeflag = false; 
  int my_guid = 0;
  int dst_guid = 0; 
  int attached_guid_num = 0;
  int *attached_guids = NULL;
  char *msg = NULL;
  char *file_name = NULL; 
  int sid = 0;

  pthread_t thread_id[2]; 
  int c = 0;
  while ((c = getopt(argc, argv, "srom:d:a:M:S:f:hv")) != -1) {
    switch (c) {
    case 's':
      sendflag = true;
      break;
    case 'r':
      receiveflag = true;
      break;
    case 'o':
      optmodeflag = true;
      break; 
    case 'd':
      dst_guid = atoi(optarg);
      break;
    case 'm':
      my_guid = atoi(optarg);
      break;
    case 'M':
      msg = optarg;
      break;
    case 'f':
      file_name = optarg;
      break; 
    case 'S':
      sid = atoi(optarg);
      break; 
    case 'a':
      attached_guids = parseAttachedGUIDs(optarg, &attached_guid_num);
      if (attached_guids == NULL) {
        fprintf(stderr, "no guids attached\n"); 
        usage(); 
      }
      break;
    case 'h':
      usage();
      exit(0); 
    case 'v':
      printf("current version is %4.2f\n", VERSION); 
      exit(0); 
    case '?':
      usage(); 
      return ARGUMENT_ERROR;
    default:
      break; 
    }
  }
  
  /*my_guid is required*/
  if (my_guid == 0) {
    fprintf(stderr, "my guid is required\n");
    usage();
    exit(EXIT_FAILURE); 
  } 
  /*non-option mode sender needs to specify dst_guid and message/filename*/
  if (!optmodeflag &&sendflag) {
    if (!dst_guid || (!msg && !file_name)) {
      fprintf(stderr, "dst_guid and msg/filename is required while "
                      "not using option mode(-o)\n"); 
      usage(); 
      exit(EXIT_FAILURE); 
    }
  }

  if (msg && file_name) {
    fprintf(stderr, "cannot send message and file at the same time\n");
    usage();
    exit(EXIT_FAILURE); 
  }
   
  /*if both -s and -r are not set, set both of them*/
  if ((!sendflag) && (!receiveflag)) {
    printf("No working mode defined."); 
    if (msg != NULL || optmodeflag) {
      sendflag = true;
    }
    receiveflag = true;
  } else if (!sendflag && receiveflag) {
    if (msg != NULL) {
      usage(); 
      fprintf(stderr, "receiver cannot send msg, omit parameter \"-m\"\n");
    }
  } else if (sendflag && !receiveflag) {
    if (attached_guid_num != 0) {
      usage(); 
      fprintf(stderr, "sender, omit parameter \"-a\"\n");
    }
  } else {
  }
  
  /*print working mode*/
  printf("Woking as ");
  if (sendflag) {
    printf("sender ");
  }
  if (receiveflag) {
    if (sendflag) {
      printf("and "); 
    }
    printf("receiver "); 
  }
  printf("\n"); 
  printf("My GUID: %u\n", my_guid);

  /*mfopen*/
  struct Handle handle;
  int ret = 0;
  ret = mfopen(&handle, "basic\0", 0, my_guid);
  if (ret) {
    fprintf(stderr, "mfopen error\n");
    exit(EXIT_FAILURE); 
  }
  
  int thread_cnt = 0; 

  /*create receiving thread*/
  if (receiveflag) {
    ret  = pthread_create(&thread_id[thread_cnt], NULL, receive_thread_start, 
                            &handle); 
    if (ret) {
      fprintf(stderr, "cannot create receive thread\n");
      exit(EXIT_FAILURE); 
    }
    ++thread_cnt;
    usleep(1000); 
  }
  
  /*create option thread*/
  if (optmodeflag) {
    option_thread_arg_t option_arg;
    option_arg.handle = &handle;
    option_arg.sendflag = sendflag;
    option_arg.receiveflag = receiveflag; 
  
    ret = pthread_create(&thread_id[thread_cnt], NULL, option_thread_start, 
                         &option_arg); 
    if (ret) {
      fprintf(stderr, "cannot create option thread\n");
      exit(EXIT_FAILURE); 
    }
    ++thread_cnt;
  } 
  
  if (sendflag && (msg != NULL)) {
    //file msg pkt
    trans_msg_t msg_to_send;
    memset(&msg_to_send, 0, sizeof(msg_to_send)); 
    msg_to_send.type = htonl(MSG);
    strcpy(msg_to_send.msg, msg); 
    uint32_t msg_size = sizeof(uint32_t) + strlen(msg) + 1; 
    //send msg pkt to api
    ret = mfsend(&handle, &msg_to_send, msg_size, dst_guid, sid);
    if (ret < 0) {
      fprintf(stderr, "mfsend error\n");
      exit(EXIT_FAILURE);
    }
    printf("sent message: %s\n", msg);
  }
  
  if (sendflag && file_name) {
    FILE *pf;
    pf = fopen(file_name, "r");
    if (!pf) {
      fprintf(stderr, "cannot open file %s\n", file_name);
      return EXIT_FAILURE; 
    }
    fseek(pf, 0L, SEEK_END);
    int32_t file_size = ftell(pf);
    //fill file metadata pkt
    trans_file_t file_to_send;
    file_to_send.type = htonl(FILE_META);
    file_to_send.file_size = htonl(file_size);
    strcpy(file_to_send.filename, file_name);
    uint32_t size = sizeof(uint32_t) * 2 + strlen(file_name) + 1;
    ret = mfsend(&handle, &file_to_send, size, dst_guid, sid);
    if (ret < 0) {
      fprintf(stderr, "mfsend error\n");
      exit(EXIT_FAILURE); 
    }
    //process data in file
    fseek(pf, 0L, SEEK_SET);
    char *buffer = (char*)malloc(CHUNK_LEN * sizeof(char));
    uint32_t type = htonl(FILE_DATA);
    memcpy(buffer, &type, sizeof(type));
    //trans_file_data_t *file_data = (trans_file_data_t*)buffer;
    //file_data->type = htonl(FILE_DATA);
    //file_data->file_data = (char*)(buffer + sizeof(uint32_t)); 
    
    do {
      printf("check!\n"); 
      size = fread(buffer + sizeof(uint32_t), sizeof(char), 
                      CHUNK_LEN - sizeof(uint32_t), pf);
      ret = mfsend(&handle, buffer, size + sizeof(uint32_t), dst_guid, 0); 
      printf("read: %u, send %u, max_chunk_len: %u\n", size, ret, CHUNK_LEN); 
      if (ret < 0) {
        fprintf(stderr, "mfsend error\n");
        exit(EXIT_FAILURE);
      }
    } while (ret == CHUNK_LEN); 

    fclose (pf);
    printf("sent file: %s, file size: %u, to GUID: %u \n",
              file_name, file_size, dst_guid);
    if (buffer != NULL) { 
      free(buffer);
    }
  }
  
  int i = 0; 
  if (attached_guid_num != 0 ) {
    printf("%u attatched guids\n", attached_guid_num);
    for (i = 0; i != attached_guid_num; ++i) {
      printf("attached guid # %u: %u\n", i, attached_guids[i]); 
    }
    ret = mfattach(&handle, attached_guids, attached_guid_num);
    if (ret) {
      fprintf(stderr, "mfattach error\n");
      return MFATTACH_ERROR; 
    }
  } 
  i = 0;
  for(; i != thread_cnt; ++i) { 
    pthread_join(thread_id[i], NULL);
  }
  if (attached_guids != NULL) {
    free(attached_guids);
  }
  return 0; 
}

int attach_guid(struct Handle *handle) {
  printf("input guid you want to attach:");
  int guid = 0; 
  scanf("%d", &guid);
  /*flush stdin*/
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
  
  int ret = mfattach(handle, &guid, 1);
  if (ret) {
    fprintf(stderr, "mfattach error\n");
    return -1; 
  }
  return 1; 
}

int send_msg(struct Handle *handle) {
  int dst_guid = 0; 
  printf("input dst guid: ");
  scanf("%d", &dst_guid);
 
  /*flush stdin buffer, otherwise getline() doesn't work*/
  int c;
  while ((c = getchar()) != '\n' && c != EOF);

  if (!dst_guid) {
    fprintf(stderr, "dst_guid cannot be 0\n"); 
    return -1; 
  }
  size_t msg_size = MAX_MSG_LEN;
  char *msg = (char*)malloc(msg_size + 1);
  printf("input message (maximum length is %d): ", MAX_MSG_LEN);
  int read_byte = getline(&msg, &msg_size, stdin);
  if (read_byte == -1) {
    fprintf(stderr, "read stdin error");
    return -1; 
  } else {
    printf("dst_guid: %u message : %s", dst_guid, msg);
    int ret = mfsend(handle, msg, read_byte, dst_guid, 0);
    free(msg); 
    if (ret < 0) {
      fprintf(stderr, "mfsend error\n");
      return -1; 
    }
    return ret; 
  }
}

int send_file(struct Handle *handle) {
  int dst_guid = 0;
  printf("input dst guid: ");
  scanf("%d", &dst_guid);

  if (!dst_guid) {
    fprintf(stderr, "dst_guid cannot be 0\n");
    return -1;
  }

  char filename[256]; 
  printf("input filename:"); 
  scanf("%s", filename); 
  int c;
  while ((c = getchar()) != '\n' && c != EOF);

  FILE *pf;
  pf = fopen(filename, "r");

  if (pf == NULL) {
    fprintf(stderr, "cannot open file %s\n", filename);
    return -1; 
  }
  fseek(pf, 0L, SEEK_END);
  int buf_size = ftell(pf);
  char *buffer = (char*)malloc(buf_size); 
  fseek(pf, 0L, SEEK_SET);
  
  while (!feof (pf)) {
    printf("read file\n"); 
    if (fgets(buffer , buf_size, pf) == NULL ) {
      break;
    }
  }
  fclose (pf);
  
  int ret = mfsend(handle, buffer, buf_size, dst_guid, 0);
  if (ret < 0) {
    fprintf(stderr, "mfsend error\n");
    return ret; 
  } 
  
  printf("send file: %s, file size: %u, to GUID: %u \n", 
            filename, buf_size, dst_guid);
  free(buffer); 
  return ret;
  
}

void *receive_thread_start(void *arg) {
  struct Handle *handle = (struct Handle*)arg; 
  printf("start to receive messages...\n");
  char buf[CHUNK_LEN];
  int ret = 0;
  FILE *pf;
  uint32_t file_size = 0;
  uint32_t received_size = 0; 
  while (1) {
    ret = mfrecv_blk(handle, NULL, buf, CHUNK_LEN, NULL, 0);
    if (ret < 0) {
      fprintf(stderr, "mfrecv_blk error\n");
      pthread_exit(NULL); 
    }
    int32_t *type = buf;
    if (*type == ntohl(MSG)) {
      trans_msg_t *received_msg = (trans_msg_t*)buf; 
      printf("Message: \"%s\" is received\n", received_msg->msg);
      continue;      
    }
    if (*type == ntohl(FILE_META)) {
      trans_file_t *received_file = (trans_file_t*)buf;
      file_size = ntohl(received_file->file_size); 
      printf("start to receive file: %s, file size: %u\n", 
                received_file->filename, file_size);
      //check if file exists
      pf = fopen(received_file->filename, "a");
      if (!pf) {
        fprintf(stderr, "cannot open file %s\n", received_file->filename);
      }
      continue;  
    } 
    
    if (*type == ntohl(FILE_DATA) && pf) {
      fwrite(buf + sizeof(uint32_t), sizeof(char), ret - sizeof(uint32_t), pf);
      received_size += (ret - sizeof(uint32_t));
      printf("file size: %u, received size %u\n", file_size, received_size); 
      if (received_size == file_size) {
        printf("all data received!\n");
        received_size = 0;
        file_size = 0;
        fclose(pf); 
      }
    }
    
  }
  pthread_exit(NULL); 
}

void *option_thread_start(void *arg) {
  option_thread_arg_t *data = (option_thread_arg_t*)arg;
  struct Handle *handle = data->handle;
  bool sendflag = data->sendflag;
  bool receiveflag = data->receiveflag;
  
  int option;
  while (true) {
    printf("Options: \n");
    if (sendflag) {
      printf("1. send message\n");
      printf("2. send file\n"); 
    }
    if (receiveflag) {
      printf("3. attach guid\n");
      printf("4. deattach guid\n");
    }
    printf("5. quit\n");
    printf("you choose: ");
    scanf("%d", &option);

    /*flush stdin buffer, otherwise getline() doesn't work*/
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    switch (option) {
    case 1:
      if (sendflag) {
        send_msg(handle);
      } else {
        fprintf(stderr, "unsupported option\n"); 
      }
      break;
    case 2:
      if (sendflag) {
        send_file(handle); 
      } else {
        fprintf(stderr, "unsupported option\n"); 
      }
      break; 
    case 3:
      if (receiveflag) {
        attach_guid(handle);
      } else {
        fprintf(stderr, "unsupported option\n"); 
      }
      break;
    case 4:
      if (receiveflag) {
        fprintf(stderr, "unsupported option\n"); 
      } else {
        printf("deattach: not implemented");
      }
      break;
    case 5:
      printf("quit!\n");
      //return 0;
      exit(0); 
    default:
      printf("unsupported option \n");
      break;
    }
  }
  pthread_exit(NULL); 
}

void gen_random_filename(char *s, const int len) {
  static const char alphanum[] =
                      "0123456789"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz";
  int i = 0; 
  for (i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  s[len] = '\0';
}

int* parseAttachedGUIDs (char *guids, int *guid_num) {
  int guid_cnt = 0;
  char *start_index = strchr(guids, '{');
  if (!start_index) {
    return NULL;
  }
  char *p = ++start_index;
  int len = 0;
  while (p < guids + strlen(guids)) {
    if ((*p == ',') || (*p == '}')) {
      ++len;
    }
    ++p;
  }
  //alloc memcpy base on num of ',' and '}'
  int *attached_guids = (int*)malloc(sizeof(int) * len);
  while (start_index < guids + strlen(guids)) {
    char *end_index;
    if ((end_index = strchr(start_index, ',')) ||
          (end_index = strchr(start_index, '}'))) {
      char tmp[32];
      memset(tmp, 0, 32);
      memcpy(tmp, start_index, end_index - start_index);
      if (atoi(tmp)) {
        attached_guids[guid_cnt++] = atoi(tmp);
      } else {
        fprintf(stderr, "cannot attach guid\n");
      }
      start_index = end_index + 1;
    } else {
      return NULL;
    }
  }
  *guid_num = guid_cnt;
  return attached_guids;
}
/*
char* process_file_name(char * filename) {
  DIR *dirptr;
  struct dirent *entry;
  dirptr = opendir(".");
  if (dirptr != NULL) {
    entry = readdir(dirptr);
    while (entry) {
      if (entry->d_type == DT_REG) {
        strcmp
      }
    }
  } else  {
    return NULL;
  }
}*/

