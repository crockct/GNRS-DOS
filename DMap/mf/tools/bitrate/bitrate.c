#include "bitrate.h"

void macToString(char *mac, char *mac_str) {
  int i;
  for (i = 0; i < 5; ++i, mac_str += 3) {
    sprintf(mac_str, "%02x:", mac[i] & 0xff);
  }
  sprintf(mac_str, "%hhx", mac[i] & 0xff);
}

//check whether mac addr is valid
bool isValidMacAddr(const char* mac) {
    int i = 0;
    int s = 0;

    while (*mac) {
       if (isxdigit(*mac)) {
          i++;
       }
       else if (*mac == ':' || *mac == '-') {
          if (i == 0 || i / 2 - 1 != s)
            break;
          ++s;
       }
       else {
           s = -1;
       }
       ++mac;
    }
    return (i == 12 && (s == 5 || s == 0))? true : false;
}

//check whether ip addr is valid
bool isValidIpAddr(const char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}
