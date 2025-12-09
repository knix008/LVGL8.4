#include "gtk_app.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>

GTKApp::GTKApp()
    : window(nullptr), image_widget(nullptr), toggle_button(nullptr),
      train_button(nullptr), capture_button(nullptr),
      status_label(nullptr), fps_label(nullptr), face_info_label(nullptr),
      face_count_label(nullptr), error_rate_label(nullptr),
      refresh_timer(0), camera_running(false), face_recognition_enabled(false),
      training_in_progress(false), capture_in_progress(false), cleanup_done(false),
      frame_count(0), recognition_frame_count(0), last_time(0), capture_count(0), last_recognition_time(0),
      last_recognized_name("Unknown"), last_recognized_confidence(0.0),
      has_recognition_result(false), training_success(false) {}

GTKApp::~GTKApp() {
    cleanup();
}

bool GTKApp::init() {
    try {
        // Initialize GTK
        gtk_init(nullptr, nullptr);

        // Create main window
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "GTK Webcam Viewer");
        gtk_window_set_default_size(GTK_WINDOW(window), Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

        // Connect window destroy signal
        g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), this);

        // Create main container (vertical box)
        GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
        gtk_container_add(GTK_CONTAINER(window), vbox);

        // Create image display widget
        image_widget = gtk_image_new();
        gtk_widget_set_size_request(image_widget, Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);
        gtk_box_pack_start(GTK_BOX(vbox), image_widget, TRUE, TRUE, 0);

        // Create horizontal box for controls
        GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        // Create toggle button
        toggle_button = gtk_toggle_button_new_with_label("Start Camera");
        gtk_widget_set_size_request(toggle_button, 150, 40);
        g_signal_connect(toggle_button, "clicked", G_CALLBACK(on_toggle_button_clicked), this);
        gtk_box_pack_start(GTK_BOX(hbox), toggle_button, FALSE, FALSE, 0);

        // Create train button
        train_button = gtk_button_new_with_label("Registering");
        gtk_widget_set_size_request(train_button, 150, 40);
        g_signal_connect(train_button, "clicked", G_CALLBACK(on_train_button_clicked), this);
        gtk_box_pack_start(GTK_BOX(hbox), train_button, FALSE, FALSE, 0);

        // Create capture button
        capture_button = gtk_button_new_with_label("Capture Photo");
        gtk_widget_set_size_request(capture_button, 150, 40);
        g_signal_connect(capture_button, "clicked", G_CALLBACK(on_capture_button_clicked), this);
        gtk_box_pack_start(GTK_BOX(hbox), capture_button, FALSE, FALSE, 0);

        // Create status label
        status_label = gtk_label_new("Status: Camera Idle");
        gtk_box_pack_start(GTK_BOX(hbox), status_label, TRUE, TRUE, 0);

        // Create Recognition FPS label
        fps_label = gtk_label_new("Recognition FPS: 0");
        gtk_box_pack_end(GTK_BOX(hbox), fps_label, FALSE, FALSE, 0);

        // Create face info label (recognized person)
        face_info_label = gtk_label_new("Person: None detected");
        gtk_box_pack_end(GTK_BOX(hbox), face_info_label, FALSE, FALSE, 0);

        // Create face count label (confidence level)
        face_count_label = gtk_label_new("Confidence: 0%");
        gtk_box_pack_end(GTK_BOX(hbox), face_count_label, FALSE, FALSE, 0);

        // Create error rate label (detection metrics)
        error_rate_label = gtk_label_new("Detection Rate: 0% | Error: 0%");
        gtk_box_pack_end(GTK_BOX(hbox), error_rate_label, FALSE, FALSE, 0);

        // Create recognition time label
        recognition_time_label = gtk_label_new("Recognition: 0ms");
        gtk_box_pack_end(GTK_BOX(hbox), recognition_time_label, FALSE, FALSE, 0);

        // Open camera
        if (!camera.open(0)) {
            LOG_WARN("Camera initialization failed");
            // Update status but continue - user can try to enable camera later
            gtk_label_set_text(GTK_LABEL(status_label), "Status: Camera Not Available");
            gtk_widget_set_sensitive(toggle_button, FALSE);
        }

        // Load face recognizer
        load_face_recognizer();

        // Initialize frame processor
        try {
            frame_processor = std::make_unique<FrameProcessor>();

            // Create and initialize a new FaceDetector for the frame processor
            auto detector = std::make_unique<FaceDetector>();
            if (!detector->initialize()) {
                LOG_ERROR("Failed to initialize FaceDetector for FrameProcessor");
                throw std::runtime_error("FaceDetector initialization failed");
            }

            frame_processor->initialize(
                std::move(detector),
                &face_recognizer
            );
            frame_processor->set_frame_scale(1.0);
            frame_processor->set_horizontal_flip(true);
            frame_processor->set_recognition_interval(Config::RECOGNITION_UPDATE_INTERVAL_US);
            LOG_INFO("Frame processor initialized successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize frame processor: " << e.what());
            throw;
        }

        // Initialize UI renderer
        try {
            ui_renderer = std::make_unique<UIRenderer>(
                Config::DISPLAY_WIDTH,
                Config::DISPLAY_HEIGHT
            );
            LOG_INFO("UI renderer initialized successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize UI renderer: " << e.what());
            throw;
        }

        // Initialize socket server for remote control
        try {
            setup_socket_server();
            LOG_INFO("Socket server initialized successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize socket server: " << e.what());
            // Continue without socket server - not critical
        }

        // Initialize training manager
        try {
            training_manager = std::make_unique<TrainingManager>();
            training_manager->initialize(&face_recognizer, &face_database);
            LOG_INFO("Training manager initialized successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize training manager: " << e.what());
            throw;
        }

        // Show all widgets
        gtk_widget_show_all(window);

        // Set up refresh timer (30ms = ~33 FPS)
        refresh_timer = g_timeout_add(30, on_refresh_timer, this);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during initialization: " << e.what());
        return false;
    }
}

void GTKApp::run() {
    gtk_main();
}

void GTKApp::cleanup() {
    // Prevent double-cleanup (can be called from both window destroy and destructor)
    if (cleanup_done) {
        return;
    }
    cleanup_done = true;

    // Stop socket server first (before other cleanup)
    if (socket_server) {
        socket_server->stop();
    }

    // Stop camera and frame processing
    camera_running = false;  // Signal to stop processing frames

    // Process pending events - this will let refresh_frame return FALSE and stop naturally
    for (int i = 0; i < 5; i++) {
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        g_usleep(20000); // 20ms between iterations
    }

    // Now it's safe to close the camera
    camera.close();  // This will join the camera thread

    // Clear the timer ID (it should have stopped by now)
    refresh_timer = 0;

    // Wait for training thread to finish
    if (training_thread.joinable()) {
        training_in_progress = false;  // Signal thread to stop if possible
        training_thread.join();
    }

    // Note: Don't explicitly close database - the FaceDatabase destructor will handle it
    // Calling close() here and then having destructor call it again causes double-free

    // Destroy window last (but only if not already destroyed by GTK)
    // The window destroy signal already triggered this cleanup
    if (window != nullptr) {
        window = nullptr;
    }
}

gboolean GTKApp::on_refresh_timer(gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    return self->refresh_frame();
}

gboolean GTKApp::refresh_frame() {
    // Stop timer immediately if cleanup has started
    if (cleanup_done) {
        return FALSE; // Stop timer
    }

    if (!camera_running || capture_in_progress || training_in_progress) {
        return TRUE; // Continue timer but don't process frames
    }

    try {
        cv::Mat frame;
        if (camera.get_frame(frame)) {
            if (!frame.empty()) {
                // Resize frame to match window display size (640x480) while maintaining aspect ratio
                const int target_width = Config::DISPLAY_WIDTH;
                const int target_height = Config::DISPLAY_HEIGHT;

                // Calculate scaling to fit within target size while maintaining aspect ratio
                double scale = std::min(
                    static_cast<double>(target_width) / frame.cols,
                    static_cast<double>(target_height) / frame.rows
                );

                int new_width = static_cast<int>(frame.cols * scale);
                int new_height = static_cast<int>(frame.rows * scale);

                // Resize with scaling
                cv::Mat scaled_frame;
                cv::resize(frame, scaled_frame, cv::Size(new_width, new_height));

                // Create output frame with letterboxing (black borders)
                cv::Mat resized_frame = cv::Mat::zeros(target_height, target_width, scaled_frame.type());

                // Calculate position to center the scaled frame
                int x_offset = (target_width - new_width) / 2;
                int y_offset = (target_height - new_height) / 2;

                // Place scaled frame in center of output frame
                scaled_frame.copyTo(resized_frame(cv::Rect(x_offset, y_offset, new_width, new_height)));

                frame = resized_frame;

                // Flip the frame horizontally for mirror effect
                cv::flip(frame, frame, 1);

                // Use FrameProcessor for face detection and recognition
                ProcessedFrame processed = frame_processor->process_frame(
                    frame,
                    face_recognition_enabled && !training_in_progress
                );

                if (!processed.is_valid) {
                    return TRUE;
                }

                // Track if recognition actually ran based on frame processor flag
                if (processed.recognition_ran) {
                    recognition_frame_count++;
                }

                // Update recognition processing time
                gchar recognition_time_text[100];
                g_snprintf(recognition_time_text, sizeof(recognition_time_text),
                          "Recognition: %.1fms", processed.processing_time_ms);
                gtk_label_set_text(GTK_LABEL(recognition_time_label), recognition_time_text);

                // Track best recognized face for UI display
                std::string best_person_name = "None detected";
                double best_confidence = 0.0;
                int recognized_count = 0;
                int unknown_count = 0;

                // Count recognized vs unknown faces
                for (const auto& face : processed.faces) {
                    if (face.id != -1) {
                        recognized_count++;
                        // Track best (highest confidence)
                        if (face.confidence > best_confidence) {
                            best_confidence = face.confidence;
                            best_person_name = face.name;
                            // Cache result for continuous display
                            last_recognized_name = face.name;
                            last_recognized_confidence = face.confidence;
                            has_recognition_result = true;
                            last_recognition_time = g_get_monotonic_time();
                        }
                    } else {
                        unknown_count++;
                    }
                }
                
                // Reset recognition result if no faces detected
                if (recognized_count == 0 && unknown_count == 0) {
                    has_recognition_result = false;
                }

                // Update UI with recognized person and confidence
                if (recognized_count > 0) {
                    gchar person_text[100];
                    g_snprintf(person_text, sizeof(person_text), "Person: %s (%d face%s)",
                              best_person_name.c_str(), recognized_count,
                              recognized_count > 1 ? "s" : "");
                    gtk_label_set_text(GTK_LABEL(face_info_label), person_text);

                    gchar conf_text[100];
                    g_snprintf(conf_text, sizeof(conf_text), "Confidence: %.1f%%", best_confidence);
                    gtk_label_set_text(GTK_LABEL(face_count_label), conf_text);
                } else if (unknown_count > 0) {
                    gchar person_text[100];
                    g_snprintf(person_text, sizeof(person_text), "Unknown: %d face%s detected",
                              unknown_count, unknown_count > 1 ? "s" : "");
                    gtk_label_set_text(GTK_LABEL(face_info_label), person_text);
                    gtk_label_set_text(GTK_LABEL(face_count_label), "Confidence: N/A");
                } else {
                    gtk_label_set_text(GTK_LABEL(face_info_label), "Person: None detected");
                    gtk_label_set_text(GTK_LABEL(face_count_label), "Confidence: 0%");
                }

                // Save clean frame for capture (BEFORE drawing on it)
                last_frame = processed.frame.clone();

                // Draw all detected faces on frame for display
                if (!processed.faces.empty()) {
                    draw_faces_on_frame(processed.frame, processed.faces);
                }

                // Convert to pixbuf and display
                GdkPixbuf* pixbuf = ui_renderer->mat_to_pixbuf(processed.frame);
                if (pixbuf != nullptr) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), pixbuf);
                    g_object_unref(pixbuf);
                }

                // Update recognition FPS counter
                frame_count++;
                gint64 current_time = g_get_monotonic_time();
                if (last_time == 0) {
                    last_time = current_time;
                }

                gint64 elapsed_us = current_time - last_time;
                if (elapsed_us >= 1000000) { // 1 second
                    double recognition_fps = (recognition_frame_count * 1000000.0) / elapsed_us;
                    gchar fps_text[50];
                    g_snprintf(fps_text, sizeof(fps_text), "Recognition FPS: %.1f", recognition_fps);
                    gtk_label_set_text(GTK_LABEL(fps_label), fps_text);

                    // Update detection error rate metrics
                    double detection_rate = face_detector.get_detection_rate();
                    double false_positive_rate = face_detector.get_false_positive_rate();
                    gchar error_rate_text[100];
                    g_snprintf(error_rate_text, sizeof(error_rate_text),
                              "Detection: %.1f%% | Error: %.1f%%",
                              detection_rate, false_positive_rate);
                    gtk_label_set_text(GTK_LABEL(error_rate_label), error_rate_text);

                    frame_count = 0;
                    recognition_frame_count = 0;
                    last_time = current_time;
                }
            }
        } else if (!camera.is_camera_active()) {
            // Camera was stopped or disconnected
            LOG_INFO("Camera disconnected");
            camera_running = false;
            gtk_button_set_label(GTK_BUTTON(toggle_button), "Start Camera");
            gtk_label_set_text(GTK_LABEL(status_label), "Status: Camera Disconnected");
            gtk_image_clear(GTK_IMAGE(image_widget));
        }
    } catch (const DetectionException& e) {
        LOG_WARN("Face detection error: " << e.what());
    } catch (const RecognitionException& e) {
        LOG_WARN("Face recognition error: " << e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in refresh_frame: " << e.what());
        camera_running = false;
        gtk_button_set_label(GTK_BUTTON(toggle_button), "Start Camera");
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Error - Check console");
    }

    return TRUE; // Continue timer
}

void GTKApp::on_toggle_button_clicked(GtkWidget* /*widget*/, gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->toggle_camera();
}

void GTKApp::toggle_camera() {
    try {
        if (!camera_running) {
            // Start camera
            if (!camera.is_camera_active()) {
                // Reopen camera if it was closed
                if (!camera.open(0)) {
                    LOG_ERROR("Failed to open camera");
                    gtk_label_set_text(GTK_LABEL(status_label), "Status: Failed to open camera");
                    return;
                }
            }
            camera.start();
            camera_running = true;
            gtk_button_set_label(GTK_BUTTON(toggle_button), "Stop Camera");
            gtk_label_set_text(GTK_LABEL(status_label), "Status: Camera Running");
        } else {
            // Stop camera and release resources
            camera.close();
            camera_running = false;
            gtk_button_set_label(GTK_BUTTON(toggle_button), "Start Camera");
            gtk_label_set_text(GTK_LABEL(status_label), "Status: Camera Stopped");
            gtk_image_clear(GTK_IMAGE(image_widget));
            gtk_label_set_text(GTK_LABEL(fps_label), "Recognition FPS: 0");
            frame_count = 0;
            recognition_frame_count = 0;
            last_time = 0;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception while toggling camera: " << e.what());
        camera_running = false;
        gtk_button_set_label(GTK_BUTTON(toggle_button), "Start Camera");
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Error - Check console");
    }
}

// Thread-safe camera start (for use from socket thread)
bool GTKApp::start_camera_safe() {
    try {
        if (camera_running) {
            return true;  // Already running
        }
        if (!camera.is_camera_active()) {
            if (!camera.open(0)) {
                LOG_ERROR("Failed to open camera");
                return false;
            }
        }
        camera.start();
        camera_running = true;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception while starting camera: " << e.what());
        return false;
    }
}

// Thread-safe camera stop (for use from socket thread)
bool GTKApp::stop_camera_safe() {
    try {
        if (!camera_running) {
            return true;  // Already stopped
        }

        // Set flag first to prevent refresh_frame from displaying more frames
        camera_running = false;

        // Close camera hardware (safe to do from any thread)
        camera.close();

        // Clear any cached frames (safe to do from any thread)
        last_frame.release();

        // Reset counters (safe to do from any thread)
        frame_count = 0;
        recognition_frame_count = 0;
        last_time = 0;

        // Schedule UI update on main GTK thread to avoid segmentation fault
        g_idle_add(on_camera_stop_complete, this);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception while stopping camera: " << e.what());
        return false;
    }
}

gboolean GTKApp::on_camera_stop_complete(gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->on_camera_stop_finished();
    return FALSE; // Remove from idle handlers
}

void GTKApp::on_camera_stop_finished() {
    // This runs on the main GTK thread, so it's safe to call GTK functions
    gtk_image_clear(GTK_IMAGE(image_widget));
    gtk_button_set_label(GTK_BUTTON(toggle_button), "Start Camera");
    gtk_label_set_text(GTK_LABEL(status_label), "Status: Camera Stopped");
    gtk_label_set_text(GTK_LABEL(fps_label), "Recognition FPS: 0");
    gtk_label_set_text(GTK_LABEL(face_info_label), "Person: None detected");
    gtk_label_set_text(GTK_LABEL(face_count_label), "Confidence: 0%");
    gtk_label_set_text(GTK_LABEL(recognition_time_label), "Recognition: 0ms");
    gtk_label_set_text(GTK_LABEL(error_rate_label), "Detection Rate: 0% | Error: 0%");

    // Force widget redraw to ensure clear takes effect immediately
    gtk_widget_queue_draw(image_widget);
}

void GTKApp::on_window_destroy(GtkWidget* /*widget*/, gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->cleanup();
    gtk_main_quit();
}

GdkPixbuf* GTKApp::mat_to_pixbuf(const cv::Mat& mat) {
    // Ensure the mat is in BGR format
    cv::Mat bgr_mat;
    if (mat.channels() == 1) {
        cv::cvtColor(mat, bgr_mat, cv::COLOR_GRAY2BGR);
    } else if (mat.channels() == 3) {
        bgr_mat = mat.clone();
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, bgr_mat, cv::COLOR_BGRA2BGR);
    } else {
        return nullptr;
    }

    // Convert BGR to RGB
    cv::Mat rgb_mat;
    cv::cvtColor(bgr_mat, rgb_mat, cv::COLOR_BGR2RGB);

    // Ensure continuous memory
    if (!rgb_mat.isContinuous()) {
        rgb_mat = rgb_mat.clone();
    }

    // Create GdkPixbuf directly with copied data
    GdkPixbuf* pixbuf = gdk_pixbuf_new(
        GDK_COLORSPACE_RGB,
        FALSE, // no alpha channel
        8,     // bits per sample
        rgb_mat.cols,
        rgb_mat.rows
    );

    if (pixbuf == nullptr) {
        LOG_ERROR("Failed to create pixbuf");
        return nullptr;
    }

    // Copy the data from cv::Mat to GdkPixbuf
    guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);

    for (int y = 0; y < rgb_mat.rows; y++) {
        memcpy(pixels + y * rowstride, rgb_mat.ptr(y), rgb_mat.cols * 3);
    }

    return pixbuf;
}

void GTKApp::draw_faces_on_frame(cv::Mat& frame, const std::vector<Face>& faces) {
    try {
        for (const auto& face : faces) {
            // Determine if face is recognized by checking face ID, confidence, and name
            // A face is recognized if:
            // 1. It has a valid positive ID (from FAISS match)
            // 2. Confidence is above the 70% threshold
            // 3. Name is not "Unknown" or "Too far"
            double threshold_percent = Config::RECOGNITION_CONFIDENCE_THRESHOLD * 100.0;
            bool is_recognized = (face.id > 0) &&
                                (face.confidence >= threshold_percent) &&
                                (face.name != "Unknown") &&
                                (face.name != "Too far");


            // Use dynamic bounding box based on detected face size
            // Scale the detected face by configured scale factor
            int box_width = static_cast<int>(face.bbox.width * Config::BOUNDING_BOX_SCALE);
            int box_height = static_cast<int>(face.bbox.height * Config::BOUNDING_BOX_SCALE);

            int face_center_x = face.bbox.x + face.bbox.width / 2;
            int face_center_y = face.bbox.y + face.bbox.height / 2;

            cv::Rect expanded_bbox(
                face_center_x - box_width / 2,
                face_center_y - box_height / 2,
                box_width,
                box_height
            );

            // Draw corner lines only (horizontal and vertical lines at each corner)
            int corner_length = static_cast<int>(box_width * 0.15); // 15% of dynamic width for corner length
            int line_thickness = 2;

            // Color based on recognition status
            // Green for recognized faces, Red for unknown faces
            cv::Scalar color = is_recognized ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);

            // Top-left corner
            // Horizontal line
            cv::line(frame,
                    cv::Point(expanded_bbox.x, expanded_bbox.y),
                    cv::Point(expanded_bbox.x + corner_length, expanded_bbox.y),
                    color, line_thickness);
            // Vertical line
            cv::line(frame,
                    cv::Point(expanded_bbox.x, expanded_bbox.y),
                    cv::Point(expanded_bbox.x, expanded_bbox.y + corner_length),
                    color, line_thickness);

            // Top-right corner
            // Horizontal line
            cv::line(frame,
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y),
                    cv::Point(expanded_bbox.x + expanded_bbox.width - corner_length, expanded_bbox.y),
                    color, line_thickness);
            // Vertical line
            cv::line(frame,
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y),
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y + corner_length),
                    color, line_thickness);

            // Bottom-left corner
            // Horizontal line
            cv::line(frame,
                    cv::Point(expanded_bbox.x, expanded_bbox.y + expanded_bbox.height),
                    cv::Point(expanded_bbox.x + corner_length, expanded_bbox.y + expanded_bbox.height),
                    color, line_thickness);
            // Vertical line
            cv::line(frame,
                    cv::Point(expanded_bbox.x, expanded_bbox.y + expanded_bbox.height),
                    cv::Point(expanded_bbox.x, expanded_bbox.y + expanded_bbox.height - corner_length),
                    color, line_thickness);

            // Bottom-right corner
            // Horizontal line
            cv::line(frame,
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y + expanded_bbox.height),
                    cv::Point(expanded_bbox.x + expanded_bbox.width - corner_length, expanded_bbox.y + expanded_bbox.height),
                    color, line_thickness);
            // Vertical line
            cv::line(frame,
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y + expanded_bbox.height),
                    cv::Point(expanded_bbox.x + expanded_bbox.width, expanded_bbox.y + expanded_bbox.height - corner_length),
                    color, line_thickness);

            // Draw label with name and confidence
            // Note: face.confidence may contain either detection confidence or recognition confidence
            // For display, show the current confidence value
            std::string label;
            int confidence_display = static_cast<int>(face.confidence);

            if (is_recognized) {
                label = face.name + " (" + std::to_string(confidence_display) + "%)";
            } else {
                label = "Unknown (" + std::to_string(confidence_display) + "%)";
            }

            int baseline = 0;
            cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.45, 1, &baseline);

            // Background color based on recognition status
            // Green background for recognized/known faces, Red for unknown faces
            cv::Scalar bg_color = is_recognized ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 200);

            // Text color: black text for known faces, white text for unknown faces
            cv::Scalar text_color = is_recognized ? cv::Scalar(0, 0, 0) : cv::Scalar(255, 255, 255);

            // Draw background for text (above the face area) with minimal padding
            cv::rectangle(frame,
                         cv::Point(expanded_bbox.x - 1, expanded_bbox.y - text_size.height - 4),
                         cv::Point(expanded_bbox.x + text_size.width + 1, expanded_bbox.y),
                         bg_color, -1);

            // Draw text with normal font weight
            cv::putText(frame, label,
                       cv::Point(expanded_bbox.x, expanded_bbox.y - 3),
                       cv::FONT_HERSHEY_SIMPLEX, 0.45, text_color, 1);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in draw_faces_on_frame: " << e.what());
    }
}

void GTKApp::load_face_recognizer() {
    try {
        LOG_INFO("Loading face recognizer (Deep Learning - ArcFace + FAISS)...");

        // Initialize database
        if (!face_database.open()) {
            LOG_ERROR("Failed to open face database");
            return;
        }

        if (!face_database.initialize()) {
            LOG_ERROR("Failed to initialize face database");
            face_database.close();
            return;
        }

        // Initialize face detector
        if (!face_detector.initialize()) {
            LOG_ERROR("Failed to initialize face detector");
            face_database.close();
            return;
        }

        // Set database reference in recognizer
        face_recognizer.set_database(&face_database);

        // Load ArcFace ONNX model (InsightFace w600k_r50)
        std::string model_path = "models/arcface_w600k_r50.onnx";
        if (!std::filesystem::exists(model_path)) {
            LOG_WARN("ArcFace model not found at " << model_path);
            LOG_INFO("Please download the model and place it at: " << model_path);
            LOG_INFO("Visit: https://huggingface.co/public-data/insightface/tree/main/models/buffalo_l");
            face_recognition_enabled = false;
            return;
        }

        LOG_INFO("Loading ArcFace model from: " << model_path);
        if (!face_recognizer.load_model(model_path)) {
            LOG_ERROR("Failed to load ArcFace model");
            face_recognition_enabled = false;
            return;
        }

        LOG_INFO("ArcFace model loaded successfully");

        // Try to load saved FAISS index first (faster startup)
        std::string faiss_index_path = "faiss_index.bin";
        if (std::filesystem::exists(faiss_index_path)) {
            LOG_INFO("Loading saved FAISS index from: " << faiss_index_path);
            if (face_recognizer.load_index(faiss_index_path)) {
                face_recognition_enabled = true;
                LOG_INFO("FAISS index loaded successfully");
                LOG_INFO("Number of people in database: " << face_database.get_num_people());
                LOG_INFO("Face recognition ready!");
                return;
            } else {
                LOG_WARN("Failed to load FAISS index, will try training from database");
            }
        }

        // Fallback: Try to train from database embeddings
        if (face_database.get_total_faces() > 0) {
            LOG_INFO("Loading face embeddings from database...");
            if (face_recognizer.train_from_database()) {
                face_recognition_enabled = true;
                LOG_INFO("Face recognizer loaded successfully");
                LOG_INFO("Number of people in database: " << face_database.get_num_people());
                LOG_INFO("Total faces in database: " << face_database.get_total_faces());
            } else {
                LOG_ERROR("Failed to train from database");
                face_recognition_enabled = false;
            }
        } else {
            LOG_INFO("No face data in database yet. Add photos to start recognizing faces.");
            face_recognition_enabled = false;
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in load_face_recognizer: " << e.what());
        face_recognition_enabled = false;
    }
}

void GTKApp::on_train_button_clicked(GtkWidget* /*widget*/, gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->train_model();
}

void GTKApp::train_model() {
    if (training_in_progress) {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Training already in progress");
        return;
    }

    // Check if model is loaded before attempting training
    if (!face_recognizer.is_model_loaded()) {
        GtkWidget* error_dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "ArcFace Model Not Loaded");
        gtk_message_dialog_format_secondary_text(
            GTK_MESSAGE_DIALOG(error_dialog),
            "Cannot train the model because the ArcFace ONNX model is missing or failed to load.\n\n"
            "Please download the ArcFace ONNX model to models/arcface_w600k_r50.onnx\n"
            "Visit: https://huggingface.co/public-data/insightface");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Model not loaded - cannot train");
        return;
    }

    // Check if dataset directory exists and has subdirectories
    if (!std::filesystem::exists("dataset")) {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Dataset directory not found");
        return;
    }

    training_in_progress = true;
    gtk_widget_set_sensitive(train_button, FALSE);
    gtk_label_set_text(GTK_LABEL(status_label), "Status: Training model from dataset... please wait");

    LOG_INFO("Starting training from dataset...");

    // Join previous training thread if it exists
    if (training_thread.joinable()) {
        training_thread.join();
    }

    // Start training in background thread
    training_thread = std::thread(&GTKApp::train_model_async, this);
}

void GTKApp::train_model_async() {
    // This runs in a background thread
    bool success = face_recognizer.train_from_images("dataset");
    training_success = success;

    // Schedule UI update on main thread
    g_idle_add(on_training_complete, this);
}

gboolean GTKApp::on_training_complete(gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->on_training_finished();
    return FALSE; // Remove from idle handlers
}

void GTKApp::on_training_finished() {
    if (training_success) {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Training complete! Ready to recognize faces.");
        face_recognition_enabled = true;
        LOG_INFO("Training successful!");
    } else {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Training failed - add photos and try again");
        LOG_ERROR("Training failed");
        face_recognition_enabled = false;
    }

    training_in_progress = false;
    gtk_widget_set_sensitive(train_button, TRUE);
}

void GTKApp::on_capture_button_clicked(GtkWidget* /*widget*/, gpointer user_data) {
    GTKApp* self = static_cast<GTKApp*>(user_data);
    self->capture_photo();
}

void GTKApp::capture_photo() {
    if (!camera_running) {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Start camera before capturing");
        return;
    }

    if (last_frame.empty()) {
        gtk_label_set_text(GTK_LABEL(status_label), "Status: No frame available to capture");
        return;
    }

    // Check if model is loaded (not just trained)
    if (!face_recognizer.is_model_loaded()) {
        GtkWidget* error_dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "ArcFace Model Not Loaded");
        gtk_message_dialog_format_secondary_text(
            GTK_MESSAGE_DIALOG(error_dialog),
            "Cannot capture photos because the ArcFace model is missing or failed to load.\n\n"
            "Please download the ArcFace ONNX model to models/arcface_w600k_r50.onnx\n"
            "Visit: https://huggingface.co/public-data/insightface");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        gtk_label_set_text(GTK_LABEL(status_label), "Status: Model not loaded - cannot capture");
        return;
    }

    // Create dataset directory if it doesn't exist
    if (!std::filesystem::exists("dataset")) {
        std::filesystem::create_directory("dataset");
    }

    // Ask user for person initial and ID via dialog
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "Capture Photo",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "OK", GTK_RESPONSE_OK,
        nullptr);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget* label1 = gtk_label_new("Person Initial (A, B, C, etc.):");
    gtk_box_pack_start(GTK_BOX(content_area), label1, FALSE, FALSE, 5);
    GtkWidget* entry_initial = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_initial), 1);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_initial), "A");
    gtk_box_pack_start(GTK_BOX(content_area), entry_initial, FALSE, FALSE, 5);
    gtk_widget_set_size_request(entry_initial, 100, 35);

    GtkWidget* label2 = gtk_label_new("Person ID (number):");
    gtk_box_pack_start(GTK_BOX(content_area), label2, FALSE, FALSE, 5);
    GtkWidget* entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "1");
    gtk_box_pack_start(GTK_BOX(content_area), entry_id, FALSE, FALSE, 5);
    gtk_widget_set_size_request(entry_id, 100, 35);

    gtk_widget_show_all(dialog);
    
    // Pause live stream and face recognition AFTER showing dialog
    capture_in_progress = true;
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        const char* initial = gtk_entry_get_text(GTK_ENTRY(entry_initial));
        const char* id_str = gtk_entry_get_text(GTK_ENTRY(entry_id));

        if (initial && strlen(initial) > 0 && id_str && strlen(id_str) > 0) {
            // Create person subdirectory structure
            std::string initial_str(initial);
            std::string id_num(id_str);

            // Convert initial to uppercase
            initial_str[0] = std::toupper(initial_str[0]);

            // Create person-specific subdirectory: dataset/A1/, dataset/B2/, etc.
            std::string person_name = initial_str + id_num;  // e.g., "A1", "B2"
            std::string person_dir = "dataset/" + person_name;

            try {
                if (!std::filesystem::exists(person_dir)) {
                    std::filesystem::create_directories(person_dir);
                    LOG_DEBUG("Created person directory: " << person_dir);
                }
            } catch (const std::exception& e) {
                gtk_label_set_text(GTK_LABEL(status_label), "Status: Failed to create person directory");
                LOG_ERROR("Error creating directory: " << e.what());
                return;
            }

            // Count existing files for this person to determine sequence number
            int sequence = 1;
            try {
                for (const auto& entry : std::filesystem::directory_iterator(person_dir)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        if (ext == ".jpg" || ext == ".png" || ext == ".bmp") {
                            sequence++;
                        }
                    }
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Error counting files: " << e.what());
            }

            // Generate filename: A1/1.jpg, A1/2.jpg, etc.
            std::string filename = person_dir + "/" + std::to_string(sequence) + ".jpg";

            // Save the frame
            if (cv::imwrite(filename, last_frame)) {
                // Register person in database if not already registered
                PersonRecord person;
                bool person_exists = face_database.get_person_by_name(person_name, person);

                if (!person_exists) {
                    if (face_database.add_person(person_name)) {
                        LOG_INFO("Person registered in database: " << person_name);
                        // Get the newly created person
                        face_database.get_person_by_name(person_name, person);
                    } else {
                        LOG_ERROR("Failed to register person in database");
                        gtk_label_set_text(GTK_LABEL(status_label), "Status: Failed to register person");
                        gtk_widget_destroy(dialog);
                        return;
                    }
                }

                // Add face image to database (for record keeping)
                face_database.add_face_image(person.id, filename);

                // Extract and store face embedding
                cv::Mat face_image = cv::imread(filename);
                if (!face_image.empty()) {
                    // Detect face in the captured image to get proper face ROI
                    // This ensures the embedding is extracted from the same region as in live stream
                    std::vector<Face> detected_faces = face_detector.detect_faces(face_image);
                    std::vector<float> embedding;
                    cv::Mat image_for_training = face_image;  // Default to full image

                    if (!detected_faces.empty()) {
                        // Use the largest detected face
                        cv::Rect best_bbox = detected_faces[0].bbox;
                        for (const auto& face : detected_faces) {
                            if (face.bbox.area() > best_bbox.area()) {
                                best_bbox = face.bbox;
                            }
                        }

                        // Extract face ROI
                        if (best_bbox.x >= 0 && best_bbox.y >= 0 &&
                            best_bbox.x + best_bbox.width <= face_image.cols &&
                            best_bbox.y + best_bbox.height <= face_image.rows) {
                            cv::Mat face_roi = face_image(best_bbox).clone();
                            embedding = face_recognizer.extract_embedding(face_roi);
                            image_for_training = face_roi;  // Use face ROI for training
                        } else {
                            embedding = face_recognizer.extract_embedding(face_image);
                        }
                    } else {
                        // No face detected, use full image as fallback
                        embedding = face_recognizer.extract_embedding(face_image);
                    }

                    if (!embedding.empty()) {
                        // Convert embedding to bytes and store in database
                        std::vector<unsigned char> embedding_bytes(
                            reinterpret_cast<unsigned char*>(embedding.data()),
                            reinterpret_cast<unsigned char*>(embedding.data()) + embedding.size() * sizeof(float)
                        );

                        if (face_database.add_face_embedding(person.id, filename, embedding_bytes)) {
                            gchar status_text[200];
                            g_snprintf(status_text, sizeof(status_text),
                                      "Status: Photo & embedding saved - %s (Total: %d faces)",
                                      person_name.c_str(), face_database.get_total_faces());
                            gtk_label_set_text(GTK_LABEL(status_label), status_text);
                            LOG_INFO("Embedding extracted and stored for: " << person_name);

                            // Add embedding to FAISS index (incremental update, no full retrain needed)
                            if (face_recognizer.add_training_data(image_for_training, person.id)) {
                                gchar add_text[200];
                                g_snprintf(add_text, sizeof(add_text),
                                          "Status: %s added to recognition model",
                                          person_name.c_str());
                                gtk_label_set_text(GTK_LABEL(status_label), add_text);
                                LOG_INFO("Person added to recognition model: " << person_name);

                                // Reload FAISS index from disk to ensure memory reflects saved state
                                std::string faiss_index_path = "faiss_index.bin";
                                if (std::filesystem::exists(faiss_index_path)) {
                                    if (face_recognizer.load_index(faiss_index_path)) {
                                        // Index reloaded successfully
                                    } else {
                                        LOG_ERROR("Failed to reload FAISS index");
                                    }
                                }

                                // Ensure label maps are up-to-date after reload
                                face_recognizer.load_labels_from_database();

                                // Verify person is in label maps
                                std::string loaded_name = face_recognizer.get_label_name(person.id);
                                if (loaded_name == "Unknown") {
                                    // Manually register if needed
                                    face_recognizer.register_person(person.name);
                                }

                                // Verify model is trained
                                if (!face_recognizer.is_trained()) {
                                    // Force train by loading the index again
                                    if (std::filesystem::exists(faiss_index_path)) {
                                        face_recognizer.load_index(faiss_index_path);
                                    }
                                }

                                // Enable face recognition if not already enabled
                                if (!face_recognition_enabled) {
                                    face_recognition_enabled = true;
                                }
                            } else {
                                gtk_label_set_text(GTK_LABEL(status_label), "Status: Embedding saved but adding to model failed");
                                LOG_ERROR("Failed to add embedding to FAISS index");
                            }
                        } else {
                            gchar status_text[200];
                            g_snprintf(status_text, sizeof(status_text),
                                      "Status: Photo saved but embedding storage failed - %s",
                                      person_name.c_str());
                            gtk_label_set_text(GTK_LABEL(status_label), status_text);
                            LOG_ERROR("Failed to store embedding for: " << person_name);
                        }
                    } else {
                        gchar status_text[200];
                        g_snprintf(status_text, sizeof(status_text),
                                  "Status: Photo saved but embedding extraction failed - %s",
                                  person_name.c_str());
                        gtk_label_set_text(GTK_LABEL(status_label), status_text);
                        LOG_ERROR("Failed to extract embedding for: " << person_name);
                    }
                } else {
                    gchar status_text[200];
                    g_snprintf(status_text, sizeof(status_text),
                              "Status: Photo saved but cannot load for embedding - %s",
                              person_name.c_str());
                    gtk_label_set_text(GTK_LABEL(status_label), status_text);
                    LOG_ERROR("Failed to load saved image: " << filename);
                }
            } else {
                gtk_label_set_text(GTK_LABEL(status_label), "Status: Failed to save photo");
                LOG_ERROR("Failed to save photo");
            }
        } else {
            gtk_label_set_text(GTK_LABEL(status_label), "Status: Invalid input - please enter initial and ID");
        }
    }

    gtk_widget_destroy(dialog);
    
    // Resume live stream and face recognition
    capture_in_progress = false;
    
    // Force a few frame refreshes to ensure the display updates
    for (int i = 0; i < 3; i++) {
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        g_usleep(10000); // 10ms delay
    }
    
    gtk_label_set_text(GTK_LABEL(status_label), "Status: Live stream resumed");
}

void GTKApp::setup_socket_server() {
    socket_server = std::make_unique<SocketServer>();

    // Register command handlers
    socket_server->register_command("camera_on", [this](const std::string& args) {
        return handle_camera_on(args);
    });

    socket_server->register_command("camera_off", [this](const std::string& args) {
        return handle_camera_off(args);
    });

    socket_server->register_command("capture", [this](const std::string& args) {
        return handle_capture(args);
    });

    socket_server->register_command("registering", [this](const std::string& args) {
        return handle_registering(args);
    });

    socket_server->register_command("status", [this](const std::string& args) {
        return handle_status(args);
    });

    socket_server->register_command("list", [this](const std::string& args) {
        return handle_list_persons(args);
    });

    socket_server->register_streaming_command("stream_recognition", [this](int client_fd) {
        handle_stream_recognition(client_fd);
    });

    // Start the socket server
    if (!socket_server->start()) {
        throw std::runtime_error("Failed to start socket server");
    }
}

std::string GTKApp::handle_camera_on(const std::string& /* args */) {
    if (start_camera_safe()) {
        return "OK:Camera started";
    } else {
        return "ERROR:Failed to start camera";
    }
}

std::string GTKApp::handle_camera_off(const std::string& /* args */) {
    if (stop_camera_safe()) {
        return "OK:Camera stopped";
    } else {
        return "ERROR:Failed to stop camera";
    }
}

std::string GTKApp::handle_capture(const std::string& args) {
    if (!camera_running) {
        return "ERROR:Camera not running";
    }

    // Parse arguments: "initial:id"
    std::istringstream iss(args);
    std::string initial, id_str;
    std::getline(iss, initial, ':');
    std::getline(iss, id_str);

    if (initial.empty() || id_str.empty()) {
        return "ERROR:Missing arguments. Usage: capture:A:1";
    }

    // Convert initial to uppercase
    initial[0] = std::toupper(initial[0]);

    // Validate ID is numeric
    try {
        std::stoi(id_str);
    } catch (...) {
        return "ERROR:Invalid ID. Must be numeric.";
    }

    std::string person_name = initial + id_str;

    // Create dataset directory if it doesn't exist
    if (!std::filesystem::exists("dataset")) {
        std::filesystem::create_directory("dataset");
    }

    std::string person_dir = "dataset/" + person_name;
    if (!std::filesystem::exists(person_dir)) {
        std::filesystem::create_directories(person_dir);
    }

    // Count existing files for this person
    int sequence = 1;
    for (const auto& entry : std::filesystem::directory_iterator(person_dir)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".jpg" || ext == ".png" || ext == ".bmp") {
                sequence++;
            }
        }
    }

    std::string filename = person_dir + "/" + std::to_string(sequence) + ".jpg";

    // Save the current frame
    if (last_frame.empty() || !cv::imwrite(filename, last_frame)) {
        return "ERROR:Failed to capture photo";
    }

    // Register person in database if not already registered
    PersonRecord person;
    bool person_exists = face_database.get_person_by_name(person_name, person);

    if (!person_exists) {
        if (!face_database.add_person(person_name)) {
            return "ERROR:Failed to register person";
        }
        face_database.get_person_by_name(person_name, person);
    }

    // Add face image to database
    face_database.add_face_image(person.id, filename);

    // Extract and store face embedding
    cv::Mat face_image = cv::imread(filename);
    if (!face_image.empty()) {
        std::vector<Face> detected_faces = face_detector.detect_faces(face_image);
        std::vector<float> embedding;
        cv::Mat image_for_training = face_image;

        if (!detected_faces.empty()) {
            cv::Rect best_bbox = detected_faces[0].bbox;
            for (const auto& face : detected_faces) {
                if (face.bbox.area() > best_bbox.area()) {
                    best_bbox = face.bbox;
                }
            }

            if (best_bbox.x >= 0 && best_bbox.y >= 0 &&
                best_bbox.x + best_bbox.width <= face_image.cols &&
                best_bbox.y + best_bbox.height <= face_image.rows) {
                cv::Mat face_roi = face_image(best_bbox).clone();
                embedding = face_recognizer.extract_embedding(face_roi);
                image_for_training = face_roi;
            } else {
                embedding = face_recognizer.extract_embedding(face_image);
            }
        } else {
            embedding = face_recognizer.extract_embedding(face_image);
        }

        if (!embedding.empty()) {
            std::vector<unsigned char> embedding_bytes(
                reinterpret_cast<unsigned char*>(embedding.data()),
                reinterpret_cast<unsigned char*>(embedding.data()) + embedding.size() * sizeof(float)
            );

            if (face_database.add_face_embedding(person.id, filename, embedding_bytes)) {
                if (face_recognizer.add_training_data(image_for_training, person.id)) {
                    std::string faiss_index_path = "faiss_index.bin";
                    if (std::filesystem::exists(faiss_index_path)) {
                        face_recognizer.load_index(faiss_index_path);
                    }
                    face_recognizer.load_labels_from_database();

                    if (!face_recognition_enabled) {
                        face_recognition_enabled = true;
                    }

                    return "OK:Photo captured and person added - " + person_name;
                } else {
                    return "ERROR:Failed to add to recognition model";
                }
            } else {
                return "ERROR:Failed to store embedding";
            }
        } else {
            return "ERROR:Failed to extract embedding";
        }
    } else {
        return "ERROR:Failed to load captured image";
    }
}

std::string GTKApp::handle_registering(const std::string& /* args */) {
    if (training_in_progress) {
        return "ERROR:Training already in progress";
    }

    train_model_async();
    return "OK:Training started";
}

std::string GTKApp::handle_status(const std::string& /* args */) {
    std::string status;
    status += "camera_running:" + std::string(camera_running ? "true" : "false") + ",";
    status += "recognition_enabled:" + std::string(face_recognition_enabled ? "true" : "false") + ",";
    status += "training_in_progress:" + std::string(training_in_progress ? "true" : "false") + ",";
    status += "people_count:" + std::to_string(face_database.get_num_people()) + ",";
    status += "total_faces:" + std::to_string(face_database.get_total_faces());

    return "OK:" + status;
}

std::string GTKApp::handle_list_persons(const std::string& /* args */) {
    try {
        std::vector<PersonRecord> persons;
        if (!face_database.get_all_people(persons)) {
            return "ERROR:Failed to retrieve person list";
        }
        
        std::string result = "OK:" + std::to_string(persons.size());
        for (const auto& person : persons) {
            result += "," + person.name;
        }
        
        return result;
    } catch (const std::exception& e) {
        return "ERROR:Failed to list persons - " + std::string(e.what());
    }
}

void GTKApp::handle_stream_recognition(int client_fd) {
    // Send initial status
    std::string initial_response = "OK:Stream started\n";
    if (write(client_fd, initial_response.c_str(), initial_response.length()) < 0) {
        LOG_ERROR("Failed to send initial response");
        return;
    }

    // Stream recognition results continuously until client disconnects
    while (socket_server->is_running()) {
        // Lock to access recognition data
        {
            std::lock_guard<std::mutex> lock(recognition_mutex);

            // Only send data if camera is running
            if (camera_running) {
                // Format: RECOGNITION:name:confidence:timestamp
                // Multiple faces separated by newlines
                // Send current recognition state
                std::string frame_data;
                
                auto now = std::chrono::high_resolution_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();

                if (has_recognition_result) {
                    frame_data = "FACE:" + last_recognized_name + ":" +
                                std::to_string(static_cast<int>(last_recognized_confidence)) + ":" +
                                std::to_string(timestamp) + "\n";
                } else {
                    frame_data = "NO_FACE:" + std::to_string(timestamp) + "\n";
                }

                // Send the frame data
                if (write(client_fd, frame_data.c_str(), frame_data.length()) < 0) {
                    LOG_INFO("Client disconnected from stream");
                    return;
                }
            }
        }

        // Sleep for 500ms between updates (matching recognition interval)
        usleep(500000);  // 500ms
    }
}

