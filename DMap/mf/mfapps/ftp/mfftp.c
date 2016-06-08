#include"mfftp.h"

void usage() {
  printf("Usage:\n");
  printf("\t-s\t work as ftp server\n");
  printf("\t-c\t work as ftp client\n");
  printf("\t-m\t my guid\n");
  printf("\t-d\t dst guid\n");
  printf("\t-l|-g|-p\t -list|-get|-put, get and put require filename\n");
  printf("\t-a\t attach a file to a guid\n");
  printf("\t-o\t option mode\n");
  printf("\t-D\t debug mode on\n"); 
  printf("Examples:\n");
  printf("\t./mfftp -s -m 5\n");
  printf("\t./mfftp -c -m 1 -d 5 --list\n"); 
  printf("\t./mfftp -c -m 1 -d 5 --put filename\n");
}

int main (int argc , char *argv[]) {
  bool server_flag = false;
  bool client_flag = false;
  bool optmode_flag = false;
  
  debug = false;

  //uint32_t my_guid = 0;
  uint32_t dst_guid = 0;
  uint32_t attached_guid = 0;
  char *file_path = NULL;
  uint32_t command = 0;   
  
  uint32_t svc_opt = 0; 
  
  int c = 0;
  //int digit_optind = 0;
  if (argc == 1) {
    usage(); 
    exit(EXIT_FAILURE); 
  }
  while (1) {
    static struct option long_options[] = {
      {"list",   no_argument,       0, 'l'},
      {"get",    required_argument, 0, 'g'},
      {"put",    required_argument, 0, 'p'},
      {"attach", required_argument, 0, 'a'},
      {0,        0,                 0,  0 }
    }; 
    int option_index = 0;
    c = getopt_long(argc, argv, "scolDg:p:m:d:a:S:", long_options, &option_index);
    if (c == -1) {
      break; 
    } 
    switch (c) {
    case 0:
      printf("debug 1*******************\n"); 
      break;
    case 's':
      server_flag = true;
      break;
    case 'c':
      client_flag = true;
      break;
    case 'm':
      my_guid = atoi(optarg);
      break;
    case 'd':
      dst_guid = atoi(optarg);
      break; 
    case 'S':
      svc_opt = atoi(optarg);
      break;  
    case 'l':
      if (command) {
        usage(); 
      }
      command = LIST;
      break;
    case 'p':
      if (command) {
        usage(); 
      }
      command = PUT;
      file_path = optarg;
      break; 
    case 'g':
      if (command) {
        usage();
      }
      command = GET;
      file_path = optarg; 
      break;
    case 'o':
      optmode_flag = true; 
      break;
    case 'D':
      debug = true;
      break;
    case '?':
      usage(); 
      break;
    default:
      break; 
    }
  }

  if (optind < argc) {
    fprintf(stderr, "non-option ARGV-elements: ");
    while (optind < argc)
      fprintf(stderr, "%s ", argv[optind++]);
    fprintf(stderr, "\n");
  }

  /*my_guid is required*/
  if (my_guid == 0) {
    fprintf(stderr, "Error: my_guid is requried\n");
    exit(EXIT_FAILURE); 
  }
  
  if ((!server_flag) && (!client_flag)) {
    fprintf(stdout, "No working mode defined, run as a server\n");
    server_flag = true; 
  } 
  
  if (server_flag && !client_flag && command) {
    printf("Omit command!\n"); 
  }
  
  if ((!command) && (!optmode_flag) && client_flag) {
    fprintf(stderr, "need a command: list/get/put\n");
    usage();
    exit(EXIT_FAILURE); 
  } 
  
  if (command && !dst_guid && !optmode_flag && client_flag) {
    fprintf(stderr, "Use -d to specify server's guid\n");
    exit(EXIT_FAILURE); 
  }

  struct Handle handle;
  int32_t ret = mfopen(&handle, "basic\0", 0, my_guid);
  if (ret) {
    fprintf(stderr, "mfopen error\n");
    exit(EXIT_FAILURE); 
  }
  
  pthread_t thread_id[2];
  int thread_cnt = 0;
  if (server_flag) {

    ret = pthread_create(&thread_id[thread_cnt], NULL, receiving_thread, 
                         &handle);
    if (ret) {
      fprintf(stderr, "cannot create receive thread\n");
      exit(EXIT_FAILURE); 
    }
    ++thread_cnt;
  }

  if (optmode_flag) {
    ret = pthread_create(&thread_id[thread_cnt], NULL, optmode_thread,
                         &handle);
    if (ret) {
      fprintf(stderr, "MFFTP_LOG: ERROR: Cannot create option mod thread\n"); 
    }
    ++thread_cnt; 
  }

  while(!receiving) {
  }
  switch (command) {
    case LIST:
      if (debug) {
        fprintf(stdout, "MFFTP_LOG: LIST\n"); 
      }
      send_list_msg(&handle, my_guid, dst_guid, svc_opt); 
      break; 
    case GET:
      if (debug) {
        fprintf(stdout, "MFFTP_LOG: GET file: %s\n", file_path); 
      }
      send_get_msg(&handle, my_guid, dst_guid, file_path); 
      break; 
    case PUT: 
      if (debug) {
        fprintf(stdout, "MFFTP_LOG: PUT file path %s\n", file_path);
      }
      send_put_msg(&handle, my_guid, dst_guid, file_path);
      send_file(&handle, dst_guid, file_path, svc_opt); 
      break; 
    default:
      fprintf(stderr, "unknown command\n"); 
      break;
  }
  
  int i = 0; 
  for (; i != thread_cnt; ++i) {
    pthread_join(thread_id[i], NULL); 
  }
  return 0; 
}
