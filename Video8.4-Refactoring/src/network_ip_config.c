#include "../include/network_ip_config.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// MODULE STATE
// ============================================================================

static ip_config_t ip_config = {
    .type = IP_TYPE_IPV4,
    .ipv4 = "192.168.1.100",
    .ipv6 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
};

// ============================================================================
// PUBLIC API - CONFIGURATION ACCESS
// ============================================================================

ip_config_t* get_ip_config(void) {
    return &ip_config;
}

void init_ip_config(void) {
    ip_config.type = IP_TYPE_IPV4;
    strncpy(ip_config.ipv4, "192.168.1.100", sizeof(ip_config.ipv4) - 1);
    ip_config.ipv4[sizeof(ip_config.ipv4) - 1] = '\0';
    strncpy(ip_config.ipv6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", sizeof(ip_config.ipv6) - 1);
    ip_config.ipv6[sizeof(ip_config.ipv6) - 1] = '\0';
}

// ============================================================================
// IP VALIDATION FUNCTIONS
// ============================================================================

/**
 * Validate IPv4 address format (xxx.xxx.xxx.xxx)
 */
bool is_valid_ipv4(const char *ip) {
    if (!ip) return false;

    int segments = 0;
    int value = -1;
    bool has_digit = false;

    for (const char *p = ip; *p != '\0'; p++) {
        if (isdigit(*p)) {
            if (value == -1) value = 0;
            value = value * 10 + (*p - '0');
            has_digit = true;
            if (value > 255) return false;
        } else if (*p == '.') {
            if (!has_digit || value < 0) return false;
            segments++;
            value = -1;
            has_digit = false;
        } else {
            return false;
        }
    }

    if (has_digit && value >= 0) segments++;
    return segments == 4;
}

/**
 * Validate IPv6 address format (simplified - checks basic structure)
 */
bool is_valid_ipv6(const char *ip) {
    if (!ip) return false;

    int segments = 0;
    int hex_digits = 0;
    bool has_double_colon = false;
    const char *p = ip;

    while (*p != '\0') {
        if (isxdigit(*p)) {
            hex_digits++;
            if (hex_digits > 4) return false;
        } else if (*p == ':') {
            if (hex_digits > 0) {
                segments++;
                hex_digits = 0;
            }
            if (*(p + 1) == ':') {
                if (has_double_colon) return false;  // Only one :: allowed
                has_double_colon = true;
                p++;
            }
        } else {
            return false;
        }
        p++;
    }

    if (hex_digits > 0) segments++;

    // IPv6 should have 8 segments, or less if :: is used
    if (has_double_colon) {
        return segments <= 7;
    } else {
        return segments == 8;
    }
}

// ============================================================================
// CONFIGURATION PERSISTENCE
// ============================================================================

int save_ip_config(void) {
    FILE *fp = fopen(IP_CONFIG_FILE, "w");
    if (!fp) return -1;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"type\": \"%s\",\n", ip_config.type == IP_TYPE_IPV4 ? "ipv4" : "ipv6");
    fprintf(fp, "  \"ipv4\": \"%s\",\n", ip_config.ipv4);
    fprintf(fp, "  \"ipv6\": \"%s\"\n", ip_config.ipv6);
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}

int load_ip_config(void) {
    // Read IP config file
    FILE *fp = fopen(IP_CONFIG_FILE, "r");
    if (!fp) {
        // Use defaults
        init_ip_config();
        return 0;
    }

    // Use static buffer instead of malloc for memory safety
    static char content[512];  // IP config file is small (type + 2 IP addresses)

    // Read file content
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Ensure file size doesn't exceed buffer
    if (file_size >= (long)sizeof(content)) {
        fclose(fp);
        // Use defaults if file is too large
        init_ip_config();
        return -1;
    }

    size_t bytes_read = fread(content, 1, file_size, fp);
    content[bytes_read] = '\0';
    fclose(fp);

    // Parse type
    if (strstr(content, "\"type\"")) {
        if (strstr(content, "ipv4")) {
            ip_config.type = IP_TYPE_IPV4;
        } else if (strstr(content, "ipv6")) {
            ip_config.type = IP_TYPE_IPV6;
        }
    }

    // Parse ipv4 address
    const char* ipv4_start = strstr(content, "\"ipv4\":");
    if (ipv4_start) {
        // Skip past "ipv4":
        const char* value_start = ipv4_start + 7; // length of "ipv4":
        const char* quote_start = strchr(value_start, '"');
        if (quote_start) {
            quote_start++;
            const char* quote_end = strchr(quote_start, '"');
            if (quote_end) {
                int len = quote_end - quote_start;
                if (len < 16) {
                    strncpy(ip_config.ipv4, quote_start, len);
                    ip_config.ipv4[len] = '\0';
                }
            }
        }
    }

    // Parse ipv6 address
    const char* ipv6_start = strstr(content, "\"ipv6\":");
    if (ipv6_start) {
        // Skip past "ipv6":
        const char* value_start = ipv6_start + 7; // length of "ipv6":
        const char* quote_start = strchr(value_start, '"');
        if (quote_start) {
            quote_start++;
            const char* quote_end = strchr(quote_start, '"');
            if (quote_end) {
                int len = quote_end - quote_start;
                if (len < 40) {
                    strncpy(ip_config.ipv6, quote_start, len);
                    ip_config.ipv6[len] = '\0';
                }
            }
        }
    }

    return 0;
}
