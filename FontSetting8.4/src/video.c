#include "../include/video.h"
#include "../include/config.h"
#include "../include/logger.h"
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
    lv_obj_t *parent_screen;
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

static void video_finished_callback(lv_event_t *e);

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

static int recreate_video_player(int video_index) {
    if (!video_state.parent_screen || video_index >= video_state.video_count) {
        return -1;
    }

    // Delete old player if it exists - this completely cleans up the previous video
    if (video_state.video_player) {
        // Stop playback first
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_STOP);
        // Delete the player object and free all resources
        lv_obj_del(video_state.video_player);
        video_state.video_player = NULL;
    }

    // Create NEW FFmpeg player for the next video
    video_state.video_player = lv_ffmpeg_player_create(video_state.parent_screen);
    if (!video_state.video_player) {
        log_error("Failed to create FFmpeg player");
        return -1;
    }

    // Videos are 368x640 (FFmpeg-aligned), center on 360x640 screen
    lv_obj_set_size(video_state.video_player, 368, SCREEN_HEIGHT);
    lv_obj_align(video_state.video_player, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_background(video_state.video_player);
    
    // Clip overflow to prevent 4px overhang on each side
    lv_obj_add_flag(video_state.video_player, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_clip_corner(video_state.video_player, true, 0);

    // Set video source for the NEW player
    if (lv_ffmpeg_player_set_src(video_state.video_player,
                                  video_state.video_paths[video_index]) != LV_RES_OK) {
        log_error("Failed to set video source");
        lv_obj_del(video_state.video_player);
        video_state.video_player = NULL;
        return -1;
    }

    // Configure NEW player
    lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_PAUSE);
    
    bool use_auto_restart = (video_state.video_count == 1);
    lv_ffmpeg_player_set_auto_restart(video_state.video_player, use_auto_restart);

    if (!use_auto_restart) {
        lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_READY, NULL);
        lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_add_event_cb(video_state.video_player, video_finished_callback, LV_EVENT_REFRESH, NULL);
    }

    // Set visibility
    if (video_state.is_visible) {
        lv_obj_clear_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(video_state.video_player, LV_OBJ_FLAG_HIDDEN);
    }

    log_info("Video player recreated for video %d", video_index);
    return 0;
}

static void video_finished_callback(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    
    // Event counter for debugging if needed
    static int event_counter = 0;
    event_counter++;
    
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
        
        if (current_time - last_switch_time < 3000) {  // 3 second debounce - increased for stability
            return;
        }
        
        if (video_state.video_count <= 1) {
            return;
        }

        // Try to switch to next video
        last_switch_time = current_time;

        // Switch to next video in circular fashion
        video_state.current_index = (video_state.current_index + 1) % video_state.video_count;

        // Recreate player with new video
        if (recreate_video_player(video_state.current_index) == 0) {
            if (video_state.is_playing) {
                lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
            }
            video_state.video_start_time = lv_tick_get();
        } else {
            log_error("Failed to recreate video player");
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
        if (video_state.video_count > 1) {
            video_state.current_index = (video_state.current_index + 1) % video_state.video_count;
            
            if (recreate_video_player(video_state.current_index) == 0) {
                lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
                video_state.video_start_time = lv_tick_get();
            } else {
                log_error("Timer failed to recreate video player");
            }
        }
    }
}

// ============================================================================
// VIDEO PUBLIC API
// ============================================================================

int video_init(lv_obj_t *parent_screen) {
    if (!parent_screen) {
        log_error("parent_screen is NULL in video_init");
        return -1;
    }

#if LV_USE_FFMPEG
    // Load all video files
    if (load_video_files() != 0) {
        log_warning("No video files found in videos directory");
        return -1;
    }

    // Store parent screen for recreating player later
    video_state.parent_screen = parent_screen;
    video_state.current_index = 0;

    // Create initial video player
    if (recreate_video_player(0) != 0) {
        return -1;
    }
    
    // Create timer for fallback video completion check (only if multiple videos)
    if (video_state.video_count > 1) {
        video_state.check_timer = lv_timer_create(video_check_timer, 5000, NULL);
        if (video_state.check_timer) {
            lv_timer_pause(video_state.check_timer);
        }
    }
    
    video_state.video_start_time = lv_tick_get();
    video_state.is_visible = false;
    video_state.is_playing = false;
    video_state.is_initialized = true;
    
    return 0;
#else
    log_error("FFmpeg support is not enabled. Please enable LV_USE_FFMPEG in lv_conf.h");
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

        // Pause the check timer
        if (video_state.check_timer) {
            lv_timer_pause(video_state.check_timer);
        }

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

    bool was_playing = video_state.is_playing;

    // Move to next video
    video_state.current_index = (video_state.current_index + 1) % video_state.video_count;
    
    // Recreate player with new video
    if (recreate_video_player(video_state.current_index) == 0) {
        if (was_playing) {
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

    bool was_playing = video_state.is_playing;

    // Move to previous video (with wrap-around)
    video_state.current_index = (video_state.current_index - 1 + video_state.video_count) % video_state.video_count;
    
    // Recreate player with new video
    if (recreate_video_player(video_state.current_index) == 0) {
        if (was_playing) {
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
    
    // Recreate player with new video
    if (recreate_video_player(video_state.current_index) == 0) {
        lv_ffmpeg_player_set_cmd(video_state.video_player, LV_FFMPEG_PLAYER_CMD_START);
        video_state.video_start_time = lv_tick_get();
    } else {
        log_error("Failed to recreate video player");
    }
}
