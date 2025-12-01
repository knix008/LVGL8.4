#include "../include/face.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"

// ============================================================================
// FACE SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_face_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Create face image in the center
    lv_obj_t *face_img = lv_img_create(content);
    lv_img_set_src(face_img, IMG_FACE);
    lv_obj_align(face_img, LV_ALIGN_CENTER, 0, -VERTICAL_OFFSET_MEDIUM);

    // Create label below the image
    lv_obj_t *face_label = lv_label_create(content);
    lv_label_set_text(face_label, "Face Screen");
    apply_label_style(face_label);
    lv_obj_align(face_label, LV_ALIGN_CENTER, 0, VERTICAL_OFFSET_SMALL);

    // Create description text
    lv_obj_t *desc_label = lv_label_create(content);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_label, SCREEN_WIDTH - CONTENT_WIDTH_LARGE_PADDING);
    apply_label_style(desc_label);
    lv_label_set_text(desc_label,
        "This is the Face screen.\n\n"
        "You can customize this screen\n"
        "to display face-related content."
    );
    lv_obj_align(desc_label, LV_ALIGN_CENTER, 0, VERTICAL_OFFSET_LARGE);

    return content;
}

// ============================================================================
// FACE SCREEN CREATION
// ============================================================================

/**
 * Creates the face screen with title bar, content area, and status bar.
 * Displays face icon and descriptive text.
 */
void create_face_screen(void) {
    lv_obj_t *face_screen = create_screen_base(SCREEN_FACE);

    create_standard_title_bar(face_screen, SCREEN_FACE);
    create_face_content(face_screen);
    create_standard_status_bar(face_screen);

    finalize_screen(face_screen, SCREEN_FACE);
}
