#include "../include/video.h"
#include "../include/config.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// ============================================================================
// VIDEO PLAYBACK STATE
// ============================================================================

#define MAX_VIDEO_FILES 20

typedef struct {
    lv_obj_t *video_player;
    char video_paths[MAX_VIDEO_FILES][MAX_VIDEO_PATH];
    int video_count;
    int current_index;
    bool is_playing;
    bool is_visible;
    bool is_initialized;
    lv_timer_t *check_timer;
    uint32_t video_start_time;
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

static int load_video_files(void) {
    DIR *dir = opendir(VIDEO_DIR);
    if (!dir) {
        printf("Warning: Cannot open video directory: %s\n", VIDEO_DIR);
        return -1;
    }

    struct dirent *entry;
    char temp_paths[MAX_VIDEO_FILES][256];
    int temp_count = 0;

    // First pass: collect all video filenames
    while ((entry = readdir(dir)) != NULL && temp_count < MAX_VIDEO_FILES) {
        if (is_video_file(entry->d_name)) {
            snprintf(temp_paths[temp_count],
                    sizeof(temp_paths[0]),
                    "%s",
                    entry->d_name);
            temp_count++;
        }
    }

    closedir(dir);

    // Sort filenames alphabetically
    for (int i = 0; i < temp_count - 1; i++) {
        for (int j = i + 1; j < temp_count; j++) {
            if (strcasecmp(temp_paths[i], temp_paths[j]) > 0) {
                char temp[256];
                strcpy(temp, temp_paths[i]);
                strcpy(temp_paths[i], temp_paths[j]);
                strcpy(temp_paths[j], temp);
            }
        }
    }

    // Second pass: create full paths
    video_state.video_count = 0;
    for (int i = 0; i < temp_count; i++) {
        snprintf(video_state.video_paths[i],
                sizeof(video_state.video_paths[0]),
                "%s/%s",
                VIDEO_DIR,
                temp_paths[i]);
        video_state.video_count++;
    }
    return video_state.video_count > 0 ? 0 : -1;
}

static void video_finished_callback(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    
    // Debug: log events to see what's actually firing
    static int event_counter = 0;
    if (++event_counter <= 10) {  // Only log first 10 events to avoid spam
        printf("Video event %d: code=%d\n", event_counter, event_code);
    }
    
    // With auto-restart enabled, LV_EVENT_READY should fire when video completes and restarts
    // We intercept this to switch to the next video instead of letting it restart the same video
    
    // Check for events that might indicate video completion
    if (event_code == LV_EVENT_READY || event_code == LV_EVENT_VALUE_CHANGED || event_code == LV_EVENT_REFRESH) {
        if (!video_state.is_playing) {
            return;
        }
        
        // Prevent rapid switching with debounce
        static uint32_t last_switch_time = 0;
        uint32_t current_time = lv_tick_get();
        
        if (current_time - last_switch_time < 2000) {  // 2 second debounce
            return;
        }
        
        if (video_state.video_count <= 1) {
            return;
        }

        // Try to switch to next video
        printf("Attempting video switch on event %d\n", event_code);
        last_switch_time = current_time;

        // Switch to next video in circular fashion
        video_state.current_index = (video_state.current_index + 1) % video_state.video_count;

        // Load and start next video
        lv_res_t res = lv_ffmpeg_player_set_src(video_state.video_player,
                                                video_state.video_paths[video_state.current_index]);
        if (res == LV_RES_OK) {
            printf("Switched to video %d\n", video_state.current_index + 1);
            lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
        } else {
            printf("ERROR: Failed to load next video! (error code: %d)\n", res);
        }
    }
}

static void video_check_timer(lv_timer_t *timer) {
    (void)timer;
    
    if (!video_state.is_playing || !video_state.video_player) {
        return;
    }
    
    // Check if enough time has passed (assume videos are max 60 seconds)
    uint32_t current_time = lv_tick_get();
    uint32_t elapsed = current_time - video_state.video_start_time;
    
    // If more than 60 seconds, assume video finished and switch
    if (elapsed > 60000) {  // 60 seconds in milliseconds
        printf("Timer: Video timeout, switching to next\n");
        
        if (video_state.video_count > 1) {
            video_state.current_index = (video_state.current_index + 1) % video_state.video_count;
            video_state.video_start_time = current_time;
            
            lv_res_t res = lv_ffmpeg_player_set_src(video_state.video_player,
                                                    video_state.video_paths[video_state.current_index]);
            if (res == LV_RES_OK) {
                printf("Timer: Switched to video %d\n", video_state.current_index + 1);
                lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
            } else {
                printf("ERROR: Timer failed to load next video!\n");
            }
        }
    }
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
    // Load all video files
    if (load_video_files() != 0) {
        printf("Warning: No video files found in %s directory\n", VIDEO_DIR);
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

    // Set the first video source
    video_state.current_index = 0;
    if (lv_ffmpeg_player_set_src(video_state.video_player,
                                  video_state.video_paths[video_state.current_index]) != LV_RES_OK) {
        printf("Error: Failed to set video source: %s\n", video_state.video_paths[video_state.current_index]);
        lv_obj_del(video_state.video_player);
        video_state.video_player = NULL;
        return -1;
    }

    // Initially pause the video (we'll start it manually)
    lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_PAUSE);
    
    // Disable auto restart - we'll handle video switching manually
    lv_ffmpeg_player_set_auto_restart(video_state.video_player, false);

    // Register for multiple events that might indicate video completion
    lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_REFRESH, NULL);
    
    // Create timer to check video completion as fallback
    video_state.check_timer = lv_timer_create(video_check_timer, 5000, NULL);  // Check every 5 seconds
    video_state.video_start_time = lv_tick_get();
    if (video_state.check_timer) {
        lv_timer_pause(video_state.check_timer);  // Start paused
    }
    
    // Initially hidden
    lv_obj_add_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
    video_state.is_visible = false;
    video_state.is_playing = false;
    video_state.is_initialized = true;
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
        video_state.video_start_time = lv_tick_get();  // Reset timer
        
        // Start the check timer
        if (video_state.check_timer) {
            lv_timer_resume(video_state.check_timer);
        }
    }
#endif
}

void video_stop(void) {
#if LV_USE_FFMPEG
    if (video_state.video_player && video_state.is_initialized && video_state.is_playing) {
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_STOP);
        video_state.is_playing = false;

        // Reset to first video
        video_state.current_index = 0;
        if (video_state.video_count > 0) {
            lv_ffmpeg_player_set_src(video_state.video_player,
                                     video_state.video_paths[video_state.current_index]);
        }


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

void video_get_info(int *index, int *total, char *path, size_t path_size) {
    if (index) {
        *index = video_state.current_index;
    }
    if (total) {
        *total = video_state.video_count;
    }
    if (path && path_size > 0 && video_state.video_count > 0) {
        strncpy(path, video_state.video_paths[video_state.current_index], path_size - 1);
        path[path_size - 1] = '\0';
    }
}

void video_next(void) {
#if LV_USE_FFMPEG
    if (!video_state.is_initialized || video_state.video_count == 0) {
        return;
    }

    // Move to next video
    video_state.current_index = (video_state.current_index + 1) % video_state.video_count;
    

    
    // Load and play next video if currently playing
    if (lv_ffmpeg_player_set_src(video_state.video_player,
                                  video_state.video_paths[video_state.current_index]) == LV_RES_OK) {
        if (video_state.is_playing) {
            lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
        }
    }
#endif
}

void video_previous(void) {
#if LV_USE_FFMPEG
    if (!video_state.is_initialized || video_state.video_count == 0) {
        return;
    }

    // Move to previous video (with wrap-around)
    video_state.current_index = (video_state.current_index - 1 + video_state.video_count) % video_state.video_count;
    

    
    // Load and play previous video if currently playing
    if (lv_ffmpeg_player_set_src(video_state.video_player,
                                  video_state.video_paths[video_state.current_index]) == LV_RES_OK) {
        if (video_state.is_playing) {
            lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
        }
    }
#endif
}

int video_get_count(void) {
    return video_state.video_count;
}

void video_trigger_next(void) {
    if (!video_state.is_playing || video_state.video_count <= 1) {
        return;
    }
    
    // Move to next video
    video_state.current_index = (video_state.current_index + 1) % video_state.video_count;
    
    // Load and play next video
    if (lv_ffmpeg_player_set_src(video_state.video_player,
                                  video_state.video_paths[video_state.current_index]) == LV_RES_OK) {
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
    } else {
        printf("ERROR: Failed to load video: %s\n", video_state.video_paths[video_state.current_index]);
    }
}
