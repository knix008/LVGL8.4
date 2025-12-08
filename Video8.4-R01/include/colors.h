/**
 * @file ui_colors.h
 * @brief Centralized UI color definitions
 *
 * Defines all color constants used throughout the application.
 * Eliminates magic hex values and provides semantic color names.
 */

#ifndef UI_COLORS_H
#define UI_COLORS_H

// ============================================================================
// BASIC COLORS
// ============================================================================

#define UI_COLOR_BLACK          0x000000
#define UI_COLOR_WHITE          0xFFFFFF
#define UI_COLOR_GRAY           0x808080
#define UI_COLOR_LIGHT_GRAY     0x888888

// ============================================================================
// SEMANTIC COLORS - Background
// ============================================================================

#define UI_COLOR_BG_POPUP       0x000000    // Popup background (black)
#define UI_COLOR_BG_CONTAINER   0x000000    // Container background (black)
#define UI_COLOR_BG_PRESSED     0x808080    // Button pressed state (gray)

// ============================================================================
// SEMANTIC COLORS - Text
// ============================================================================

#define UI_COLOR_TEXT_PRIMARY   0xFFFFFF    // Primary text (white)
#define UI_COLOR_TEXT_SECONDARY 0x888888    // Secondary/instruction text (light gray)
#define UI_COLOR_TEXT_ERROR     0xFF6666    // Error text (light red)

// ============================================================================
// SEMANTIC COLORS - Buttons
// ============================================================================

#define UI_COLOR_BTN_SUCCESS    0x00AA00    // Success/Save button (green)
#define UI_COLOR_BTN_DANGER     0xAA0000    // Danger/Cancel button (red)
#define UI_COLOR_BTN_OK         0x00FF00    // OK button (bright green)

// ============================================================================
// SEMANTIC COLORS - Borders
// ============================================================================

#define UI_COLOR_BORDER_ERROR   0xFF0000    // Error border (red)
#define UI_COLOR_BORDER_SUCCESS 0x00FF00    // Success border (green)
#define UI_COLOR_BORDER_NORMAL  0xFFFFFF    // Normal border (white)

// ============================================================================
// SEMANTIC COLORS - Switches/Toggles
// ============================================================================

#define UI_COLOR_SWITCH_IPV4    0x00FF00    // IPv4 mode (green)
#define UI_COLOR_SWITCH_IPV6    0xFF0000    // IPv6 mode (red)

// ============================================================================
// SEMANTIC COLORS - Calendar/Special
// ============================================================================

#define UI_COLOR_CALENDAR_DEFAULT   0xFF9800    // Calendar default (orange)
#define UI_COLOR_CALENDAR_SELECTED  0xBF360C    // Calendar selected (dark orange)

// ============================================================================
// SEMANTIC COLORS - Shadows
// ============================================================================

#define UI_COLOR_SHADOW         0x000000    // Shadow color (black)

#endif // UI_COLORS_H
