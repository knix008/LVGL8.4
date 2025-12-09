#include "gtk_app.h"
#include "logger.h"
#include <signal.h>
#include <csignal>
#include <atomic>

// Global flag for shutdown request (atomic and signal-safe)
static std::atomic<int> shutdown_requested(0);

// Global pointer to app
static GTKApp* g_app = nullptr;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        shutdown_requested.store(sig);
    }
}

// Idle function to handle shutdown from GTK main loop
static gboolean on_shutdown_idle(gpointer /* user_data */) {
    LOG_INFO("Shutdown requested from GTK main loop");
    if (g_app) {
        g_app->cleanup();
    }
    gtk_main_quit();
    return FALSE;  // Remove from idle queue
}

int main(int /*argc*/, char* /*argv*/[]) {
    try {
        GTKApp app;
        g_app = &app;

        // Register signal handlers for graceful shutdown
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        if (!app.init()) {
            LOG_ERROR("Failed to initialize GTK application");
            return 1;
        }

        // Add periodic check for shutdown signals
        g_timeout_add(100, [](gpointer /* user_data */) -> gboolean {
            if (shutdown_requested.load() != 0) {
                int sig = shutdown_requested.load();
                LOG_INFO("Received signal " << sig << " - initiating graceful shutdown");
                // Schedule cleanup from GTK main loop
                g_idle_add(on_shutdown_idle, nullptr);
                return FALSE;  // Remove from timeout
            }
            return TRUE;  // Keep checking
        }, nullptr);

        LOG_INFO("GTK Webcam Viewer started successfully");
        app.run();
        app.cleanup();
    } catch (const std::exception& e) {
        LOG_ERROR("Exception occurred: " << e.what());
        return 1;
    }

    return 0;
}
