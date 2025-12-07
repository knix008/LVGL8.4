#include "../include/slideshow.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// ============================================================================
// SLIDESHOW CONFIGURATION
// ============================================================================

#define MAX_SLIDESHOW_IMAGES 20
#define SLIDESHOW_INTERVAL 3000  // 3 seconds in milliseconds
#define IMAGES_DIR_PATH "images"

typedef struct {
    char image_paths[MAX_SLIDESHOW_IMAGES][512];
    int image_count;
    int current_index;
    lv_obj_t *slideshow_img;
    lv_timer_t *slideshow_timer;
} SlideshowState;

static SlideshowState slideshow_state = {0};

// ============================================================================
// SLIDESHOW HELPER FUNCTIONS
// ============================================================================

static int is_progressive_jpeg(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return 0;  // Can't open, assume not progressive
    }

    unsigned char byte1, byte2;
    int is_progressive = 0;

    // Read JPEG file and look for progressive marker (0xFFC2)
    // JPEG markers: 0xFFD8 (SOI), 0xFFC0 (SOF0-baseline), 0xFFC2 (SOF2-progressive)
    while (fread(&byte1, 1, 1, file)) {
        if (byte1 == 0xFF) {
            if (fread(&byte2, 1, 1, file)) {
                // Check for progressive JPEG marker (SOF2)
                if (byte2 == 0xC2) {
                    is_progressive = 1;
                    break;
                }
                // Check for baseline JPEG marker (SOF0)
                if (byte2 == 0xC0) {
                    is_progressive = 0;
                    break;
                }
            }
        }
    }

    fclose(file);
    return is_progressive;
}

static int is_image_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;

    // Check if it's a supported image format
    int is_supported = (strcasecmp(ext, ".png") == 0 ||
                       strcasecmp(ext, ".jpg") == 0 ||
                       strcasecmp(ext, ".jpeg") == 0);

    if (!is_supported) {
        return 0;
    }

    // For JPEG files, check if they are progressive and skip them
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) {
        // Build full file path
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", IMAGES_DIR_PATH, filename);

        if (is_progressive_jpeg(filepath)) {
            printf("Skipping progressive JPEG: %s\n", filename);
            return 0;
        }
    }

    return 1;
}

static void load_slideshow_images(void) {
    DIR *dir = opendir(IMAGES_DIR_PATH);
    if (!dir) {
        printf("Error: Cannot open images directory: %s\n", IMAGES_DIR_PATH);
        slideshow_state.image_count = 0;
        return;
    }

    struct dirent *entry;
    char temp_paths[MAX_SLIDESHOW_IMAGES][512];
    int temp_count = 0;

    // First pass: load all image filenames
    while ((entry = readdir(dir)) != NULL && temp_count < MAX_SLIDESHOW_IMAGES) {
        if (is_image_file(entry->d_name)) {
            snprintf(temp_paths[temp_count],
                    sizeof(temp_paths[0]),
                    "%s",
                    entry->d_name);
            temp_count++;
        }
    }

    closedir(dir);

    // Sort the filenames alphabetically
    for (int i = 0; i < temp_count - 1; i++) {
        for (int j = i + 1; j < temp_count; j++) {
            if (strcasecmp(temp_paths[i], temp_paths[j]) > 0) {
                char temp[512];
                strcpy(temp, temp_paths[i]);
                strcpy(temp_paths[i], temp_paths[j]);
                strcpy(temp_paths[j], temp);
            }
        }
    }

    // Second pass: copy sorted filenames with LVGL path prefix
    slideshow_state.image_count = 0;
    for (int i = 0; i < temp_count; i++) {
        snprintf(slideshow_state.image_paths[i],
                sizeof(slideshow_state.image_paths[0]),
                "A:%s/%s",
                IMAGES_DIR_PATH,
                temp_paths[i]);
        //printf("Loaded image %d: %s\n", i + 1, slideshow_state.image_paths[i]);
        slideshow_state.image_count++;
    }

    //printf("Total: Loaded %d images for slideshow\n", slideshow_state.image_count);
}

static void slideshow_timer_callback(lv_timer_t *timer) {
    (void)timer;

    if (slideshow_state.image_count == 0) {
        return;
    }

    // Move to next image
    slideshow_state.current_index = (slideshow_state.current_index + 1) % slideshow_state.image_count;

    // Update the image
    if (slideshow_state.slideshow_img) {
        lv_img_set_src(slideshow_state.slideshow_img,
                      slideshow_state.image_paths[slideshow_state.current_index]);
        // Invalidate to ensure rendering
        lv_obj_invalidate(slideshow_state.slideshow_img);
        //printf("Changed to image: %s\n", slideshow_state.image_paths[slideshow_state.current_index]);
    }
}

// ============================================================================
// SLIDESHOW PUBLIC API
// ============================================================================

int slideshow_init(lv_obj_t *parent_screen) {
    if (!parent_screen) {
        printf("Error: parent_screen is NULL\n");
        return -1;
    }

    // Load slideshow images
    load_slideshow_images();

    // Initialize if no images found
    if (slideshow_state.image_count == 0) {
        printf("Warning: No images found in %s directory\n", IMAGES_DIR_PATH);
        return -1;
    }

    // Create slideshow image
    slideshow_state.slideshow_img = lv_img_create(parent_screen);

    // Explicitly set size to match screen dimensions
    // Since images are 320x640, they should fill the entire screen
    lv_obj_set_width(slideshow_state.slideshow_img, SCREEN_WIDTH);
    lv_obj_set_height(slideshow_state.slideshow_img, SCREEN_HEIGHT);

    // Position at top-left corner
    lv_obj_align(slideshow_state.slideshow_img, LV_ALIGN_TOP_LEFT, 0, 0);

    // Move to background layer so title/status bars appear on top
    lv_obj_move_background(slideshow_state.slideshow_img);

    // Set first image immediately
    lv_img_set_src(slideshow_state.slideshow_img,
                  slideshow_state.image_paths[slideshow_state.current_index]);
    lv_obj_invalidate(slideshow_state.slideshow_img);

    // Create timer for slideshow
    slideshow_state.slideshow_timer = lv_timer_create(slideshow_timer_callback,
                                                      SLIDESHOW_INTERVAL, NULL);

    //printf("Slideshow initialized: %d images loaded, first image: %s\n",
    //       slideshow_state.image_count,
    //       slideshow_state.image_paths[slideshow_state.current_index]);

    return 0;
}

lv_obj_t *slideshow_get_image(void) {
    return slideshow_state.slideshow_img;
}
