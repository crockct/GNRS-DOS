#include "bitratetest.h"

void print_usage() {
  fprintf(stdout, "usage:\n");
  fprintf(stdout, "-m	mac address(required)\n");
  fprintf(stdout, "-a	ip address(optional), default is localhost(127.0.0.1)\n");
  fprintf(stdout, "-s	server port number(optional), default is %u\n", 
          DEFAULT_SERVER_PORT);
  fprintf(stdout, "-c	client port number(optional), default is %u\n",
          DEFAULT_CLIENT_PORT); 
  fprintf(stdout, "example:\n");
  fprintf(stdout, "./bitratetest -a 127.0.0.1 -s 6060 -m 00:1d:e1:3b:5d:d6\n"); 
}

int main(int argc, char **argv) {
  extern char *optarg;  
  char *mac_str = NULL;

  char mac[MAC_BYTE_SIZE];
  memset(mac, 0, MAC_BYTE_SIZE); 
  
  char ip[IP_BUF_SIZE];
  memset(ip, 0, IP_BUF_SIZE);
  memcpy(ip, kLocalHost, strlen(kLocalHost));
  
  int server_port_num = DEFAULT_SERVER_PORT;
  int client_port_num = DEFAULT_CLIENT_PORT; 
  
  int c = 0;
  while ((c = getopt(argc, argv, "m:a:s:c:")) != -1) {
    switch(c) {
    case 'm':
      if (!isValidMacAddr(optarg)) {
        fprintf(stderr, "Error: mac address is not valid!\n");
        exit(EXIT_FAILURE); 
      }
      //convert from string to bytes
      sscanf(optarg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
             &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
      mac_str = (char *)malloc(MAC_BYTE_SIZE); 
      strcpy(mac_str, optarg);
      break;
    case 'a':
      if (!isValidIpAddr(optarg)) {  
        fprintf(stderr, "Error: ip address format is not valid!\n");
        exit(EXIT_FAILURE); 
      }
      strcpy(ip, optarg); 
      break;
    case 's':
      server_port_num = atoi(optarg); 
      break;
    case 'c':
      client_port_num = atoi(optarg);
      break; 
    case '?':
      print_usage();
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
    print_usage();
    exit(EXIT_FAILURE);
  }
  
  if (!mac_str) {
    fprintf(stderr, "Error: mac address is required!\n");
    print_usage();
    exit(EXIT_FAILURE);
  }
  
  //make lookup
  bitrate_local_lookup_t lookup;
  lookup.lookup_id = 0;
  memcpy(lookup.mac, mac, 6); 
  
  //create a udp/ip socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    fprintf(stderr, "Error: cannot create socket for query bitrate!\n");
    exit(EXIT_FAILURE);
  }

  //bind the socket to an udp port
  struct sockaddr_in my_addr;
  memset((void *)&my_addr, 0 , sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr.sin_port = htons(client_port_num);

  if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
    fprintf(stderr, "Error: socket bind failed!\n");
    exit(EXIT_FAILURE); 
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family=AF_INET;
  server_addr.sin_port = htons(server_port_num);
  server_addr.sin_addr.s_addr=inet_addr(ip);
  
  if (sendto(sock, &lookup, sizeof(bitrate_local_lookup_t), 0, 
             (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    fprintf(stderr, "Error: send message to server failed!\n");
    exit(EXIT_FAILURE); 
  }

  fprintf(stdout, "sent bitrate lookup to %s:%u for mac: %s from port: %u\n ", 
          ip, server_port_num, mac_str, client_port_num);
  
  //TODO: add a timeout, return 0 if times out. otherwise, it get stuck here.
  char buf[BUF_SIZE];  
  int32_t recvlen = recvfrom(sock, buf, BUF_SIZE, 0, 0, 0);
  if(recvlen > 0) {
    //buf[recvlen] = '\0';
  }
  bitrate_local_resp_t *resp = (bitrate_local_resp_t*)buf;
  //int32_t ret = atoi(buf);
  fprintf(stdout, "mac: %s, bitrate: %u mbps, valid second: %u\n", 
          mac_str, ntohl(resp->bitrate), ntohl(resp->valid_sec)); 
  return  0;
}

