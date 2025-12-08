// ============================================================================
// NETWORK MODULE - MAIN COORDINATOR
// ============================================================================
// This file serves as the main coordinator for the network module.
// Actual implementation is split across focused modules:
// - network_ip_config.c: IP address validation and configuration storage
// - network_input.c: User input handling and cursor management
// - network_ui.c: UI creation and display logic
//
// The public API functions (create_network_screen, save_ip_config, load_ip_config)
// are implemented directly in their respective modules and exported via headers.
// This coordinator file exists to document the module structure.
// ============================================================================

/*
 * Module Architecture:
 *
 * network.h - Main public API header
 *   ├── network_ip_config.h/c - IP configuration and validation
 *   ├── network_input.h/c - Input handling and event callbacks
 *   └── network_ui.h/c - UI creation and display
 *
 * Dependencies:
 *   - network_ui.c includes network_ip_config.h and network_input.h
 *   - network_input.c includes network_ip_config.h and network_ui.h
 *   - network_ip_config.c is standalone
 */
