#ifndef GTK_APP_H
#define GTK_APP_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <memory>
#include "camera.h"
#include "face_detector.h"
#include "deep_face_recognizer.h"
#include "face_database.h"
#include "frame_processor.h"
#include "ui_renderer.h"
#include "training_manager.h"
#include "socket_server.h"
#include "config.h"
#include "logger.h"
#include "exceptions.h"

class GTKApp {
private:
    // GTK Widgets
    GtkWidget* window;
    GtkWidget* image_widget;
    GtkWidget* toggle_button;
    GtkWidget* train_button;
    GtkWidget* capture_button;
    GtkWidget* status_label;
    GtkWidget* fps_label;
    GtkWidget* recognition_time_label;  // Display elapsed time for recognition

    // Camera and Face Recognition
    Camera camera;
    FaceDetector face_detector;
    DeepFaceRecognizer face_recognizer;
    FaceDatabase face_database;

    // Refactored components
    std::unique_ptr<FrameProcessor> frame_processor;
    std::unique_ptr<UIRenderer> ui_renderer;
    std::unique_ptr<TrainingManager> training_manager;
    std::unique_ptr<SocketServer> socket_server;

    guint refresh_timer;
    guint recognition_timer;  // Separate timer for recognition (100ms = 10 times/sec)
    bool camera_running;
    bool face_recognition_enabled;
    std::atomic<bool> training_in_progress;
    bool capture_in_progress;
    bool cleanup_done;
    int frame_count;
    int recognition_frame_count;  // Count frames where recognition actually ran
    gint64 last_time;
    cv::Mat last_frame;
    cv::Mat latest_frame;  // Latest frame for recognition timer
    std::mutex latest_frame_mutex;  // Protect latest_frame access
    std::vector<Face> recognized_faces;  // Store recognition results
    std::mutex recognized_faces_mutex;  // Protect recognized_faces access
    int capture_count;
    gint64 last_recognition_time;

    // Cache for last recognition result (to display on stream)
    std::string last_recognized_name;
    double last_recognized_confidence;
    bool has_recognition_result;

    // Use Config constants for thresholds
    // DYNAMIC_BOX_SCALE -> Config::BOUNDING_BOX_SCALE
    // RECOGNITION_THRESHOLD -> 70.0 (percentage, derived from Config::RECOGNITION_CONFIDENCE_THRESHOLD * 100)

    // Training thread management
    std::thread training_thread;
    std::atomic<bool> training_success;

    // Face recognition mutex (ONNX Runtime is not thread-safe)
    std::mutex recognition_mutex;

    // Static callback wrappers
    static gboolean on_refresh_timer(gpointer user_data);
    static gboolean on_recognition_timer(gpointer user_data);
    static void on_toggle_button_clicked(GtkWidget* widget, gpointer user_data);
    static void on_train_button_clicked(GtkWidget* widget, gpointer user_data);
    static void on_capture_button_clicked(GtkWidget* widget, gpointer user_data);
    static void on_window_destroy(GtkWidget* widget, gpointer user_data);
    static gboolean on_training_complete(gpointer user_data);
    static gboolean on_camera_stop_complete(gpointer user_data);

    // Instance methods
    gboolean refresh_frame();
    gboolean process_recognition();
    void toggle_camera();
    void train_model();
    void train_model_async();
    void on_training_finished();
    void on_camera_stop_finished();
    void capture_photo();
    void update_ui();
    GdkPixbuf* mat_to_pixbuf(const cv::Mat& mat);
    void draw_faces_on_frame(cv::Mat& frame, const std::vector<Face>& faces);
    void load_face_recognizer();

    // Socket command handlers
    void setup_socket_server();
    std::string handle_camera_on(const std::string& args);
    std::string handle_camera_off(const std::string& args);
    std::string handle_capture(const std::string& args);
    std::string handle_registering(const std::string& args);
    std::string handle_status(const std::string& args);
    std::string handle_list_persons(const std::string& args);
    void handle_stream_recognition(int client_fd);

    // Thread-safe camera control (for use from socket server thread)
    bool start_camera_safe();
    bool stop_camera_safe();

public:
    GTKApp();
    ~GTKApp();

    bool init();
    void run();
    void cleanup();
};

#endif // GTK_APP_H
