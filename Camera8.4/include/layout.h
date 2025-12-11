/**
 * @file ui_layout.h
 * @brief Centralized UI layout constants
 *
 * Defines layout dimensions, spacing, and positioning constants.
 * Eliminates magic numbers for sizes and positions.
 */

#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

// ============================================================================
// POPUP DIMENSIONS
// ============================================================================

#define UI_POPUP_MESSAGE_BOX_WIDTH      265
#define UI_POPUP_IP_CONTAINER_WIDTH     320
#define UI_POPUP_IP_CONTAINER_HEIGHT    620

// ============================================================================
// CONTAINER DIMENSIONS
// ============================================================================

#define UI_CONTAINER_TOGGLE_WIDTH       260
#define UI_CONTAINER_TOGGLE_HEIGHT      50

#define UI_CONTAINER_IP_DISPLAY_WIDTH   300
#define UI_CONTAINER_IP_DISPLAY_HEIGHT  60

#define UI_CONTAINER_CONTROL_BUTTON_WIDTH   130
#define UI_CONTAINER_CONTROL_BUTTON_HEIGHT  45

// ============================================================================
// WIDGET DIMENSIONS
// ============================================================================

#define UI_SWITCH_WIDTH                 60
#define UI_SWITCH_HEIGHT                30

#define UI_INPUT_DISPLAY_WIDTH          280

// ============================================================================
// KEYPAD DIMENSIONS (Network IP Popup)
// ============================================================================

#define UI_KEYPAD_BUTTON_SIZE           60
#define UI_KEYPAD_BUTTON_SPACING        10

// ============================================================================
// KOREAN INPUT KEYBOARD DIMENSIONS (Number Screen)
// ============================================================================

#define UI_KOREAN_KEYBOARD_CONTAINER_WIDTH      320
#define UI_KOREAN_KEYBOARD_CONTAINER_HEIGHT     580

#define UI_KOREAN_KEYBOARD_BUTTON_WIDTH         80
#define UI_KOREAN_KEYBOARD_BUTTON_HEIGHT        60
#define UI_KOREAN_KEYBOARD_BUTTON_SPACING       10

#define UI_KOREAN_TEXT_DISPLAY_WIDTH            280
#define UI_KOREAN_TEXT_DISPLAY_HEIGHT           70

#define UI_KOREAN_CTRL_BUTTON_WIDTH             90
#define UI_KOREAN_CTRL_BUTTON_HEIGHT            50

// ============================================================================
// POSITIONING
// ============================================================================

#define UI_POS_ORIGIN_X                 0
#define UI_POS_ORIGIN_Y                 0

#define UI_OFFSET_SMALL                 3

#endif // UI_LAYOUT_H
