#include "../include/video.h"
#include "../include/config.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// ============================================================================
// VIDEO PLAYBACK CONFIGURATION
// ============================================================================

#define VIDEO_FILES_DIR "assets/videos"

typedef struct {
    lv_obj_t *video_player;
    char video_path[MAX_VIDEO_PATH];
    bool is_playing;
    bool is_visible;
    bool is_initialized;
} VideoState;

static VideoState video_state = {0};

// ============================================================================
// VIDEO HELPER FUNCTIONS
// ============================================================================

static int find_video_file(void) {
    // Use specific video file: Video02.mp4
    const char *video_filename = "Video02.mp4";

    snprintf(video_state.video_path,
            sizeof(video_state.video_path),
            "%s/%s",
            VIDEO_FILES_DIR,
            video_filename);

    // Check if file exists
    FILE *file = fopen(video_state.video_path, "r");
    if (!file) {
        printf("Warning: Video file not found: %s\n", video_state.video_path);
        return -1;
    }
    fclose(file);

    printf("Found video file: %s\n", video_state.video_path);
    return 0;
}

// ============================================================================
// VIDEO PUBLIC API
// ============================================================================

int video_init(lv_obj_t *parent_screen) {
    if (!parent_screen) {
        printf("Error: parent_screen is NULL\n");
        return -1;
    }

#if LV_USE_FFMPEG
    // Find a video file
    if (find_video_file() != 0) {
        printf("Warning: No video files found in assets/videos directory\n");
        return -1;
    }

    // Create FFmpeg player
    video_state.video_player = lv_ffmpeg_player_create(parent_screen);
    if (!video_state.video_player) {
        printf("Error: Failed to create FFmpeg player\n");
        return -1;
    }

    // Set size to full screen (320x640)
    lv_obj_set_width(video_state.video_player, SCREEN_WIDTH);
    lv_obj_set_height(video_state.video_player, SCREEN_HEIGHT);

    // Position at top-left corner (full screen)
    lv_obj_align(video_state.video_player, LV_ALIGN_TOP_LEFT, 0, 0);

    // Move video to background so title/status bars appear on top
    lv_obj_move_background(video_state.video_player);

    // Set the video source
    if (lv_ffmpeg_player_set_src(video_state.video_player, video_state.video_path) != LV_RES_OK) {
        printf("Error: Failed to set video source: %s\n", video_state.video_path);
        lv_obj_del(video_state.video_player);
        video_state.video_player = NULL;
        return -1;
    }

    // Configure auto restart (loop the video)
    lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_PAUSE);
    lv_ffmpeg_player_set_auto_restart(video_state.video_player, true);

    // Initially hidden
    lv_obj_add_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
    video_state.is_visible = false;
    video_state.is_playing = false;
    video_state.is_initialized = true;

    printf("Video player initialized with: %s\n", video_state.video_path);
    return 0;
#else
    printf("Error: FFmpeg support is not enabled. Please enable LV_USE_FFMPEG in lv_conf.h\n");
    return -1;
#endif
}

void video_start(void) {
#if LV_USE_FFMPEG
    if (video_state.video_player && video_state.is_initialized && !video_state.is_playing) {
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
        video_state.is_playing = true;
        printf("Video playback started\n");
    }
#endif
}

void video_stop(void) {
#if LV_USE_FFMPEG
    if (video_state.video_player && video_state.is_initialized && video_state.is_playing) {
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_STOP);
        video_state.is_playing = false;
        printf("Video playback stopped\n");
    }
#endif
}

void video_hide(void) {
    if (video_state.video_player && video_state.is_visible) {
        lv_obj_add_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
        video_state.is_visible = false;
    }
}

void video_show(void) {
    if (video_state.video_player && !video_state.is_visible) {
        lv_obj_clear_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
        video_state.is_visible = true;
    }
}

lv_obj_t *video_get_player(void) {
    return video_state.video_player;
}

bool video_is_playing(void) {
    return video_state.is_playing;
}
