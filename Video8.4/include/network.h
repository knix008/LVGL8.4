#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>

// IP address types
typedef enum {
    IP_TYPE_IPV4 = 0,
    IP_TYPE_IPV6 = 1
} ip_type_t;

// IP configuration structure
typedef struct {
    ip_type_t type;
    char ipv4[16];  // xxx.xxx.xxx.xxx\0
    char ipv6[40];  // xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx\0
} ip_config_t;

void create_network_screen(void);
int save_ip_config(void);
int load_ip_config(void);

#endif
