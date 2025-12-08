#ifndef NETWORK_IP_CONFIG_H
#define NETWORK_IP_CONFIG_H

#include <stdbool.h>
#include "network.h"

// IP address configuration constants
#define IPV4_MAX_LENGTH 15  // xxx.xxx.xxx.xxx (max 15 chars + null terminator)
#define IPV6_MAX_LENGTH 39  // xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx (max 39 chars + null terminator)
// Note: IP_CONFIG_FILE is defined in config.h

/**
 * Get pointer to the current IP configuration
 * @return Pointer to ip_config_t structure
 */
ip_config_t* get_ip_config(void);

/**
 * Validate IPv4 address format (xxx.xxx.xxx.xxx)
 * @param ip IPv4 address string to validate
 * @return true if valid, false otherwise
 */
bool is_valid_ipv4(const char *ip);

/**
 * Validate IPv6 address format (simplified - checks basic structure)
 * @param ip IPv6 address string to validate
 * @return true if valid, false otherwise
 */
bool is_valid_ipv6(const char *ip);

/**
 * Save IP configuration to persistent storage
 * @return 0 on success, -1 on failure
 */
int save_ip_config(void);

/**
 * Load IP configuration from persistent storage
 * @return 0 on success, -1 on failure (uses defaults)
 */
int load_ip_config(void);

/**
 * Initialize IP configuration with default values
 */
void init_ip_config(void);

#endif // NETWORK_IP_CONFIG_H
