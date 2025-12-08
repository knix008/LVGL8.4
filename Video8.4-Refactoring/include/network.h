#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>

// ============================================================================
// CORE TYPES
// ============================================================================

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

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * Create the network configuration screen
 * This is the main entry point for the network module
 * Implemented in: network_ui.c
 */
void create_network_screen(void);

/**
 * Save IP configuration to persistent storage
 * Implemented in: network_ip_config.c
 * @return 0 on success, -1 on failure
 */
int save_ip_config(void);

/**
 * Load IP configuration from persistent storage
 * Implemented in: network_ip_config.c
 * @return 0 on success, -1 on failure (uses defaults)
 */
int load_ip_config(void);

#endif
