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

static int is_video_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;

    return (strcasecmp(ext, ".mp4") == 0 ||
            strcasecmp(ext, ".avi") == 0 ||
            strcasecmp(ext, ".mkv") == 0 ||
            strcasecmp(ext, ".mov") == 0 ||
            strcasecmp(ext, ".webm") == 0);
}

static int find_video_file(void) {
    DIR *dir = opendir("assets/videos");
    if (!dir) {
        printf("Warning: Cannot open video directory: assets/videos\n");
        return -1;
    }

    struct dirent *entry;
    int found = 0;

    // Find the first video file
    while ((entry = readdir(dir)) != NULL) {
        if (is_video_file(entry->d_name)) {
            // Check if the full path will fit in the buffer
            size_t name_len = strlen(entry->d_name);
            size_t dir_len = strlen(VIDEO_FILES_DIR);
            if (name_len + dir_len + 2 > MAX_VIDEO_PATH) {
                printf("Warning: Video filename too long, skipping: %s\n", entry->d_name);
                continue;
            }

            snprintf(video_state.video_path,
                    sizeof(video_state.video_path),
                    "%s/%s",
                    VIDEO_FILES_DIR,
                    entry->d_name);
            found = 1;
            printf("Found video file: %s\n", video_state.video_path);
            break;
        }
    }

    closedir(dir);
    return found ? 0 : -1;
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

    // Set size to match content area (between title and status bars)
    lv_obj_set_width(video_state.video_player, SCREEN_WIDTH);
    lv_obj_set_height(video_state.video_player, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);

    // Position below title bar
    lv_obj_align(video_state.video_player, LV_ALIGN_TOP_LEFT, 0, TITLE_BAR_HEIGHT);

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
