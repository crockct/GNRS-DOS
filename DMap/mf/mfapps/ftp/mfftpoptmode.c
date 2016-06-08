#include "mfftpoptmode.h"

void *optmode_thread(void *arg) {
  struct Handle *handle = (struct Handle*)arg;
  int option; 
  while(true) {
    fprintf(stdout, "*******Options********\n");
    fprintf(stdout, "1 LIST\n");
    fprintf(stdout, "2 GET\n");
    fprintf(stdout, "3 PUT\n");
    fprintf(stdout, "**********************\n"); 
    scanf("%d", &option);
    
    /*flush stdin buffer, otherwise getline() doesn't work*/
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    switch (option) {
     case 1:
      list_opt(handle); 
      break; 
     case 2:
      get_opt(handle); 
      break;
     case 3:
      put_opt(handle); 
      break;
     case 4:
      break;
     default:
      fprintf(stderr, "Unsupported Option\n"); 
      break; 
    }
  }
}

int list_opt(struct Handle *handle) {
  fprintf(stdout, "You chose LIST\n"); 
  int dst_guid = 0;
  do {
    fprintf(stdout, "Input Dst GUID: ");
    scanf("%d", &dst_guid); 
    /*flush stdin buffer, otherwise getline() doesn't work*/
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (!dst_guid) {
      fprintf(stdout, "Dst GUID cannot be 0\n"); 
    }
  } while (dst_guid == 0); 
  send_list_msg(handle, my_guid, dst_guid, 0);
  return 0; 
}

int get_opt(struct Handle *handle) {
  fprintf(stdout, "You chose GET\n"); 
  int dst_guid = 0;
  do {
    fprintf(stdout, "Input Dst GUID: ");
    scanf("%d", &dst_guid);
    /*flush stdin buffer, otherwise getline() doesn't work*/
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (!dst_guid) {
      fprintf(stdout, "Dst GUID cannot be 0\n");
    }
  } while (dst_guid == 0);
  char filename[MAX_FILENAME_LEN]; 
  fprintf(stdout, "please input filename: ");
  scanf("%s", filename);
  /*flush stdin buffer*/
  int c;
  while ((c = getchar()) != '\n' && c != EOF);

  send_get_msg(handle, my_guid, dst_guid, filename);
  return 0; 
}

int put_opt(struct Handle *handle) {
  fprintf(stdout, "You chose PUT\n"); 
  int dst_guid = 0;
  do {
    fprintf(stdout, "Input Dst GUID: ");
    scanf("%d", &dst_guid);
    /*flush stdin buffer, otherwise getline() doesn't work*/
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (!dst_guid) {
      fprintf(stdout, "Dst GUID cannot be 0\n");
    }
  } while (dst_guid == 0);
  char filepath[1000];
  fprintf(stdout, "please input filepath: ");
  scanf("%s", filepath);
  /*flush stdin buffer*/
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
  fprintf(stdout, "service option (0-default, 1-multihoming, 2-anycast)"); 
  int svc_opt = 0;
  scanf("%d", &svc_opt); 
  while ((c = getchar()) != '\n' && c != EOF);
 
  send_put_msg(handle, my_guid, dst_guid, filepath);
  send_file(handle, dst_guid, filepath, svc_opt);
  
  return 0; 
}
