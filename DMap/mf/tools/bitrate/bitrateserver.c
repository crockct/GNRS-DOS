#include "bitrateserver.h"

void usage() {
  fprintf(stdout, "usage: ./bitrate -i interface -p port#\n");
  fprintf(stdout, "-i	interface(required), for example eth0, wlan1, wmx0\n");
  fprintf(stdout, "-s	server port number(optional), default is %u\n", 
          DEFAULT_SERVER_PORT);
  fprintf(stdout, "-c	client port number(optional), default is %u\n",
          DEFAULT_CLIENT_PORT); 
  fprintf(stdout, "-d	enable debug mode\n"); 
}

int main(int argc, char* argv[]){
  extern char *optarg;
  char *dev = 0;
  int server_port_num = DEFAULT_SERVER_PORT;
  
  int c = 0;
  while ((c = getopt(argc, argv, "di:s:")) != -1) {
    switch(c) {
    case 'i': 
      dev = optarg;
      break;
    case 's':
      server_port_num = atoi(optarg);
      break;
    //case 'c':
    //  client_port_num = atoi(optarg);
    //  break; 
    case 'd':
      debug = true;
      break; 
    case '?':
      usage();
      exit(EXIT_FAILURE); 
    default:
      abort(); 
      break; 
    }
    if (optind > argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
    }
  }
 
  if (optind < argc) {
    fprintf(stderr, "non-option ARGV-elements: ");
    while (optind < argc) {
      printf ("%s ", argv[optind++]);
    }
    printf ("\n");
    usage(); 
    exit(EXIT_FAILURE);  
  }

  if (dev == 0) {
    fprintf(stderr, "interface is required\n");
    usage();
    exit(EXIT_FAILURE); 
  }
  
  int sock ;   

  //create a udp/ip socket
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    fprintf(stderr, "ERROR: cannot create socket \n"); 
    exit(EXIT_FAILURE); 
  }

  //bind the socket to an udp port
  struct sockaddr_in my_addr; 
  memset((void *)&my_addr, 0 , sizeof(my_addr)); 
  my_addr.sin_family = AF_INET; 
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr.sin_port = htons(server_port_num); 
  
  if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
    fprintf(stderr, "ERROR: socket bind failed! \n");
    exit(EXIT_FAILURE); 
  }
  
  char buf[BUF_SIZE]; 
  struct sockaddr_in client_addr; 
  socklen_t addrlen = sizeof(client_addr); 

  while (1) {
    if (debug) {
      fprintf(stdout, "device %s, port %d is waiting for bitrate req\n", 
              dev, server_port_num);
    }

    //waiting for msg 
    int recvlen = recvfrom(sock, buf, MAC_BUF_SIZE, 0, 
                           (struct sockaddr *)&client_addr, &addrlen);
    if (recvlen > 0) {
    }
    
    bitrate_local_lookup_t *lookup = (bitrate_local_lookup_t*)buf;
    int lookup_id = ntohl(lookup->lookup_id);
      
    char mac_str[MAC_BUF_SIZE];

    //convert mac bytes to string with format 00:00:00:00:00:00
    macToString(lookup->mac, mac_str);

    if (!isValidMacAddr(mac_str)) {
      fprintf(stderr, "Error: format of mac addr is wrong!"); 
    }
 
    if (debug) {
      char clientIp[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client_addr.sin_addr), clientIp, INET_ADDRSTRLEN);
      fprintf(stdout, "received lookup id: %u for mac: %s from %s:%u\n", 
              lookup_id, mac_str, clientIp, ntohs(client_addr.sin_port));
    }

    const char* bitrate; 
    if (!strcmp(dev,"wlan0") || !strcmp(dev,"wlan1")) {
      bitrate = get_wifi_bitrate(dev, mac_str); 
    } else if (!strcmp(dev, "wmx0")) {
      bitrate = get_wimax_bitrate(mac_str);
    } else {
      fprintf(stderr, "Error: unsupported device\n");
      exit(EXIT_FAILURE); 
    }

    //convert to UTC time;   
    time_t now = time(NULL);
    struct tm* gmtm = gmtime(&now);
   
    if (gmtm != NULL && debug) {
      fprintf(stdout,  "UTC time: %s", asctime(gmtm)); 
    } 

    if (gmtm == NULL) {
      fprintf(stderr, "Error: Failed to convert get UTC time\n"); 
    }

    if (debug) {
      fprintf(stdout, "bitrate of %s is %s mbps\n", mac_str, bitrate); 
    }
    
    //fill resp 
    bitrate_local_resp_t resp;
    resp.lookup_id = htonl(lookup_id);
    resp.bitrate = htonl(atoi(bitrate));
    resp.valid_sec = htonl(30); 
    
    //send bit rate back to client
    if (sendto(sock, &resp, sizeof(bitrate_local_resp_t), 0, 
                 (struct sockaddr *)&client_addr, addrlen) < 0) {
      fprintf(stderr, "Error: message reply to client failed\n");
    }
    if (debug) {
      fprintf(stdout, "sent resp to port: %u\n", ntohs(client_addr.sin_port));
    }
  }  
}


const char* get_wimax_bitrate(char *mac_addr){
  FILE *fp;
  char output[65535];
  const char *cmd1 = "wget -qO- \"10.14.0.10:5054/result/queryDatabase?"
                     "expID=wimax_clients01&query=select%20mac,mcsdlmod"
                     "%20from%20wimax_clients01_client%20where%20mac='";

  const char *cmd2 ="'%20order%20by%20rowid%20desc%20limit%201&format=raw\"";

  char *command = (char*)malloc(strlen(cmd1) + strlen(mac_addr)+strlen(cmd2));
  strcpy(command, cmd1);
  strcat(command, mac_addr);
  strcat(command, cmd2); 
  
  if (debug) {
    fprintf(stdout, "%s\n", command); 
  }

  fp = popen(command,"r");
  if (fp == NULL) {
    fprintf(stderr, "Error: getBitRate(): Failed to run command\n" );
  }
  
  int i = 0;
  int ch = 0; 
  while ((ch = fgetc(fp)) != EOF) {
    output[i] = ch;
    i++;
  }
  
  //if result of query is empty, return "0"
  if(i == 0) {
    return "0"; 
  }
  output[i] = '\0';
  fprintf(stderr, "%s\n", output); 
  int addrLength = 18;
  char * macAddr = (char*)malloc(addrLength);
  int index = 2;
  int j = 0; 
  for (; j != 17; ++j) {
     macAddr[j] = output[index];
     index++;
  }
  macAddr[17] = '\0';
  
  char code[20];
  int k = 0; 
  while(output[++index] != '\0') {
    code[k] = output[index]; 
    k++;
  }
  code[k-1]='\0';
  
  pclose(fp);
  return codeToBitrate(code);
} 

const char* get_wifi_bitrate(const char* interface, char *mac_addr){
  if (!strcmp(mac_addr, "00:00:00:00:00:00")) {
    return "0"; 
  }
  FILE *fp;
  char output[65535];
  char command[256];
  sprintf(command, "iw %s station dump", interface); 
  fp = popen(command,"r");
  if (fp == NULL) {
    fprintf(stderr, "Error: getBitRate(): Failed to run command\n" );
    exit(EXIT_FAILURE);
  }

  int i = 0;
  int ch = 0; 
  while ((ch = fgetc(fp)) != EOF) {
    output[i] = ch; 
    i++;
  }
  
  /*if result of query is empty, return "0"*/
  if(i == 0) {
    return "0";
  }

  output[i] = '\0';
  //fprintf(stderr, "%s\n", output); 
  //char * macAddr;
  char * bitrate;

  char station[256]; 
  sprintf(station, "Station %s", mac_addr);
  bool start_flag = false; 
  for (i = 0; i < (int)strlen(output); ++i) { 
    if (matchText(output, i, station) == 0) {
      start_flag = true; 
    }
    if (start_flag && (matchText(output, i, "tx bitrate") == 0)) {
      bitrate = extractBitRate(output, i);
      if (bitrate == NULL) {
        pclose(fp); 
        fprintf(stderr, "Error:cannot get bit rate value"); 
        return "0";
      } else {
        pclose(fp);
        return bitrate; 
      }
    }
  }
  return "0"; 
}



const char* codeToBitrate(char* code){
  if(strcmp(code,"64QAM 5/6") == 0) {
    return "17";
  }
  else {
    return "0";
  }
}

int matchText(char *buffer, u_int index, const char *text) {
  unsigned int N = strlen(text);
  if((index + N) >= strlen(buffer)) {
    return -1;
  }
  unsigned int i;
  for(i = 0; i < N; i++) {
    if(buffer[index + i] != text[i]) {
      return -1;
    }
  }
  return 0;
}

char * getMacAddr(char *buffer, int index){
  int start_index = index + 8;
  char *mac_addr = (char*)malloc(18);
  int i = 0; 
  for(; i != 17; ++i) {
    mac_addr[i] = buffer[start_index];
    start_index++;
  }
  mac_addr[17] = '\0';
  return mac_addr;
}

char* extractBitRate(char *buffer, int index){
  int start_index = index + 10;
  char *bitrate_buffer = (char*)malloc(7);
  if( buffer[start_index] != ':'){
    fprintf(stderr, "ERROR: no ':'"); 
    return NULL;
  }
  start_index++;
  int i = 0;
  while(index < (int)strlen(buffer) && buffer[start_index] != '.') {
    while(buffer[start_index] >= '0' && buffer[start_index] <= '9') {

      bitrate_buffer[i] = buffer[start_index];
      start_index++;
      i++;
    }
    if(buffer[start_index] == '.') break;
    start_index++;
  }
  bitrate_buffer[i] = '\0';

  return bitrate_buffer;
}

